/* st_main.c -- status bar */

#include "doomdef.h"
#include "st_main.h"
#include "r_local.h"

stbar_t	stbar; //800984F4
psxobj_t statuspic;//0x800a92cc

int     weaponowned[NUMWEAPONS] = {0, 1, 2, 3, 4, 5, 6, 7, 0};  //80073ebc
short	micronums_x[NUMMICROS] = { 199, 212, 225, 238, 199, 212, 225, 238 };//80073ee0
short	micronums_y[NUMMICROS] = { 204, 204, 204, 204, 216, 216, 216, 216 };//80073ef0
short	card_y[NUMCARDS] = { 204, 212, 220, 204, 212, 220 };//80073f00

sbflash_t	flashCards[NUMCARDS];	/* INFO FOR FLASHING CARDS & SKULLS */  //800a9298

boolean facedraw;           //uGp00000b48
int		facetics;           //uGp00000b4c
int		newface;            //uGp00000a40
boolean	gibdraw;            //uGp00000a74
boolean doSpclFace;         //uGp000008e8
spclface_e	spclFaceType;   //sGp00000924
facedata_t *facegraphic;    //0x80078054|puGp00000c44

facedata_t facedata[NUMFACES] =//0x80073c98
{
	{ 118, 202,   0,  41, 19, 29 },// STFST01 - 0
	{ 118, 202,  20,  41, 19, 29 },// STFST02 - 1
	{ 118, 202, 234, 137, 19, 29 },// STFST00 - 2
	{ 118, 202,  40,  41, 21, 31 },// STFTL00 - 3
	{ 118, 202,  62,  41, 21, 31 },// STFTR00 - 4
	{ 118, 202,  84,  41, 19, 31 },// STFOUCH0 - 5
	{ 118, 202, 104,  41, 19, 31 },// STFEVL0 - 6   EVILFACE
	{ 118, 202, 124,  41, 19, 31 },// STFKILL0 - 7
	{ 118, 202, 144,  41, 19, 31 },// STFST11 - 8
	{ 118, 202, 164,  41, 19, 31 },// STFST10 - 9
	{ 118, 202, 184,  41, 19, 31 },// STFST12 - 10
	{ 118, 202, 204,  41, 20, 31 },// STFTL10 - 11
	{ 118, 202, 226,  41, 21, 31 },// STFTR10 - 12
	{ 118, 202,   0,  73, 19, 31 },// STFOUCH1 - 13
	{ 118, 202,  20,  73, 19, 31 },// STFEVL1 - 14
	{ 118, 202,  40,  73, 19, 31 },// STFKILL1 - 15
	{ 118, 202,  60,  73, 19, 31 },// STFST21 - 16
	{ 118, 202,  80,  73, 19, 31 },// STFST20 - 17
	{ 118, 202, 100,  73, 19, 31 },// STFST22 - 18
	{ 118, 202, 120,  73, 22, 31 },// STFTL20 - 19
	{ 118, 202, 142,  73, 22, 31 },// STFTR20 - 20
	{ 118, 202, 166,  73, 19, 31 },// STFOUCH2 - 21
	{ 118, 202, 186,  73, 19, 31 },// STFEVL2 - 22
	{ 118, 202, 206,  73, 19, 31 },// STFKILL2 - 23
	{ 118, 202, 226,  73, 19, 31 },// STFST31 - 24
	{ 118, 202,   0, 105, 19, 31 },// STFST30 - 25
	{ 118, 202,  20, 105, 19, 31 },// STFST32 - 26
	{ 118, 202,  40, 105, 23, 31 },// STFTL30 - 27
	{ 118, 202,  64, 105, 23, 31 },// STFTR30 - 28
	{ 118, 202,  88, 105, 19, 31 },// STFOUCH3 - 29
	{ 118, 202, 108, 105, 19, 31 },// STFEVL3 - 30
	{ 118, 202, 128, 105, 19, 31 },// STFKILL3 - 31
	{ 118, 202, 148, 105, 19, 31 },// STFST41 - 32
	{ 118, 202, 168, 105, 19, 31 },// STFST40 - 33
	{ 118, 202, 188, 105, 19, 31 },// STFST42 - 34
	{ 118, 202, 208, 105, 24, 31 },// STFTL40 - 35
	{ 118, 202, 232, 105, 23, 31 },// STFTR40 - 36
	{ 118, 202,   0, 137, 18, 31 },// STFOUCH4 - 37
	{ 118, 202,  20, 137, 19, 31 },// STFEVL4 - 38
	{ 118, 202,  40, 137, 19, 31 },// STFKILL4 - 39
	{ 118, 202,  60, 137, 19, 31 },// STFGOD0 - 40  GODFACE
	{ 118, 202,  80, 137, 19, 31 },// STFDEAD0 - 41 DEADFACE
	{ 118, 202, 100, 137, 19, 30 },// STSPLAT0 - 42 FIRSTSPLAT
	{ 114, 201, 120, 137, 27, 30 },// STSPLAT1 - 43
	{ 114, 204, 148, 137, 28, 30 },// STSPLAT2 - 44
	{ 114, 204, 176, 137, 28, 30 },// STSPLAT3 - 45
	{ 114, 204, 204, 137, 28, 30 } // STSPLAT4 - 46
};

symboldata_t symboldata[] = // 80073db4
{
	{   0, 195,  11,  16 },// 0 - 0
	{  12, 195,  11,  16 },// 1 - 1
	{  24, 195,  11,  16 },// 2 - 2
	{  36, 195,  11,  16 },// 3 - 3
	{  48, 195,  11,  16 },// 4 - 4
	{  60, 195,  11,  16 },// 5 - 5
	{  72, 195,  11,  16 },// 6 - 6
	{  84, 195,  11,  16 },// 7 - 7
	{  96, 195,  11,  16 },// 8 - 8
	{ 108, 195,  11,  16 },// 9 - 9
	{ 232, 195,  11,  16 },// - - 10
	{ 120, 195,  11,  15 },// % - 11
	{   0, 211,   7,  16 },// ! - 12
	{   8, 211,   7,  16 },// . - 13
	{  16, 211,  15,  16 },// A - 14
	{  32, 211,  13,  16 },// B - 15
	{  46, 211,  12,  16 },// C - 16
	{  60, 211,  13,  16 },// D - 17
	{  74, 211,  13,  16 },// E - 18
	{  88, 211,  13,  16 },// F - 19
	{ 102, 211,  13,  16 },// G - 20
	{ 116, 211,  13,  16 },// H - 21
	{ 130, 211,   6,  16 },// I - 22
	{ 136, 211,  12,  16 },// J - 23
	{ 148, 211,  14,  16 },// K - 24
	{ 162, 211,  13,  16 },// L - 25
	{ 176, 211,  15,  16 },// M - 26
	{ 192, 211,  15,  16 },// N - 27
	{ 208, 211,  13,  16 },// O - 28
	{ 222, 211,  13,  16 },// P - 29
	{ 236, 211,  13,  16 },// Q - 30
	{   0, 227,  13,  16 },// R - 31
	{  14, 227,  13,  16 },// S - 32
	{  28, 227,  14,  16 },// T - 33
	{  42, 227,  13,  16 },// U - 34
	{  56, 227,  15,  16 },// V - 35
	{  72, 227,  15,  16 },// W - 36
	{  88, 227,  15,  16 },// X - 37
	{ 104, 227,  13,  16 },// Y - 38
	{ 118, 227,  13,  16 },// Z - 39
	{ 132, 230,  13,  13 },// a - 40
	{ 146, 230,  12,  13 },// b - 41
	{ 158, 230,  11,  13 },// c - 42
	{ 170, 230,  11,  13 },// d - 43
	{ 182, 230,  10,  13 },// e - 44
	{ 192, 230,  11,  13 },// f - 45
	{ 204, 230,  11,  13 },// g - 46
	{ 216, 230,  12,  13 },// h - 47
	{ 228, 230,   5,  13 },// i - 48
	{ 234, 230,  10,  13 },// j - 49
	{   0, 243,  12,  13 },// k - 50
	{  12, 243,   9,  13 },// l - 51
	{  22, 243,  13,  13 },// m - 52
	{  36, 243,  13,  13 },// n - 53
	{  50, 243,  11,  13 },// o - 54
	{  62, 243,  11,  13 },// p - 55
	{  74, 243,  11,  13 },// q - 56
	{  86, 243,  11,  13 },// r - 57
	{  98, 243,  12,  13 },// s - 58
	{ 112, 243,  11,  13 },// t - 59
	{ 124, 243,  11,  13 },// u - 60
	{ 136, 243,  13,  13 },// v - 61
	{ 150, 243,  13,  13 },// w - 62
	{ 164, 243,  13,  13 },// x - 63
	{ 178, 243,  13,  13 },// y - 64
	{ 192, 243,  13,  13 } // z - 65
};

/*
====================
=
= ST_Init
=
= Locate and load all needed graphics
====================
*/

void ST_Init (void)//L800382D4()
{
    if (PageCount != 0)
		I_Error("ST_Init: initial texture cache foulup\n");

	ImageToVram(&statuspic, "STATUS", 0);

	if (PageCount != 0)
		I_Error("ST_Init: final texture cache foulup\n");

	xcount = 0;
	ycount = 0;
	PageCount = 1;
	xycount = 0;
	V_PagFlags |= 1;// vram page pos

	Z_FreeTags(mainzone, PU_CACHE);

	//printf("ST_Init: DONE\n");
}

/*================================================== */
/* */
/*  Init this stuff EVERY LEVEL */
/* */
/*================================================== */
void ST_InitEveryLevel(void)//L8003838C()
{
	int		i;

	stbar.gotgibbed = false;
	stbar.specialFace = f_none;
	stbar.messagedelay = 0;

	for (i = 0; i < NUMCARDS; i++)
	{
		stbar.tryopen[i] = false;
		flashCards[i].active = false;
	}

	doSpclFace = false;
	gibdraw = false;	/* DON'T DRAW GIBBED HEAD SEQUENCE */

	/* force everything to be updated on next ST_Update */
	facedraw = true;
	facetics = 0;
	facegraphic = &facedata[0];
}


/*
====================
=
= ST_Ticker
=
====================
*/

void ST_Ticker (void)//L80038404()
{
	player_t    *player;
	int		    ind, base;

	player = &players[consoleplayer];

	/* */
	/* Animate face */
	/* */
	if (--facetics <= 0)
	{
		facetics = M_Random ()&15;
		newface = M_Random ()&3;
		if (newface == 3)
			newface = 1;
		doSpclFace = false;
	}

	/* */
	/* Draw special face? */
	/* */
	if (stbar.specialFace)
	{
		doSpclFace = true;
		spclFaceType = stbar.specialFace;
		facetics = TICRATE;
		stbar.specialFace = f_none;
	}

	/* */
	/* Did we get gibbed? */
	/* */
	if (stbar.gotgibbed)
	{
        stbar.gibdelay = GIBTIME;
        stbar.gibframe = 0;
        stbar.gotgibbed = false;
        gibdraw = true;
	}

	/* */
	/* Animate gibbed face */
	/* */
    if (gibdraw && (--stbar.gibdelay <= 0))
    {
        stbar.gibdelay = GIBTIME;
        stbar.gibframe += 1;
        if (stbar.gibframe >= 5)
        {
            gibdraw = false;
            facedraw = false;
        }
    }

	/* */
	/* Message initialization */
	/* */
    if (player->message)
    {
        stbar.message = player->message;
        stbar.messagedelay = 75;
        player->message = NULL;
    }

    /* */
	/* Countdown time for the message */
	/* */
    if (stbar.messagedelay)
        stbar.messagedelay--;

	/* */
	/* Tried to open a CARD or SKULL door? */
	/* */
	for (ind = 0; ind < NUMCARDS; ind++)
	{
		/* CHECK FOR INITIALIZATION */
		if (stbar.tryopen[ind])
		{
			stbar.tryopen[ind] = false;
			flashCards[ind].active = true;
			flashCards[ind].delay = FLASHDELAY;
			flashCards[ind].times = FLASHTIMES+1;
			flashCards[ind].doDraw = false;
		}

		/* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
		if (flashCards[ind].active && !--flashCards[ind].delay)
		{
			flashCards[ind].delay = FLASHDELAY;
			flashCards[ind].doDraw ^= 1;
			if (!--flashCards[ind].times)
				flashCards[ind].active = false;
			if (flashCards[ind].doDraw && flashCards[ind].active)
				S_StartSound(NULL,sfx_itemup);
		}
	}

	/* */
	/* God mode cheat */
	/* */
	if ((player->cheats & CF_GODMODE) || player->powers[pw_invulnerability])
	{
		stbar.face = GODFACE;
	}
	else
	{
	    /* */
	    /* Draw gibbed head */
	    /* */
		if (gibdraw)
		{
			stbar.face = stbar.gibframe + FIRSTSPLAT;
		}
		/* */
        /* face change */
        /* */
		else
		{
			if (player->health)
			{
				if (doSpclFace)
				{
					base = player->health / 20;
					base = (base < 4) ? (4 - base) * 8 : 0;
					stbar.face = spclFaceType + base;
				}
				else
				{
					base = player->health / 20;
					base = (base < 4) ? (4 - base) * 8 : 0;
					stbar.face = newface + base;
				}
			}
			else
			{
				stbar.face = DEADFACE;
			}
		}
	}

	facegraphic = &facedata[stbar.face];

	/* */
	/* Do red-/gold-shifts from damage/items */
	/* */
	ST_doPaletteStuff();
}


/*
====================
=
= ST_Drawer
=
====================
*/
extern char mapnames[][32];

#if SHOWFPS
extern int fps;
#endif // _SHOWFPS

void ST_Drawer (void)//L80038888();
{
    //psxobj_t	*spr;
	char		text[64];
	RECT		area;
	player_t	*player;
	weapontype_t weapon;
	int ammo, ind, xpos, ypos;

	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT    *sprtstat = (SPRT*) getScratchAddr(128);//1F800200

	player = &players[consoleplayer];

	setRECT(&area, 0, 0, 0, 0);
	SetDrawMode(drawmode, 0, 0, statuspic.vtpage, &area);
	W_AddPrim(drawmode);// add to order table

	/* */
	/* make sprite status */
	/* */
	setSprt(sprtstat);
	setShadeTex(sprtstat, 1);
	setRGB0(sprtstat, 128, 128, 128);
	sprtstat->clut = palette[16];

	/* */
	/* Draw Text Message */
	/* */
	if (stbar.messagedelay > 0)
	{
		ST_DrawMessage(2, 193, stbar.message);
	}
	else if (player->automapflags & AF_ACTIVE)
	{
	    /* */
        /* Draw Map Name Message */
        /* */
		sprintf(text, "LEVEL %d:%s", gamemap, mapnames[gamemap-1]);
		ST_DrawMessage(2, 193, text);
	}

	/* */
	/* Draw StatusBar Graphic */
	/* */
	setXY0(sprtstat, 0, 200);
	setUV0(sprtstat, 0, 0);
	setWH(sprtstat, 256, 40);
	W_AddPrim(sprtstat);// add to order table

	weapon = player->pendingweapon;

	if (weapon == wp_nochange)
		weapon = player->readyweapon;

	if (weaponinfo[weapon].ammo == am_noammo)
		ammo = 0;
	else
		ammo = player->ammo[weaponinfo[weapon].ammo];

	/* */
	/* Ammo */
	/* */
	ST_DrawValue(28, 204, ammo);

	/* */
	/* Health */
	/* */
	ST_DrawValue(71, 204, player->health);

	/* */
	/* Armor */
	/* */
	ST_DrawValue(168, 204, player->armorpoints);

	/* */
	/* Cards & skulls */
	/* */
	xpos = 114;
	ypos = 184;
	for (ind = 0; ind < NUMCARDS; ind++)
	{
        if (player->cards[ind] || (flashCards[ind].active && flashCards[ind].doDraw))
        {
            /* */
            /* Draw Keys Graphics */
            /* */
            setXY0(sprtstat, 100, card_y[ind]);
            setUV0(sprtstat, xpos, ypos);
            setWH(sprtstat, 11, 8);
            W_AddPrim(sprtstat);// add to order table
        }
		xpos += 11;
	}

	/* */
	/* Weapons & level */
	/* */
	if (netgame != gt_deathmatch)
	{
	    /* */
		/* Number Box Graphic */
		/* */
		setXY0(sprtstat, 200, 205);
		setUV0(sprtstat, 180, 184);
		setWH(sprtstat, 51, 23);
		W_AddPrim(sprtstat);// add to order table

		/* */
		/* Numbers Graphic */
		/* */
		xpos = 232;
		ypos = 184;
		for (ind = wp_shotgun; ind < NUMMICROS; ind++)
		{
			if (player->weaponowned[ind])
			{
				setXY0(sprtstat, micronums_x[ind] + 5, micronums_y[ind] + 3);
				setUV0(sprtstat, xpos, ypos);
				setWH(sprtstat, 4, 6);
				W_AddPrim(sprtstat);// add to order table
			}
			xpos += 4;
		}

		/* */
		/* White Selector Box Graphic */
		/* */
		setXY0(sprtstat, micronums_x[weaponowned[weapon]], micronums_y[weaponowned[weapon]]);
		setUV0(sprtstat, 164, 192);
		setWH(sprtstat, 12, 12);
		W_AddPrim(sprtstat);// add to order table
	}
	/* */
	/* Or, frag counts! */
	/* */
	else
	{
	    /* */
		/* Frags Graphic */
		/* */
		setXY0(sprtstat, 209, 221);
		setUV0(sprtstat, 208, 243);
		setWH(sprtstat, 33, 8);
		W_AddPrim(sprtstat);// add to order table

		/* */
		/* Frag Count */
		/* */
		ST_DrawValue(225, 204, player->frags);
	}

	/* */
	/* face change */
	/* */
	if (facedraw)
	{
	    /* */
		/* Face Graphic */
		/* */
		setXY0(sprtstat, facegraphic->xpos, facegraphic->ypos);
		setUV0(sprtstat, facegraphic->x, facegraphic->y);
		setWH(sprtstat, facegraphic->w, facegraphic->h);
		W_AddPrim(sprtstat);// add to order table
	}

	/* */
	/* Drawing cheat "Warp To Level and Vram_Viewer" */
	/* */
	if (gamepaused)
		ST_CheatDraw();


    #if SHOWFPS
    fps = (60-((fps)/60));
    fps = fps > 0? fps:0;
    D_DebugSetPrintPos(10, 10);
    D_DebugPrint("fps %d", fps);
    #endif // SHOWFPS
}


void ST_DrawValue(int x, int y, int value)//L8003A144()
{
	int index, i;
	unsigned char number[16];
	boolean minus;
	int xpos;
	SPRT    *sprtstat = (SPRT*) getScratchAddr(128);//1F800200

	minus = false;

	if (value < 0)
	{
		minus = true;
		value = -value;// absolute value
	}

	for (index = 0; index < 16; index++)
	{
		number[index] = value % 10;
		value /= 10;
		if (!value) break;
	}

	setWH(sprtstat, 11, 16);

	xpos = x;
	for (i = 0; i <= index; i++)
	{
		setXY0(sprtstat, xpos, y);
		setUV0(sprtstat, symboldata[number[i]].x, 195);
		W_AddPrim(sprtstat);// add to order table
		xpos -= 11;
	}

	if (minus)
	{
		setXY0(sprtstat, xpos, y);
		setUV0(sprtstat, symboldata[10].x, 195);
		W_AddPrim(sprtstat);// add to order table
	}
}

void ST_DrawMessage(int x, int y, char *text)//L8003A750()
{
	unsigned int c,cc;
	unsigned int vx, vy, xpos;
	SPRT    *sprtstat = (SPRT*) getScratchAddr(128);//1F800200

	setWH(sprtstat, 8, 8);

	while (*text != 0)
	{
		c = *text;

		if ((byte)(c - 'a') < 26) { c -= 32; }

		cc = c - 33;
		if (cc < '@')
		{
			vy = cc;
			if (cc < 0) { vy = cc - 2; }

			vx = cc << 3;
			vy = ((vy >> 5) << 3) + 168;

			setXY0(sprtstat, x, y);
			setUV0(sprtstat, vx, vy);
			W_AddPrim(sprtstat);// add to order table
		}

		text++;
		x += 8;
	}
}

extern int 	warpmap;        //80078094, iGp00000c84
extern int  Vram_page;   //80077D00, iGp000008f0

void ST_CheatDraw(void)//L8003AA80()
{
	char		text[64];
	player_t	*p;
	POLY_F4     *backgroud = (POLY_F4*) getScratchAddr(128);//1F800200

	p = &players[consoleplayer];

	//Draw Pause Graphic
	if (!(p->cheats & CF_PAUSE) )
	{
		DrawStaticImage(&pausepic, 107, 108, palette[0]);
	}

	if (p->cheats & CF_WARPLEVEL)
	{
		sprintf(text, "warp to level %d", warpmap);
		ST_DrawText(-1, 40, text);
		ST_DrawText(-1, 60, mapnames[warpmap-1]);
	}
	else if (p->cheats & CF_VRAMVIEWER)
	{
		//Black Background for Vram_Viewer
		setPolyF4(backgroud);
		setRGB0(backgroud, 0, 0, 0);
		setXY4(backgroud, 0, 0, 256, 0, 0, 240, 256, 240);
		W_AddPrim(backgroud);// add to order table

		Vram_Viewer(Vram_page);
	}
}

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS		1
#define STARTBONUSPALS		9
#define NUMREDPALS			8
#define NUMBONUSPALS		4
// Radiation suit, green shift.
#define RADIATIONPAL		13
#define INVULNERABILITYPAL	14	//Doom psx

void ST_doPaletteStuff(void)//L8003AE6C()
{
	int		palette_;
	int		cnt;
	int		bzc;
	player_t	*plyr;

	plyr = &players[consoleplayer];

	cnt = plyr->damagecount;

	if (plyr->powers[pw_strength])
	{
		// slowly fade the berzerk out
		bzc = 12 - (plyr->powers[pw_strength] >> 6);

		if (bzc > cnt)
			cnt = bzc;
	}

	//Normal Palette
	palette_ = 0;

	// Handling colormaps.
	viewlighting = true;

	if (plyr->powers[pw_infrared] >= 61 || plyr->powers[pw_infrared] & 8)
	{
		viewlighting = false;
	}

	if (plyr->powers[pw_invulnerability] >= 61 || plyr->powers[pw_invulnerability] & 8)
	{
		viewlighting = false;
		palette_ = INVULNERABILITYPAL;
	}
	else if (cnt)
	{
		palette_ = (cnt + 7) >> 3;

		if (palette_ >= NUMREDPALS)
			palette_ = NUMREDPALS - 1;

		palette_ += STARTREDPALS;
	}
	else if(plyr->powers[pw_ironfeet] >= 61 || plyr->powers[pw_ironfeet] & 8)
	{
		palette_ = RADIATIONPAL;
	}
	else if (plyr->bonuscount)
	{
		palette_ = (plyr->bonuscount + 7) >> 3;

		if (palette_ >= NUMBONUSPALS)
			palette_ = NUMBONUSPALS - 1;

		palette_ += STARTBONUSPALS;
	}

	palettebase = palette[palette_];
}

int ST_GetTextCenterX(char *text)//L8003AFB4()
{
	int c;
	int width;

	width = 0;
	while (*text)
	{
		c = *text;

		if ((byte)(c - 'A') < 26)
		{
			c -= 51;
		}
		else if ((byte)(c - 'a') < 26)
		{
			c -= 57;
		}
		else if ((byte)(c - '0') < 10)
		{
			c -= '0';
		}
		else if (c == '%')
		{
			c = 11;
		}
		else if (c == '!')
		{
			c = 12;
		}
		else if (c == '.')
		{
			c = 13;
		}
		else if (c == '-')
		{
			c = 10;
		}
		else
		{
			width += 6; // space
			text++;
			continue;
		}

		width += symboldata[c].w;
		text++;
	}

	return (((256 - width) * 128) / 256);
}

void ST_DrawText(int x, int y, char *text)//L8003B0A0()
{
	RECT	area;
	int		xpos;
	int		ypos;
	int		c;

	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT    *sprtstat = (SPRT*) getScratchAddr(128);//1F800200

	setRECT(&area, 0, 0, 0, 0);
	SetDrawMode(drawmode, 0, 0, statuspic.vtpage, &area);
	W_AddPrim(drawmode);// add to order table

	/* make sprite status*/
	SetSprt(sprtstat);
	setShadeTex(sprtstat, 1);
	setRGB0(sprtstat, 128, 128, 128);
	sprtstat->clut = palette[16];

	xpos = x;
	if (xpos == -1)//Get Center Text Position
	{
		xpos = ST_GetTextCenterX(text);
	}

	while (*text)
	{
		c = *text;
		ypos = y;

		if ((byte)(c - 'A') < 26)
		{
			c -= 51;
		}
		else if ((byte)(c - 'a') < 26)
		{
			c -= 57;
			ypos = y + 3;
		}
		else if ((byte)(c - '0') < 10)
		{
			c -= '0';
		}
		else if (c == '%')
		{
			c = 11;
		}
		else if (c == '!')
		{
			c = 12;
		}
		else if (c == '.')
		{
			c = 13;
		}
		else if (c == '-')
		{
			c = 10;
		}
		else
		{
			xpos += 6; // space
			text++;
			continue;
		}

		setWH(sprtstat, symboldata[c].w, symboldata[c].h);
		setXY0(sprtstat, xpos, ypos);
		setUV0(sprtstat, symboldata[c].x, symboldata[c].y);
		W_AddPrim(sprtstat);// add to order table

		xpos += symboldata[c].w;
		text++;
	}
}
