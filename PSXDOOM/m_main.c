/* m_main.c -- main menu */

#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

unsigned short STARTY[4] = {91, 133, 158, 200};//80077A64 sGp00000654
#define CURSORX    50
#define CURSORY(y) (STARTY[y])

char gametypenames[][16] = //80073B0C
{
	"Single Player",
	"Cooperative",
	"Deathmatch"
};

char skillnames[][16] = //80073B3C
{
	"I am a Wimp",
	"Not Too Rough",
	"Hurt Me Plenty",
	"Ultra Violence",
	"Nightmare"//[GEC] Master Edition
};

typedef enum
{
	single,
	coop,
	dmatch,
	NUMMODES
} playmode_t;


psxobj_t backpic;//0x800977f0
psxobj_t doompic;//0x80097830
psxobj_t connectpic;//0x800978F0

/*
=================
=
= RunMenu
=
=================
*/

int RunMenu(void)//L8003592C()
{
	do
    {
		if (MiniLoop(M_Start, M_Stop, M_Ticker, M_Drawer) == ga_timeout)
			return ga_timeout;

		NextTextureCacheIdx();
		DrawStaticImage(&backpic, 0, 0, palette[0]);
		DrawStaticImage(&doompic, 75, 20, palette[17]);
		UpdateDrawOTag();
		DrawRender();

		if(starttype == gt_single) break;

		P_LoadingPlaque(&connectpic, 54, 103, palette[0]);
		Link_Conection();
		NextTextureCacheIdx();
		DrawStaticImage(&backpic, 0, 0, palette[0]);
		DrawStaticImage(&doompic, 75, 20, palette[17]);
		UpdateDrawOTag();
		DrawRender();
		S_StartSound(NULL, sfx_pistol);// SNDPSTOL.mid
	} while (cancel_link != 0);

	G_InitNew(startskill, startmap, starttype);
	G_RunGame();
	return 0;
}

/*
=================
=
= M_Start
=
=================
*/


int	cursorframe/*, cursorcount*/;//80078000 00000bf0
int	movecount; //80077FA4 00000b94
//int playermap;

int	cursorpos[MAXPLAYERS];//80077E2C uGp00000a1c,
int m_vframe1[MAXPLAYERS]; //80077D24 uGp00000914

void M_Start(void)//L80035A9C()
{
    netgame = 0;
    consoleplayer = 0;
    playeringame[0] = true;
    playeringame[1] = false;

	Valloc_Init();

	ImageToVram(&loadingpic, "LOADING", 0);
	P_LoadingPlaque(&loadingpic, 95, 109, palette[16]);
	S_Lcd_Load(0);

	ImageToVram(&backpic, "BACK", 0);
	ImageToVram(&doompic, "DOOM", 0);
	ImageToVram(&connectpic, "CONNECT", 0);

	cursorframe = 0;
	cursorpos[0] = 0;
	m_vframe1[0] = 0;

	if (starttype == gt_single)
		movecount = 2;
    else
        movecount = 54;//LIMIT LEVEL

	if (movecount < startmap)
		startmap = 1;

    psxcd_play_at_andloop(CD_TRACK[2],CDVolume,0,0,CD_TRACK[2],CDVolume,0,0);
	do {} while (psxcd_elapsed_sectors() == 0);

	//Desactiva la vision en la pantalla
	draw[0].isbg = 0;
	draw[1].isbg = 0;
	drawside ^= 1;

	//Draw fake menu
    M_Drawer();
    M_CrossFadeWipe();

	//Activa la vision en la pantalla
	draw[0].isbg = 1;
	draw[1].isbg = 1;

	last_ticon = ticon;
}

/*
=================
=
= M_Stop
=
=================
*/

void M_Stop(int exit)//L80035C40()
{
    S_StartSound(0, sfx_pistol);
    psxcd_stop();

    if (((exit == ga_exit) && (starttype == gt_single)) && (startmap != 1))
    {
        startmap = 31;
    }
}

/*
=================
=
= M_Ticker
=
=================
*/

int M_Ticker(void)//L80035CC4()
{
    int buttons;

    buttons = ticbuttons[0];

    /* exit menu if button press */
    if (ticbuttons[0] != 0)
        last_ticon = ticon;

    /* exit menu if time out */
    if ((ticon - last_ticon) > 1800)
        return ga_timeout;

    /* animate skull */
    if ((gamevbls < (int)gametic) && ((gametic & 3) == 0))
        cursorframe ^= 1;

    if (ticbuttons[0] != oldticbuttons[0])
    {
        if ((ticbuttons[0] & PAD_START) != 0)
        {
            return ga_exit;
        }
        if ((ticbuttons[0] & PAD_ACTION) && (cursorpos[0] >= gamemode))
        {
            if (cursorpos[0] < options)
            {
                return ga_exit;
            }
            if ((cursorpos[0] == options) && MiniLoop(O_Start,O_Stop,O_Ticker,O_Drawer) == ga_warped)
            {
                return ga_warped;
            }
        }
    }

    /* check for movement */
    if (!(buttons & PAD_ARROWS))
        m_vframe1[0] = 0;
    else
    {
        m_vframe1[0] -= vblsinframe[0];
        if (m_vframe1[0] <= 0)
        {
            m_vframe1[0] = TICRATE;

            if (buttons & PAD_DOWN)
            {
                cursorpos[0] += 1;
                if (cursorpos[0] == NUMMENUITEMS)
                    cursorpos[0] = 0;

                S_StartSound(NULL, sfx_pstop);
            }
            else if (buttons & PAD_UP)
            {
                cursorpos[0] += -1;
                if (cursorpos[0] == -1)
                    cursorpos[0] = NUMMENUITEMS-1;

                S_StartSound(NULL, sfx_pstop);
            }

            switch (cursorpos[0])
			{
			    case	level:
			        if (buttons & PAD_RIGHT)
					{
						startmap++;
                        if (movecount < startmap)
                            startmap = movecount;
                        else
                            S_StartSound(NULL, sfx_swtchx);
					}
					else if (buttons & PAD_LEFT)
					{
						startmap--;
						if (startmap < 1)
                            startmap = 1;
                        else
                            S_StartSound(NULL, sfx_swtchx);
					}
					break;

                case	difficulty:
                    if (buttons & PAD_RIGHT)
					{
						//if (startskill < sk_nightmare)
						if (startskill < sk_hard)
                        {
                            startskill++;
                            S_StartSound(NULL, sfx_swtchx);
                        }
					}
					else if (buttons & PAD_LEFT)
					{
                        if (startskill != 0)
                        {
                            startskill--;
                            S_StartSound(NULL, sfx_swtchx);
                        }
					}
					break;
                case	gamemode:
					if (buttons & PAD_RIGHT)
					{
                        if (starttype < gt_deathmatch)
                        {
							starttype++;
							if(starttype == gt_coop)
                                startmap = 1;
                            S_StartSound(NULL, sfx_swtchx);
                        }
					}
					else if (buttons & PAD_LEFT)
					{
						if (starttype != gt_single)
                        {
                            starttype--;
                            if (starttype == gt_single)
                                startmap = 1;
                            S_StartSound(NULL, sfx_swtchx);
                        }
                    }

                    if (starttype != gt_single)
                        movecount = 54;
                    else
                        movecount = 2;

                    if (startmap > movecount)
                        startmap = 1;
					break;
				default:
					break;
			}
        }
    }
    return ga_nothing;
}

/*
=================
=
= M_Drawer
=
=================
*/

void M_Drawer(void)//L80036058()
{
    NextTextureCacheIdx();
    DrawStaticImage(&backpic, 0, 0, palette[0]);
    DrawStaticImage(&doompic, 75, 20, palette[17]);

    DrawImage(statuspic.vtpage, palette[16], CURSORX, CURSORY(cursorpos[0]) - 2, M_SKULL_VX + (cursorframe * M_SKULL_W), M_SKULL_VY, M_SKULL_W, M_SKULL_H);

    ST_DrawText(74, STARTY[0], "Game Mode");
    ST_DrawText(90, STARTY[0]+20, gametypenames[starttype]);
    if (starttype == 0)
    {
        if (startmap == 1)
            ST_DrawText(74, STARTY[1], "Ultimate Doom");
        else
            ST_DrawText(74, STARTY[1], "Doom II");
    }
    else
    {
        ST_DrawText(74, STARTY[1], "Level");
        if (startmap >= 10)
			ST_DrawValue(148, STARTY[1], startmap);
		else
            ST_DrawValue(136, STARTY[1], startmap);
    }

    ST_DrawText(74, STARTY[2], "Difficulty");
    ST_DrawText(90, STARTY[2] + 20, skillnames[startskill]);
    ST_DrawText(74, STARTY[3], "Options");

    UpdateDrawOTag();
    DrawRender();
}

/*
=================
=
= M_CrossFadeWipe
=
=================
*/

void M_CrossFadeWipe(void)//L80036248()
{
	DRAWENV	    tmp_draw[2];					/* draw environment */
	DISPENV     tmp_disp[2];					/* display environment*/
	POLY_FT4	poly[2];
	int			intensity;
	long        d_side;

	Valloc_Init();

	SetDefDrawEnv(&tmp_draw[0], 512, 256, 256, 240);
	tmp_draw[0].isbg = 1;
	tmp_draw[0].dtd = 0;
	tmp_draw[0].dfe = 1;

	SetDefDrawEnv(&tmp_draw[1], 768, 256, 256, 256);
	tmp_draw[1].isbg = 1;
	tmp_draw[1].dtd = 0;
	tmp_draw[1].dfe = 1;

	SetDefDispEnv(&tmp_disp[0], 768, 256, 256, 240);
	SetDefDispEnv(&tmp_disp[1], 512, 256, 256, 240);

	MoveImage(&disp[drawside].disp, 768, 256);

	DrawSync(0);
	VSync(0);

	PutDrawEnv(&tmp_draw[0]);
	PutDispEnv(&tmp_disp[0]);

	//Set Layer1
	setPolyFT4(&poly[0]);
	setUV4(&poly[0], 0, 0, 255, 0, 0, 239, 255, 239);
	setXY4(&poly[0], 0, 0, 255, 0, 0, 239, 255, 239);

	if (drawside == 0)
		poly[0].tpage = getTPage(2, 0, 256, 0);
	else
		poly[0].tpage = getTPage(2, 0, 0, 0);

	poly[0].clut = 0;

	//Set Layer2
	setPolyFT4(&poly[1]);
	setSemiTrans(&poly[1], 1);
	setUV4(&poly[1], 0, 0, 255, 0, 0, 239, 255, 239);
	setXY4(&poly[1], 0, 0, 255, 0, 0, 239, 255, 239);

	if (drawside != 0)
		poly[1].tpage = getTPage(2, 0, 256, 0);
	else
		poly[1].tpage = getTPage(2, 0, 0, 0);

	poly[1].clut = 0;

	d_side = 0;
	intensity = 255;
	do
	{
		setRGB0(&poly[0], intensity, intensity, intensity);
		W_AddPrim(&poly[0]);// add to order table*/

		setRGB0(&poly[1], ~intensity, ~intensity, ~intensity);
		W_AddPrim(&poly[1]);// add to order table*/

		UpdateDrawOTag();

		//DrawRender
		DrawSync(0);
		VSync(0);

		d_side ^= 1;
		PutDrawEnv(&tmp_draw[d_side]);
		PutDispEnv(&tmp_disp[d_side]);

		intensity -= 5;
	} while (intensity >= 0);

	UpdateDrawOTag();
	DrawRender();
}
