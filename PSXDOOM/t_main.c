/* t_main.c -- title intro */

#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

int vframe1 = 0; //80077AB4 iGp000006a4
int vframe2 = 0; //80077AB8 iGp000006a8

psxobj_t loadingpic;//0x80097870
psxobj_t marb01pic;//0x80097890
psxobj_t buttonspic;//0x800978B0
psxobj_t neterrpic; //0x800978d0
psxobj_t pausepic;//0x80097850
psxobj_t titlepic;//0x80097810

/*
=======================
=
= T_Start
=
=======================
*/

void T_Start(void)//L80034EA0()
{
	int lump;

	Valloc_Init();
	Z_FreeTags(mainzone,PU_CACHE);//test

	W_CacheLumpName("LOADING", PU_STATIC, 0);
	ImageToVram(&loadingpic, "LOADING", 0);
	P_LoadingPlaque(&loadingpic, 95, 109, palette[16]);
	S_Lcd_Load(0);

	W_CacheLumpName("MARB01", PU_STATIC, 0);
	W_CacheLumpName("BUTTONS", PU_STATIC, 0);
	W_CacheLumpName("NETERR", PU_STATIC, 0);
	W_CacheLumpName("PAUSE", PU_STATIC, 0);

	ImageToVram(&marb01pic, "MARB01", 0);
	ImageToVram(&buttonspic, "BUTTONS", 0);
	ImageToVram(&neterrpic, "NETERR", 0);
	ImageToVram(&pausepic, "PAUSE", 0);
	ImageToVram(&titlepic, "TITLE", 0);

	lump = R_TextureNumForName("SKY09");
	skytexturep = &textures[lump];
	skypalette = palette[15];

	W_CacheLumpNum(skytexturep->lump, PU_CACHE, true);

	TextureCache(skytexturep);

	y_scroll = 250;

	psxcd_play(CD_TRACK[1], CDVolume);
	do {} while(psxcd_elapsed_sectors() == 0);
}

/*
=======================
=
= T_Stop
=
=======================
*/

void T_Stop(int exit)//L80035070()
{
	S_StartSound(NULL, sfx_barexp);
	psxcd_stop();
}

/*
=======================
=
= T_Ticker
=
=======================
*/

int T_Ticker(void)//L8003509C()
{
    byte *cache, *cachetmp;
    int exit, pixel, cnt, i;

    exit = ga_nothing;

    if (ticbuttons[0] != 0)
    {
        exit = ga_exit;
    }
    else
    {
        vframe1 -= vblsinframe[consoleplayer];
        if (vframe1 <= 0)
        {
            vframe1 = 2;
            if (y_scroll != 0)
            {
                y_scroll--;
                if (y_scroll == 0)
                    last_ticon = ticon;
            }
        }

        vframe2 -= vblsinframe[consoleplayer];
        if (vframe2 <= 0)
        {
            vframe2 = 2;

            //Fire Out
            if (((y_scroll < 50) & (y_scroll ^ 1) & 1) != 0)
            {
                cache = (byte *)(lumpcache[skytexturep->lump].cache) + 8;

                pixel = *(cache + FIREPOS1) - 1;
                if (pixel < 0)
                    pixel = 0;

                cachetmp = (cache + FIREPOS2);
                for (i = (FIRESKY_WIDTH - 1); i >= 0; i--)
                    *cachetmp-- = pixel;
            }
            P_FireSky(skytexturep);
        }

        if (y_scroll == 0)
            exit = -((unsigned int)(ticon - last_ticon < 1800) ^ 1) & 7;
    }

    return exit;
}

/*
=======================
=
= T_Draw
=
=======================
*/

void T_Draw(void)//L80035214()
{
	POLY_FT4 poly1;
	RECT rect;
	int i;
	byte *cache;

	NextTextureCacheIdx();

	// Draw Title Textures
	setPolyFT4(&poly1);
	setRGB0(&poly1, 128, 128, 128);

	//setUV4(p,_u0,_v0,_u1,_v1,_u2,_v2,_u3,_v3)
	setUV4(&poly1,
		0  , 0  ,
		255, 0  ,
		0  , 239,
		255, 239);

	//setXY4(p,_x0,_y0,_x1,_y1,_x2,_y2,_x3,_y3)
	setXY4(&poly1,
		0  , y_scroll,
		255, y_scroll,
		0  , y_scroll + 239,
		255, y_scroll + 239);

	poly1.tpage = titlepic.vtpage;
	poly1.clut = palette[17];

	W_AddPrim(&poly1);// add to order table

	// Draw 4 Fire Textures
	if (skytexturep->index == -1)
	{
		rect.x = (skytexturep->vramx >> 1) + ((skytexturep->vtpage & 15) << 6);
		rect.y = (skytexturep->vramy) + ((skytexturep->vtpage & 16) << 4);
		rect.w = 32;
		rect.h = 128;
		cache = (byte *)(lumpcache[skytexturep->lump].cache) + 8;
		LoadImage(&rect, (unsigned long *)(byte*)(cache));
		skytexturep->index = TextureCacheIdx;
	}

	setPolyFT4(&poly1);
	setRGB0(&poly1, 128, 128, 128);

	//setUV4(p,_u0,_v0,_u1,_v1,_u2,_v2,_u3,_v3)
	setUV4(&poly1,
		skytexturep->vramx     , skytexturep->vramy,
		skytexturep->vramx + 63, skytexturep->vramy,
		skytexturep->vramx     , skytexturep->vramy + 127,
		skytexturep->vramx + 63, skytexturep->vramy + 127);

	//setXY4(p,_x0,_y0,_x1,_y1,_x2,_y2,_x3,_y3)
	setXY4(&poly1,
		0, 116,
		63, 116,
		0, 243,
		63, 243);

	poly1.tpage = skytexturep->vtpage;
	poly1.clut = skypalette;

	for (i = 0; i < 4; i++)
	{
		W_AddPrim(&poly1);// add to order table
		poly1.x0 += 63;
		poly1.x1 += 63;
		poly1.x2 += 63;
		poly1.x3 += 63;
	}

	UpdateDrawOTag();
	DrawRender();
}
