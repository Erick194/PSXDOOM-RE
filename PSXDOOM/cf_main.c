/* cf_main.c -- configuration menu */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "r_local.h"

char fuctionsnames[][16] =//80073B9C
{
	"Attack",
	"Use",
	"Strafe On",
	"Speed",
	"Strafe Left",
	"Strafe Right",
	"Weapon Backward",
	"Weapon Forward"
};

/*
=================
=
= CF_Start
=
=================
*/

void CF_Start(void)//L8003773C()
{
	S_StartSound(NULL, sfx_pistol);
	cursorframe = 0;
	cursorpos[0] = 0;
	ImageToVram(&buttonspic, "BUTTONS", 0);
}

/*
=================
=
= CF_Stop
=
=================
*/

void CF_Stop(int exit)//L80037780()
{
	S_StartSound(NULL, sfx_pistol);
	cursorpos[0] = options;
}

/*
=================
=
= CF_Ticker
=
=================
*/

int CF_Ticker(void)//L800377AC()
{
	unsigned int *tmpcfg;
	int cnt, buttons;

    if ((gamevbls < (int)gametic) && ((gametic & 3) == 0))
        cursorframe ^= 1;

	buttons = ticbuttons[0];

	if (buttons & PAD_ARROWS)
	{
        m_vframe1[0] -= vblsinframe[0];
        if (m_vframe1[0] <= 0)
        {
            m_vframe1[0] = TICRATE;

            if (buttons & PAD_DOWN)
			{
				cursorpos[0]++;
				if (cursorpos[0] > 8)
					cursorpos[0] = 0;

				S_StartSound(NULL, sfx_pstop);
			}
			else if (buttons & PAD_UP)
			{
				cursorpos[0]--;
				if (cursorpos[0] < 0)
					cursorpos[0] = 8;

				S_StartSound(NULL, sfx_pstop);
			}
        }
	}
	else
	{
		m_vframe1[0] = 0;
	}

	if (buttons & (PAD_START | PAD_SELECT))
		return ga_exit;

	if (buttons != oldticbuttons[0])
	{
		if (cursorpos[0] < 8)
		{
			tmpcfg = TempConfiguration;
			for (cnt = 0; cnt < 8; cnt++, tmpcfg++)
			{
				if (buttons & *tmpcfg)
				{
					ActualConfiguration[cursorpos[0]] = *tmpcfg;
					S_StartSound(NULL, sfx_swtchx);
					break;
				}
			}
		}
		else if (buttons & PAD_ACTION) // Set Default Configuration
		{
			D_memcpy(&ActualConfiguration, &DefaultConfiguration, sizeof(int) * 8);
			S_StartSound(NULL, sfx_swtchx);
		}
	}

	return ga_nothing;
}

/*
=================
=
= CF_Drawer
=
=================
*/

void CF_Drawer(void)//L80037984()
{
	int		picid, cnt, cnt2, xpos, ypos;
	unsigned int *tmpcfg;
	unsigned int *actcfg;

	NextTextureCacheIdx();

	//Draw Backround MARB01 Pic
	for (ypos = 0; ypos < 4; ypos++)
	{
		for (xpos = 0; xpos < 4; xpos++)
		{
			DrawStaticImage(&marb01pic, xpos << 6, ypos << 6, palette[0]);
		}
	}

	ST_DrawText(-1, 20, "Configuration");//Draw Text

	//Draw Skull Selector
	DrawImage(statuspic.vtpage, palette[16], 10, (cursorpos[0] * 20) + 43, (cursorframe * M_SKULL_W) + M_SKULL_VX, M_SKULL_VY, M_SKULL_W, M_SKULL_H);

	//Draw Psx Buttons Graphics
	ypos = 45;
	actcfg = ActualConfiguration;
	for (cnt = 0; cnt < 8; cnt++, actcfg++)
	{
		picid = 0;

		tmpcfg = TempConfiguration;
		for (cnt2 = 0; cnt2 < 8; cnt2++, tmpcfg++)
		{
			if(*actcfg == *tmpcfg)
				break;

			picid++;
		}
		//Draw Buttons PSX
        if (cursorpos[0] != cnt || !(ticon & 8))
            DrawImage(buttonspic.vtpage, palette[0], 32, ypos, buttonspic.vramx + (picid * 16), buttonspic.vramy, 16, 16);

		ypos += 20;
	}

	//Draw Options names
	ypos = 45;
	for (cnt = 0; cnt < 8; cnt++)
	{
		ST_DrawText(70, ypos, fuctionsnames[cnt]);
		ypos += 20;
	}

	//Draw Default Text
    if (cursorpos[0] != cnt || !(ticon & 8))
        ST_DrawText(70, (cnt * 20) + 45, "Default");

	UpdateDrawOTag();
	DrawRender();
}
