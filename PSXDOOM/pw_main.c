/* pw_main.c -- password menu */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "r_local.h"


char *passwordChar = "bcdfghjklmnpqrstvwxyz0123456789!";//80073B7C
byte Passwordbuff[12];//0x80096388

int CurPasswordSlot = 0;//80077A70 iGp00000660
int PassInvalidTic;//80077E58 uGp00000a48
int PassCodePos;//80077F9C uGp00000b8c

boolean doPassword = false;//80077A6C

/*
=================
=
= PW_Start
=
=================
*/

void PW_Start(void)//L80036C1C()
{
	S_StartSound(NULL, sfx_pistol);

	oldticbuttons[0] = ticbuttons[0];
	oldticbuttons[1] = ticbuttons[1];
	m_vframe1[0] = 0;
	PassInvalidTic = 0;
    PassCodePos = 0;
}

/*
=================
=
= PW_Stop
=
=================
*/

void PW_Stop(int exit)//L80036C1C()
{
	S_StartSound(NULL, 7);// SNDPSTOL.mid
	PassCodePos = 32;
	PW_Drawer();
}

/*
=================
=
= PW_Ticker
=
=================
*/

int PW_Ticker(void)//L80036CA0()
{
    int buttons, levelnum, skill;

    if(PassInvalidTic && (gamevbls < (int)gametic))
    {
        PassInvalidTic--;
        if((PassInvalidTic & 7) == 4)
            S_StartSound(0,0x18);
    }

    buttons = ticbuttons[0];

    if (buttons & PAD_ARROWS)
	{
        m_vframe1[0] -= vblsinframe[0];
        if (m_vframe1[0] <= 0)
        {
            m_vframe1[0] = TICRATE;

            if (buttons & PAD_DOWN)
			{
				if (PassCodePos < 24)
                {
                    PassCodePos += 8;
                    S_StartSound(NULL, sfx_pstop);
                }
			}
			else if (buttons & PAD_UP)
			{
				if (PassCodePos >= 8)
                {
				    PassCodePos -= 8;
                    S_StartSound(NULL, sfx_pstop);
                }
			}

			if (buttons & PAD_RIGHT)
			{
                PassCodePos += 1;
                if (PassCodePos >= 32)
                    PassCodePos = 31;
                else
                    S_StartSound(NULL, sfx_pstop);
			}
			else if (buttons & PAD_LEFT)
			{
				PassCodePos -= 1;
                if (PassCodePos < 0)
                    PassCodePos = 0;
                else
                    S_StartSound(NULL, sfx_pstop);
			}
        }
	}
	else
	{
		m_vframe1[0] = 0;
	}

	if (buttons & (PAD_START | PAD_SELECT))
		return ga_exit;// exit

    if (buttons != oldticbuttons[0])
	{
        if (buttons & (PAD_SQUARE | PAD_CROSS| PAD_CIRCLE))
        {
            S_StartSound(NULL, sfx_swtchx);
            if (CurPasswordSlot < 10)
            {
                Passwordbuff[CurPasswordSlot] = PassCodePos;
                CurPasswordSlot++;
                if (CurPasswordSlot < 10)
                    return ga_nothing;
            }

            if(Decode_Password(Passwordbuff, &levelnum, &skill, NULL))
            {
                startskill = skill;
                startmap = levelnum;
                gamemap = levelnum;
                gameskill = skill;
                doPassword = true; //80077A6C

                //I_Error("startskill: %d\n startmap: %d", startskill, startmap);
                return ga_warped;
            }

            PassInvalidTic = 16;
        }
        else if(buttons & PAD_TRIANGLE)
        {
            S_StartSound(NULL, sfx_swtchx);
            CurPasswordSlot--;
            if (CurPasswordSlot < 0)
                CurPasswordSlot = 0;
        }

        Passwordbuff[CurPasswordSlot] = 0;
	}

    return ga_nothing;
}

/*
=================
=
= PW_Drawer
=
=================
*/

void PW_Drawer(void) //L80036F34()
{
    byte	pass[2] = {0,0};
	byte	c;
	int		texid, cnt;
	int		xpos, ypos, pos1, pos2;
	RECT	area;

	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT *textsprite = (SPRT*) getScratchAddr(128);//1F800200

	NextTextureCacheIdx();

	//Draw Backround MARB01 Pic
	for (ypos = 0; ypos < 4; ypos++)
	{
		for (xpos = 0; xpos < 4; xpos++)
		{
			DrawStaticImage(&marb01pic, xpos << 6, ypos << 6, palette[0]);
		}
	}

	setRECT(&area, 0, 0, 0, 0);
	SetDrawMode(drawmode, 0, 0, statuspic.vtpage, &area);
	W_AddPrim(drawmode);// add to order table

	setSprt(textsprite);
	textsprite->clut = palette[16];

	for (cnt = 0; cnt < 32; cnt++)
	{
	    setRGB0(textsprite, 128, 128, 128);

        pos1 = cnt;
        if (cnt < 0)
          pos1 = cnt + 7;

        pos1 >>= 3;
        pos2 = cnt - (pos1 << 3);

        xpos = (pos2 * 20) + 48;
        ypos = (pos1 * 20) + 60;

        if (PassCodePos == cnt)
		{
			if (!(ticon & 4)) continue;
			setRGB0(textsprite, 255, 0, 0);
		}

        c = passwordChar[cnt];
        if ((byte)(c - 'a') < 26)
        {
            texid = (byte)(c - 57);
            ypos = (pos1 * 20) + 63;
        }
        else
        {
            if ((byte)(c - '0') < 10)
			{
				texid = (byte)(c - '0');
			}
			else if (c == '!')
			{
				texid = 12;
			}
        }

        setXY0(textsprite, xpos, ypos);
        setUV0(textsprite, symboldata[texid].x, symboldata[texid].y);
        setWH(textsprite, symboldata[texid].w, symboldata[texid].h);

        W_AddPrim(textsprite);// add to order table
	}

	ST_DrawText(-1,20,"Password");

	xpos = 58;
	ypos = 160;
	for (cnt = 0; cnt < CurPasswordSlot; cnt++)
	{
	    pass[0] = passwordChar[Passwordbuff[cnt]];
		ST_DrawText(xpos, ypos, pass);//Draw Text
		xpos += 14;
	}

	if (cnt < 10)
	{
		xpos = (cnt * 14) + 58;
		do
		{
			ST_DrawText(xpos, ypos, ".");//Draw Text
			xpos += 14;
			cnt++;
		} while (cnt < 10);
	}

	if (PassInvalidTic & 4)//80077E58
	{
		ST_DrawText(-1, 200, "Invalid Password");//Draw Text
	}

	UpdateDrawOTag();
	DrawRender();
}
