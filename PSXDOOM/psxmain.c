/*-------------------------------------------------**
**                                                 **
**-------------------------------------------------*/

/* SONY LIBRARIES */
#include <sys/types.h>
#include <r3000.h>
#include <asm.h>
#include <kernel.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsn.h>
#include <libgs.h>
#include <libapi.h>
#include <libcomb.h>
#include <libetc.h>
#include <sys/file.h> //O_RDONLY|O_WRONLY|O_NOWAIT
#include <sys/ioctl.h>

//---------------------------------------

#include "doomdef.h"
#include "r_local.h"

DRAWENV	draw[2];					/* draw environment */      //800A8E90, 800A8EEC
DISPENV disp[2];					/* display environment*/    //800a8f48, 800A8F5C

long drawside;//80077F20

int	drawsync1;//80077FE4
int	drawsync2;//80077F3C
extern int	v_sync;//80077CC8

#define OTSIZE	(1<<14)//16384
unsigned long	ot[OTSIZE];//80086378 psxdoom ot
unsigned long	*sys_ot[2] = {ot, ot};//off_80077A44[0], off_80077A48[1]

#define	BUFFSIZE	(8)
static long fr, fw;//80078058, 80077D40
static unsigned long ev_r, ev_w, ev_e;	//read/write/error event descriptor //80077D50, 80077E6C,
static char recbuf[BUFFSIZE];//0x80077cd4
static char senbuf[BUFFSIZE];//0x80077ddc

// Vram
unsigned int PagesXY[12][2] =
{
	//0x80073aac, 0x80073ab0
	{ 512 ,  0  },
	{ 640 ,  0  },
	{ 768 ,  0  },
	{ 896 ,  0  },
	{  0  , 256 },
	{ 128 , 256 },
	{ 256 , 256 },
	{ 384 , 256 },
	{ 512 , 256 },
	{ 640 , 256 },
	{ 768 , 256 },
	{ 896 , 256 },
};
//unsigned char *vram_cache;//80077DA0

int PageCount = 0;//*(r28 + 2628)
int TextureCacheIdx = 0;//80077A40
int xycount = 0;  //*(r28 + 3212);
int xcount = 0;   //*(r28 + 3320)
int ycount = 0;   //*(r28 + 3324)
int V_PagFlags = 0;//*(r28 + 1576)//80077A38

int main()
{
	D_DoomMain ();
	return (0);
}

void PSX_INIT(void)//L80032804()
{
    ResetCallback();
    PadInit(0);
    ResetGraph(0);
    SetGraphDebug(0);

    InitGeom();
    SetGeomScreen(128);
    SetGeomOffset(128, 100);

    SetDefDrawEnv(&draw[0], 0, 0, 256, 240);
    draw[0].isbg = 1;
    draw[0].dtd = 0;
    SetDefDrawEnv(&draw[1], 256, 0, 256, 240);
    draw[1].isbg = 1;
    draw[1].dtd = 0;
    SetDefDispEnv(&disp[0], 256, 0, 256, 240);
    SetDefDispEnv(&disp[1], 0, 0, 256, 240);

    drawside = 0;

    EnterCriticalSection();
    //__asm__("nop");
    ExitCriticalSection();

    /* Initialize link cable communications */
    {
        /* attacth the SIO driver to the kernel */
        AddCOMB();

        /* open an event to detect the end of read operation */
        ev_r = OpenEvent(HwSIO, EvSpIOER, EvMdNOINTR, NULL);
        EnableEvent(ev_r);

        /* open an event to detect the end of write operation */
        ev_w = OpenEvent(HwSIO, EvSpIOEW, EvMdNOINTR, NULL);
        EnableEvent(ev_w);

        /* open stream for writing */
        fw = open("sio:", O_WRONLY);

        /* open stream for reading */
        fr = open("sio:", O_RDONLY|O_NOWAIT);

        /* set comminucation rate */
        CombSetBPS(38400);
    }

    DrawRender();
    DrawRender();

    SetDispMask(1);
}

#include <stdarg.h> //va_list|va_start|va_end
void I_Error(char *error, ...)//L800329BC
{
    int Fnt;
    char buffer[256];
	va_list args;
	va_start (args, error);
	D_vsprintf (buffer, error, args);
	va_end (args);
	DrawRender();

	FntLoad(960, 256); // load the font from the BIOS into the framebuffer
	Fnt = FntOpen(0, 0, 256, 200, 0, 256);
	SetDumpFnt(Fnt); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters

	FntPrint("\n\n\n %s", buffer);
	FntFlush(Fnt);// refresh the font
	DrawRender();
	while (1){} // draw and display forever
}

void ImageToVram(psxobj_t *pic, char *name, int lump)//L80032A68()
{
	int w, h;
	byte *data;

	if (name) {
		lump = W_GetNumForName(name);
	}

	data = (byte *)W_CacheLumpNum(lump, PU_CACHE, false);

	if (!lumpencode[lump]) {
		decode(data, tempbuffer);
		data = (byte *)&tempbuffer;
	}

	/* vram xy_wh */
	pic->x = BIGSHORT(((patchpsx_t*)data)->leftoffset);
	pic->y = BIGSHORT(((patchpsx_t*)data)->topoffset);
	pic->w = BIGSHORT(((patchpsx_t*)data)->width);
	pic->h = BIGSHORT(((patchpsx_t*)data)->height);

	/* vram base x */
	w = (int)BIGSHORT(((patchpsx_t*)data)->width) + 0xf;
	if (w < 0) {
		w = (int)BIGSHORT(((patchpsx_t*)data)->width) + 0x1e;
	}
	pic->vbasex = (short)(w >> 4);

	/* vram base y */
	h = (int)BIGSHORT(((patchpsx_t*)data)->height) + 0xf;
	if (h < 0) {
		h = (int)BIGSHORT(((patchpsx_t*)data)->height) + 0x1e;
	}
	pic->vbasey = (short)(h >> 4);

	/* vram tpage */
	pic->vtpage = 0;

	/* vram lump */
	pic->lump = (short)lump;

	TextureCache(pic);
}

void DrawStaticImage(psxobj_t *psxobj, int xpos, int ypos, int pal)//L80032B78()
{
	TextureCache(psxobj);
	DrawImage(psxobj->vtpage, pal, xpos, ypos, psxobj->vramx, psxobj->vramy, psxobj->w, psxobj->h);
}

void DrawImage(int vtpage, int pal, int xpos, int ypos,  char vramx, char vramy, int width, int height)//L80032BF8()
{
	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT *pic = (SPRT*) getScratchAddr(128);//1F800200

	SetDrawMode(drawmode, 0, 0, vtpage, NULL);
	W_AddPrim(drawmode);// add to order table

	setSprt(pic);
	setRGB0(pic, 128, 128, 128);
	setUV0(pic, vramx, vramy);
	setWH(pic, width, height);
	setXY0(pic, xpos, ypos);
	pic->clut = pal;
	W_AddPrim(pic);// add to order table
}

void P_LoadingPlaque(psxobj_t *psxobj, int xpos, int ypos, int pal)//L80033154()
{
	DrawSync(0);
	MoveImage(&disp[drawside].disp, disp[drawside^1].disp.x, disp[drawside^1].disp.y);

	//Setup Loading Image Obj
	NextTextureCacheIdx();
	TextureCache(psxobj);

	DrawImage(psxobj->vtpage, pal, xpos, ypos, psxobj->vramx, psxobj->vramy, psxobj->w, psxobj->h);

	UpdateDrawOTag();
	DrawRender();
}

void NextTextureCacheIdx(void)//L8003324C()
{
    TextureCacheIdx += 1;
}

#if SHOWFPS
int fps;
#endif // SHOWFPS

void DrawRender(void)//80033264
{
	DrawSync(0);

	#if SHOWFPS
    fps = VSync(0);
    #else
    VSync(0);
    #endif // SHOWFPS

	drawside ^= 1;

	PutDrawEnv(&draw[drawside]);
	PutDispEnv(&disp[drawside]);

	do
    {
		v_sync = VSync(-1);
		drawsync1 = v_sync - drawsync2;
	} while(drawsync1 < 2);

    if (demoplayback || demorecording)
    {
        do
        {
            v_sync = VSync(-1);
            drawsync1 = v_sync - drawsync2;
        } while(drawsync1 < 4);
        drawsync1 = 4;
    }

    drawsync2 = v_sync;
}

void Add_vsync(void)//L80033380()
{
    v_sync++;
}

/* VRAM memory allocation structure */
typedef struct v_vrammem
{
	psxobj_t *imgobj[MAX_VRAMMEM];			// ptr to start addr of image
} VRAMMEM;

VRAMMEM *vram_cache;//80077DA0

//uGp00000628	V_PagFlags
//iGp00000990	vram_cache
//uGp00000a44	PageCount
//uGp00000c8c	xycount
//uGp00000cf8	xcount
//uGp00000cfc	ycount
//uGp00000630	TextureCacheIdx

void Init_Vram_Cache(void)//800333A0
{
	vram_cache = (VRAMMEM *)Z_Malloc((sizeof(VRAMMEM) * MAX_DYNAMIC_TPAGE), PU_STATIC, 0);
	D_memset(vram_cache, 0, (sizeof(VRAMMEM)  *MAX_DYNAMIC_TPAGE));
	Valloc_Init();
}

void TextureCache(psxobj_t *psxobj)
{
	RECT			frame;
	psxobj_t	    **imgobj;
	unsigned long	*vram, *vram2;
	byte			*tmp;
	psxobj_t		*psxobj1;
	byte			*data;
	unsigned int	lastpage;
	int             x, y;
	short		    xs, ys;

	lastpage = PageCount;
	psxobj->index = TextureCacheIdx;

	if (psxobj->vtpage == 0)
	{
	check_again:
		if ((psxobj->vbasex + xcount) > MIN_VRAM_ALLOC_SIZE)
		{
			ycount += xycount;
			xcount = 0;
			xycount = 0;
		}

		if ((psxobj->vbasey + ycount) > MIN_VRAM_ALLOC_SIZE)
		{
			do
			{
				PageCount = (PageCount + 1) % MAX_DYNAMIC_TPAGE;
			} while ((V_PagFlags >> (PageCount & 0x1f) & 1U) != 0);

			if (PageCount == lastpage)
			{
				I_Error("Texture Cache Overflow\n");
			}
			xcount = 0;
			ycount = 0;
			xycount = 0;
		}

		imgobj = &vram_cache[PageCount].imgobj[(ycount * MIN_VRAM_ALLOC_SIZE) + xcount];

		vram = (unsigned long*)imgobj;
		xs = psxobj->vbasex;	// xsize of alloc block
		ys = psxobj->vbasey;	// ysize of alloc block

		// copy address object into the block
		for (y = 0; y < ys; y++, vram += (MIN_VRAM_ALLOC_SIZE - xs))
		{
			for (x = 0; x < xs; x++, vram++)
			{
				psxobj1 = (psxobj_t *)*(unsigned long*)vram;//get address obj on vram

				if (psxobj1 != 0)
				{
					if (psxobj1->index == TextureCacheIdx)
					{
						xcount += psxobj1->vbasex;
						if (xycount < psxobj1->vbasey)
							xycount = psxobj1->vbasey;
						goto check_again;
					}
					else
						V_ClearBlock(psxobj1);
				}
			}
		}

		vram2 = (unsigned long*)imgobj;
		xs = psxobj->vbasex;	// xsize of alloc block
		ys = psxobj->vbasey;	// ysize of alloc block

		// copy address object into the block
		for (y = 0; y < ys; y++, vram2 += (MIN_VRAM_ALLOC_SIZE - xs))
		{
			for (x = 0; x < xs; x++, vram2++)
			{
				*(unsigned long *)vram2 = (unsigned long)(void*)psxobj;
			}
		}

		data = (byte *)W_CacheLumpNum(psxobj->lump, PU_CACHE, false);
		if (lumpencode[psxobj->lump] == 0)
		{
			tmp = (byte *)tempbuffer;
			decode(data, tmp);
			data = tmp;
		}

		// copy address vram into the object
		psxobj->vptr = (unsigned long*)imgobj;

		frame.x = PagesXY[PageCount][0] + (xcount << 3);
		frame.y = PagesXY[PageCount][1] + (ycount << 4);
		frame.w = ((psxobj->w) << 16) >> 17;
		frame.h = psxobj->h;
		LoadImage(&frame, (unsigned long*)(byte*)(data + 8));

		psxobj->vramx = (xcount << 4);
		psxobj->vramy = (ycount << 4);
		psxobj->vtpage = GetTPage(1, 0, (PageCount + 4) * 128, PagesXY[PageCount][1]);

		xcount += psxobj->vbasex;
		if (xycount < psxobj->vbasey)
			xycount = psxobj->vbasey;
	}
}

void V_ClearBlock(psxobj_t *psxobj)
{
	unsigned long   *vmptr;
	int             x, y;
	short		    xs, ys;

	vmptr = (unsigned long *)psxobj->vptr;

	psxobj->vtpage = 0;		// erase texture page ID.
	xs = psxobj->vbasex;	// xsize of alloc block
	ys = psxobj->vbasey;	// ysize of alloc block

	// erase address block
	for (y = 0; y < ys; y++, vmptr += (MIN_VRAM_ALLOC_SIZE - xs))
	{
		for (x = 0; x < xs; x++, vmptr++)
		{
			*vmptr = 0; // zero block
		}
	}
}

void Valloc_Init(void)
{
	int			    obj, j, i;
	short		    xs, ys;
	psxobj_t	    *psxobj;
	psxobj_t	    **imgobj;
	unsigned long   *vmptr;
	unsigned int    page;
	unsigned int    pagenum;
	unsigned int    flags;

	for (page = 0; page < MAX_DYNAMIC_TPAGE; page = pagenum + 1)
	{
		do
		{
			pagenum = page;
			page = pagenum + 1;
		} while ((V_PagFlags >> (pagenum & 0x1f)) & 1U);

		imgobj = vram_cache[pagenum].imgobj;

		for (obj = 0; obj < MAX_VRAMMEM; obj++, imgobj++)
		{
			psxobj = *imgobj;
			if (psxobj != 0)
			{
				vmptr = (unsigned long *)psxobj->vptr;

				psxobj->vtpage = 0;		// erase texture page ID.
				xs = psxobj->vbasex;	// xsize of alloc block
				ys = psxobj->vbasey;	// ysize of alloc block

				// erase address block
				for (j = 0; j < ys; j++, vmptr += (MIN_VRAM_ALLOC_SIZE - xs))
				{
					for (i = 0; i < xs; i++, vmptr++)
					{
						*vmptr = 0; // zero block
					}
				}
			}
		}
	}

	// Set Page Position
	PageCount = 0;

	flags = V_PagFlags & 1;
	while (flags != 0)
	{
		PageCount = (PageCount + 1) % MAX_DYNAMIC_TPAGE;
		flags = V_PagFlags >> (PageCount & MAX_DYNAMIC_TPAGE) & 1;
	}

	//Reset Positions
	xycount = 0;
	xcount = 0;
	ycount = 0;
}

void Vram_Viewer(int page)//80033938
{
    psxobj_t    **imgobj;
	psxobj_t    *psxobj;
	int         i, xpos, ypos, w, y, h, obj;
	POLY_FT4    *pagepic = (POLY_FT4*) getScratchAddr(128);//1F800200
	LINE_F2     *line = (LINE_F2*) getScratchAddr(128);//1F800200

	setPolyFT4(pagepic);
	setRGB0(pagepic, 128, 128, 128);
	setXYWH(pagepic, 0, 0, 256, 240);
	setUVWH(pagepic, 0, 0, 255, 255);
	pagepic->tpage = GetTPage(1, 0, (page + 4) * 128, PagesXY[page][1]);
	pagepic->clut =palette[0];
	W_AddPrim (pagepic);

	imgobj = vram_cache[page].imgobj;

	for (obj = 0; obj < MAX_VRAMMEM; obj++, imgobj++)
	{
		psxobj = *imgobj;

		if(psxobj)
		{
			y = psxobj->vramy;
			xpos = psxobj->vramx;

			y = (y << 4) - y << 4;
			if(y < 0)
			{
				y += 255;
			}

			h = psxobj->h;
			w = psxobj->w;

			h = (h << 4) - h << 4;
			ypos = y >> 8;
			if(h < 0)
			{
				h += 255;
			}
			h = (h >> 8);

			setLineF2(line);
			setRGB0(line, 255, 0, 0);

			//top line
			setXY2(line, xpos, ypos, xpos + w, ypos);
			W_AddPrim (line);

			//right line
			setXY2(line, xpos + w, ypos, xpos + w, ypos + h);
			W_AddPrim (line);

			//bottom line
			setXY2(line, xpos + w, ypos + h, xpos, ypos + h);
			W_AddPrim (line);

			//left line
			setXY2(line, xpos, ypos + h, xpos, ypos);
			W_AddPrim (line);
		}
	}
}

boolean cancel_link; //0x80077A3C *(r28 + 1580)

unsigned int TempConfiguration[8] = { PAD_TRIANGLE, PAD_CIRCLE, PAD_CROSS, PAD_SQUARE, PAD_L1, PAD_R1, PAD_L2, PAD_R2 };//80073c1c
unsigned int ActualConfiguration[8] = { PAD_TRIANGLE, PAD_CIRCLE, PAD_CROSS, PAD_SQUARE, PAD_L1, PAD_R1, PAD_L2, PAD_R2 };	//80073c3c
unsigned int DefaultConfiguration[8] = { PAD_TRIANGLE, PAD_CIRCLE, PAD_CROSS, PAD_SQUARE, PAD_L1, PAD_R1, PAD_L2, PAD_R2 };	//80073C5C
unsigned int NewConfiguration[8];//0x80078120

void Link_Conection(void)//L800345A0()
{
	/* The following lines enclosed by square brackets are necessary for the current PSYQ SDK libcomb.lib */
	/* This is due to the version of libcomb.lib library used in the original PSXDOOM, it worked differently. */
	/* Also the CompaCTS macros were added as it is the current way to create links between two consoles */
	{
	    /* wait for DSR and CTS line to be asserted */
        /* wait for other PlayStation to be powered-up */
        while((CombSioStatus() & (COMB_DSR|COMB_CTS)) == 0x180 )
        {
            if (PadRead(0) & PAD_SELECT)
            {
                cancel_link = true;
                CombCancelRead(); // Cancel async read request
                return;
            }
        };
	}

	//if (_comb_control(3,0,0) != 0)	//Original line
	if(CombCTS() == 0)  /* Return status of CTS */
    {
        consoleplayer = 0;
        read(fr, recbuf, BUFFSIZE);
        do
        {
            if (PadRead(0) & PAD_SELECT)
			{
				cancel_link = true;
				CombCancelRead(); // Cancel async read request
				return;
			}
        } while (TestEvent(ev_r) == 0);

        //do {} while (_comb_control(3,0,0) == 0);   //Original line
        do {} while (CombCTS() == 0);   /* Return status of CTS */

        write(fw, senbuf, BUFFSIZE);
    }
    else
    {
        consoleplayer = 1;
        write(fw, senbuf, BUFFSIZE);
        read(fr, recbuf, BUFFSIZE);
        do
        {
            if (PadRead(0) & PAD_SELECT)
			{
				cancel_link = true;
				CombCancelRead(); // Cancel async read request
				return;
			}
        } while (TestEvent(ev_r) == 0);
    }

	Sync_Data_Transmission();

	if (consoleplayer == 0)
	{
		senbuf[1] = (char)starttype;
		senbuf[2] = (char)startskill;
		senbuf[3] = (char)startmap;
		*(int*)&senbuf[4] = Get_CfgCode(ActualConfiguration);

		//do {} while (_comb_control(3,0,0) == 0);   //Original line
		do {} while (CombCTS() == 0);/* Return status of CTS */

		write(fw, senbuf, BUFFSIZE);
		read(fr, recbuf, BUFFSIZE);

		do {} while (TestEvent(ev_r) == 0);

		BT_DATA[0] = (buttons_t *)ActualConfiguration;
		BT_DATA[1] = (buttons_t *)Get_CfgByCode(*(int*)&recbuf[4]);
	}
	else
	{
		read(fr, recbuf, BUFFSIZE);

		do {} while (TestEvent(ev_r) == 0);

		starttype = recbuf[1];
		startskill = recbuf[2];
		startmap = recbuf[3];

		BT_DATA[1] = (buttons_t *)ActualConfiguration;
		BT_DATA[0] = (buttons_t *)Get_CfgByCode(*(int*)&recbuf[4]);

		*(int*)&senbuf[4] = Get_CfgCode(ActualConfiguration);

		//do {} while (_comb_control(3,0,0) == 0);   //Original line
		do {} while (CombCTS() == 0);/* Return status of CTS */
		write(fw, senbuf, BUFFSIZE);
	}
	cancel_link = false;
}

boolean Update_Conection(void)//L80034868()
{
	unsigned char up1, up2;
	long side;
	boolean conection;

	up1 = players[0].mo->x ^ players[0].mo->y ^ players[1].mo->x ^ players[1].mo->y;
    up2 = up1 ^ (up1 >> 8);
    up2 = up2 ^ (up1 >> 16);

	senbuf[0] = 170;//171 on final doom
	senbuf[1] = up2;
    senbuf[2] = *(char *)(vblsinframe + consoleplayer);
	*(int*)&senbuf[4] = ticbuttons[consoleplayer];

	Sync_Read_Write();

	if (recbuf[0] == 170 && (recbuf[1] == senbuf[1]))//171 on final doom
	{
        if (consoleplayer == 0)
        {
            ticbuttons[1] = *(int*)&recbuf[4];
        }
        else
        {
            ticbuttons[0] = *(int*)&recbuf[4];
        }

        if (consoleplayer == 0)
            vblsinframe[1] = recbuf[2];
        else
            vblsinframe[0] = recbuf[2];

        conection = false;
	}
    else
    {
        DrawSync(0);

        side = drawside ^ 1;
        MoveImage(&disp[drawside].disp, disp[side].disp.x, disp[side].disp.y);

        //Setup Net Error image obj
        NextTextureCacheIdx();
        TextureCache(&neterrpic);

        DrawImage(neterrpic.vtpage, palette[16], 84, 109, neterrpic.vramx, neterrpic.vramy, neterrpic.w, neterrpic.h);

        UpdateDrawOTag();
        DrawRender();
        Sync_Data_Transmission();

        ticbuttons[1] = 0;
        oldticbuttons[0] = 0;
        ticbuttons[1] = 0;
        oldticbuttons[0] = 0;

        conection = true;
    }

    return conection;
}

void Sync_Data_Transmission(void) //L80034AC0()
{
	int cnt;

    cnt = 0;
    Again:
    do
    {
        senbuf[0] = (char)cnt;

        /* Data transmission */
        Sync_Read_Write();

        if (recbuf[0] == senbuf[0])
        {
            cnt += 1;
            if (cnt < 8) goto Again;
        }

        if (!(cnt < 8)) return;
        cnt = 0;
    } while( true );
}

void Sync_Read_Write (void) //L80034B1C()
{
	int sync;

	do
	{
		if (consoleplayer == 0)
        {
			//do {} while (_comb_control(3,0,0) == 0);   //Original line
			do {} while (CombCTS() == 0);   /* Return status of CTS */

			write(fw, &senbuf, BUFFSIZE);
			read(fr, &recbuf, BUFFSIZE);

			sync = VSync(-1);
			do
            {
				if (TestEvent(ev_r) != 0) return;

			} while ((VSync(-1) - sync) < 300);
		}
		else
        {
			read(fr, &recbuf, BUFFSIZE);

			sync = VSync(-1);
			do
            {
				if (TestEvent(ev_r) != 0)
                {
					//do {} while (_comb_control(3,0,0) == 0);   //Original line
					do {} while (CombCTS() == 0);   /* Return status of CTS */
					write(fw, &senbuf, BUFFSIZE);
					return;
				}

			} while ((VSync(-1) - sync) < 300);
		}

		CombResetError();   /* Reset error bits */
	} while( true );
}

void UpdateDrawOTag(void)//80034C60
{
	if(sys_ot[0] != sys_ot[1])
	{
		*sys_ot[1] = 0xffffff;
		DrawOTag(sys_ot[0]);
	}
	sys_ot[0] = ot;
	sys_ot[1] = ot;
}

/*
==============
=
= Get_CfgCode
=
= Encodes the buttons flags to send them to the link connection
=
==============
*/

unsigned int Get_CfgCode(unsigned int *cfgdata)//L80034CAC
{
    unsigned int *tmpcfg;
	int i, j, cfgcode;

	tmpcfg = TempConfiguration;
	cfgcode = 0;

	for(i = 0; i < 8; i++, cfgdata++)
	{
		for(j = 0; j < 8; j++, tmpcfg++)
		{
			if (*cfgdata == *tmpcfg)
				break;
		}

		cfgcode |= (j << (i << 2));
	}

	return cfgcode;
}

/*
==============
=
= Get_CfgByCode
=
= Decodes the value received from the link connection and turn them into the buttons flags
=
==============
*/

unsigned int *Get_CfgByCode(unsigned int cfgcode)//L80034D0C
{
    unsigned int *tmpcfg;
	unsigned int *newcfg;
	int i, code;

	tmpcfg = TempConfiguration;
	newcfg = NewConfiguration;

	for(i = 0; i < 8; i++)
	{
        code += (((cfgcode >> (i << 2))));
        *newcfg++ = *(tmpcfg + (code & 15));
	}

	return NewConfiguration;
}

/*================================================================== */
/* */
/* W_AddPrim: Add primitive polygons to the buffer (ot), created by williams. */
/* This function does not exist in the code as a function, it should have been a macro define. */
/* */
/*================================================================== */

#define  GP0_GPUREAD ((unsigned long*)  getScratchAddr(1540))
#define  GP1_GPUSTAT ((unsigned long*)  getScratchAddr(1541))

void W_AddPrim (void* prim)
{
	unsigned long *entry, *entry1;
	int length;

	entry1 = sys_ot[1];
    do
    {
        length = getlen(prim);
        entry = sys_ot[1];

        if(sys_ot[1] >= sys_ot[0])
        {
            if((((length+1) + entry) < (ot + (OTSIZE))))
            {
            addprim:
                entry = (length+1) + sys_ot[1];
                sys_ot[1] = entry;

                catPrim(entry1, sys_ot[1]);
                setlen(entry1, length);
                entry = (unsigned long *)prim+1;//skip prim->tag

                //Copy prim to ot
                while (length--)
                {
                    entry1++;
                    *entry1 = *entry++;
                }

                while (sys_ot[0] != sys_ot[1])
                {
                    if((*GP1_GPUSTAT & 0x4000000) == 0)  {break;}

                    entry = sys_ot[0];
                    length = getlen(entry);
                    sys_ot[0] = nextPrim(entry);

                    //Copy address
                    while (length--)
                    {
                        entry++;
                        *GP0_GPUREAD = *entry;
                    }
                }
                return;
            }

            sys_ot[1] = ot;
            catPrim(entry1, sys_ot[1]);
            setlen(entry1, 0);

            entry1 = sys_ot[1];
        }

        if (((length+1) + entry) < sys_ot[0]) goto addprim;

        while (sys_ot[0] != sys_ot[1])
        {
            if((*GP1_GPUSTAT & 0x4000000) == 0)  {break;}

            entry = sys_ot[0];
            length = getlen(entry);
            sys_ot[0] = nextPrim(entry);

            //Copy address
            while (length--)
            {
                entry++;
                *GP0_GPUREAD = *entry;
            }
        }
    } while(1);
}

#if 0 //OLD VERSION
void W_AddPrim (void* prim, unsigned long* addr0, unsigned long* addr1)
{
	unsigned long *entry, *entry1;
	int length;

	entry1 = sys_ot[1];
    do
    {
        length = getlen(prim);
        entry = sys_ot[1];

        if(sys_ot[1] >= sys_ot[0])
        {
            if((((length+1) + entry) < (ot + (OTSIZE))))
            {
            addprim:
                entry = (length+1) + sys_ot[1];
                sys_ot[1] = entry;

                catPrim(entry1, sys_ot[1]);
                setlen(entry1, length);
                entry = (unsigned long *)prim+1;//skip prim->tag

                //Copy prim to ot
                while (length--)
                {
                    entry1++;
                    *entry1 = *entry++;
                }

                if(sys_ot[0] != sys_ot[1])
                {
                    do {
                        if((*addr1 & 0x4000000) == 0)  {break;}

                        entry = sys_ot[0];
                        length = getlen(entry);
                        sys_ot[0] = nextPrim(entry);

                        //Copy address
                        while (length--)
                        {
                            entry++;
                            *addr0 = *entry;
                        }

                    } while(sys_ot[0] != sys_ot[1]);
                }
                return;
            }

            sys_ot[1] = ot;
            catPrim(entry1, sys_ot[1]);
            setlen(entry1, 0);

            entry1 = sys_ot[1];
        }

        if (((length+1) + entry) < sys_ot[0]) goto addprim;

        if(sys_ot[0] != sys_ot[1])
        {
            do
            {
                if((*addr1 & 0x4000000) == 0)  {break;}

                entry = sys_ot[0];
                length = getlen(entry);
                sys_ot[0] = nextPrim(entry);

                //Copy address
                while (length--)
                {
                    entry++;
                    *addr0 = *entry;
                }
            } while (sys_ot[0] != sys_ot[1]);
        }

    } while(1);
}
#endif // 0
/******************END OF MAIN.C*************************/
