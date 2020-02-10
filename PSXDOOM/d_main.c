/* D_main.c  */

//------------------------------
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

//global gp para checkear funciones
int _gp = 'D' | ('M' << 8) | ('G' << 16) | ('P' << 24);

int	*demo_p = NULL;             //80077418
int	*demobuffer = NULL;         //8007741C
int SfxVolume = 100;	        //80077420
int MusVolume = 100;	        //80077424
int CDVolume = 0x3CFF;	        //80077428

int gamevbls;		            //80077DD0 /* may not really be vbls in multiplayer */
int	gametic;		            //80077E78
int ticsinframe;                //800780C0 /* how many tics since last drawer */
int ticon;			            //80077F74
//int frameon;
int ticbuttons[MAXPLAYERS];		//80077D70, 80077D74
int oldticbuttons[MAXPLAYERS];	//8007803C, 80078040
int	vblsinframe[MAXPLAYERS];	//80077de8 /* range from 4 to 8 */

int	v_sync;                     //80077CC8

#define SetVolPsx(vol) ((int)((vol*0x7f)/100))

skill_t    startskill = sk_medium;  //8007742C
int        startmap   = 1;          //80077430
gametype_t starttype  = gt_single;  //80077434

byte tempbuffer[0x10000];	//80098528 Psx Doom

mobj_t	emptymobj;  //0x800a9c14

//Por ahora
int			switchlist[MAXSWITCHES * 2];
button_t	buttonlist[MAXBUTTONS];//800975B0

anim_t	    anims[MAXANIMS];
thinker_t	thinkercap;	/* both the head and tail of the thinker list */    //80096378
mobj_t		mobjhead;	/* head and tail of mobj list */                    //800A8C74,
buttons_t   *BT_DATA[MAXPLAYERS];//80077DF4, 80077DF8
line_t	    *linespeciallist[MAXLINEANIMS];//0x800973a4

void D_DoomMain (void) //L80012274()
{
	byte		*data;
	data = (byte *)tempbuffer;

	//printf("MASTER EDITION BY GEC\n");

	PSX_INIT();

	/* WMS Sound System Init */
	PsxSoundInit(SetVolPsx(SfxVolume), SetVolPsx(MusVolume), data);

	Z_Init();
	Init_Vram_Cache();
	W_Init();
	R_Init();
	ST_Init();

	gamevbls = 0;
	gametic = 0;

	ticsinframe = 0;
	ticon = 0;

	ticbuttons[0] = ticbuttons[1] = 0;
	oldticbuttons[0] = oldticbuttons[1] = 0;

	while (1)
	{
		if (RunTitle() != ga_exit)
        {
			if (RunDemo((char *)_DEMO1_LMP) != ga_exit)
			{
				if (RunCredits() != ga_exit)
				{
					if (RunDemo((char *)_DEMO2_LMP) != ga_exit)
					{
						continue;
					}
				}
			}
		}

		do{} while (RunMenu() != ga_timeout);
	}
}

int RunLegal(void)//L800123A4()
{
	int exit;
	exit = MiniLoop(L_Start, L_Stop, L_Ticker, L_Draw);
	return exit;
}

int RunTitle(void)//L800123E4()
{
	int exit;
	exit = MiniLoop(T_Start, T_Stop, T_Ticker, T_Draw);
	return exit;
}

int RunDemo(char *demoname)//L80012424()
{
	int pfile;
	int exit;

	demo_p = Z_Alloc(0x4000, PU_STATIC, 0);

	pfile = OpenFile(demoname);
	ReadFile(pfile, demo_p, 0x4000);
	CloseFile(pfile);

	exit = G_PlayDemoPtr();
	Z_Free(demo_p);

	return exit;
}

int RunCredits(void)//L800124A8()
{
	int		exit;
	exit = MiniLoop(C_Start, C_Stop, C_Ticker, C_Draw);
	return exit;
}


#include <stdarg.h> //va_list|va_start|va_end

int debugX, debugY;//80077E5C|uGp00000a4c, 80077E68|uGp00000a58
extern psxobj_t statuspic;  //800A92CC

void D_DebugSetPrintPos(int x, int y)//L800124E8()
{
	debugX = x;
	debugY = y;
}

void D_DebugPrint(const char *text, ...)//L800124F8()
{
	char buffer[256];
	va_list args;
	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT    *debugsprite = (SPRT*) getScratchAddr(128);//1F800200

	SetDrawMode(drawmode, 0, 0, statuspic.vtpage, NULL);
	W_AddPrim(drawmode);// add to order table

	SetSprt(debugsprite);
	SetSemiTrans(debugsprite, 0);
	SetShadeTex(debugsprite, 0);

	setRGB0(debugsprite, 128, 128, 128);
	debugsprite->clut = palette[0];

	va_start(args, text);
	D_vsprintf(buffer, text, args);
	va_end(args);

	ST_DrawMessage(debugX, debugY, buffer);
	debugY += 8;
}

#define WORDMASK	3

/*int abs(int x)
{
	if (x<0)
		return -x;
	return x;
}*/

#define abs(x) ((x) < 0 ? -(x) : (x))

/*
====================
=
= D_memset
=
====================
*/

void D_memset(void *dest, int val, int count)//L80012850()
{
	byte	*p;
	int		*lp;

	/* round up to nearest word */
	p = dest;
	while ((int)p & WORDMASK)
	{
		if (--count < 0)
			return;
		*p++ = val;
	}

	/* write 32 bytes at a time */
	lp = (int *)p;
	val = (val << 24) | (val << 16) | (val << 8) | val;
	while (count >= 32)
	{
		lp[0] = lp[1] = lp[2] = lp[3] = lp[4] = lp[5] = lp[6] = lp[7] = val;
		lp += 8;
		count -= 32;
	}

	/* finish up */
	p = (byte *)lp;
	while (count--)
		*p++ = val;
}


void D_memcpy(void *dest, void *src, int count)//L8001290C()
{
	byte	*d, *s;

	d = (byte *)dest;
	s = (byte *)src;
	while (count--)
		*d++ = *s++;
}


void D_strncpy(char *dest, char *src, int maxcount)//L80012940()
{
	byte	*p1, *p2;
	p1 = (byte *)dest;
	p2 = (byte *)src;
	while (maxcount--)
		if (!(*p1++ = *p2++))
			return;
}

int D_strncasecmp(char *s1, char *s2, int len)//L8001297C()
{
	while (*s1 && *s2)
	{
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
		if (!--len)
			return 0;
	}
	if (*s1 != *s2)
		return 1;
	return 0;
}

void D_strupr(char *s)//L800129D4()
{
	char	c;

	while ((c = *s) != 0)
	{
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
		*s++ = c;
	}
}

/*
===============
=
= M_Random
=
= Returns a 0-255 number
=
===============
*/

unsigned char rndtable[256] = { //0x80058888
	0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241 ,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224 ,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95 ,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226 ,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36 ,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106 ,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136 ,
	120, 163, 236, 249
};

int	rndindex = 0;//80077438|uGp0000002c
int prndindex = 0;//8007743C||uGp00000028

int P_Random(void)//L80012A18()
{
	prndindex = (prndindex + 1) & 0xff;
	return rndtable[prndindex];
}

int M_Random(void)//L80012A44()
{
	rndindex = (rndindex + 1) & 0xff;
	return rndtable[rndindex];
}

void M_ClearRandom(void) // L80012A70()
{
	rndindex = prndindex = 0;
}

void M_ClearBox(fixed_t *box)//L80012A80()
{
	box[BOXTOP] = box[BOXRIGHT] = MININT;
	box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y)//L80012AA0()
{
	if (x<box[BOXLEFT])
		box[BOXLEFT] = x;
	else if (x>box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y<box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	else if (y>box[BOXTOP])
		box[BOXTOP] = y;
}

void M_AddToBox2(fixed_t *box, fixed_t x, fixed_t y)//L80012B10()
{
	if (x<box[BOXLEFT])
		box[BOXLEFT] = x;
	if (x>box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y<box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	if (y>box[BOXTOP])
		box[BOXTOP] = y;
}


/*
===============
=
= MiniLoop
=
===============
*/

int MiniLoop(void(*start)(void), void(*stop)(int), int(*ticker)(void), void(*drawer)(void))//80012B78
{
	int		exit;
	int		buttons;

	if (netgame != gt_single)
		Sync_Data_Transmission();

	gameaction = ga_nothing;
	gamevbls = 0;
	gametic = 0;
	ticon = 0;
	ticsinframe = 0;

	/* */
	/* setup (cache graphics, etc) */
	/* */
	start();

	drawsync1 = 0;
	drawsync2 = VSync(-1);
	//printf("RUN Mini Loop\n");
	while (1)
	{
		vblsinframe[consoleplayer] = drawsync1;

		// get buttons for next tic
		oldticbuttons[0] = ticbuttons[0];
		oldticbuttons[1] = ticbuttons[1];

		buttons = PadRead(0);
		ticbuttons[consoleplayer] = buttons;

		if (netgame != gt_single)
		{
		    if (Update_Conection() != 0)
            {
                gameaction = ga_warped;
                exit = ga_warped; // hack for NeXT level reloading and net error
                break;
            }
		}
		//Read|Write demos
		else if (demorecording || demoplayback)
        {
            if (demoplayback)
            {
                if (buttons & PAD_ALL)
                {
                    exit = ga_exit;
                    break;
                }

                buttons = BIGLONG(*demobuffer++);
                ticbuttons[consoleplayer] = buttons;
            }

            if (demorecording)
            {
                *demobuffer++ = BIGLONG(buttons);
            }

            if ((buttons & PAD_START) || ((demobuffer - demo_p) >= 0x4000))
            {
                exit = ga_exitdemo;
                break;
            }
        }

		ticon += vblsinframe[0];
		if (ticsinframe < (ticon >> 2))
		{
			gametic+=1;
			ticsinframe = (ticon >> 2);
		}

		exit = ticker();
		if (exit != ga_nothing)
			break;

		drawer();

		if (gamevbls < gametic)
		{
			S_UpdateSounds();
		}

		gamevbls = gametic;
	}

	stop(exit);

	oldticbuttons[1] = ticbuttons[1];
	oldticbuttons[0] = ticbuttons[0];

	return exit;
}
