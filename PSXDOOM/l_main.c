/* l_main.c -- legal intro */

#include "doomdef.h"
#include "r_local.h"

int y_scroll;//80077FB8 uGp00000ba8
int last_ticon;//80077D38 iGp00000928

psxobj_t legalspic;//0x800979b0

/*
=======================
=
= L_Start
=
=======================
*/

void L_Start(void)//L80034D5C()
{
	Valloc_Init();
	ImageToVram(&legalspic, "LEGALS", 0);
	S_StartSound(NULL, sfx_sgcock);
	y_scroll = 240;
}

/*
=======================
=
= L_Stop
=
=======================
*/

void L_Stop(int exit)//L80034DA8()
{
	S_StartSound(NULL, sfx_barexp);
	M_CrossFadeWipe();
}

/*
=======================
=
= L_Ticker
=
=======================
*/

int L_Ticker(void)//L80034DD4()
{
    int exit;

    exit = ga_nothing;

    if (y_scroll > 0)
    {
        y_scroll--;

        if (y_scroll == 0)
            last_ticon = ticon;
    }
    else if ((ticon - last_ticon) > 120)
    {
        exit = ga_timeout;

        if ((ticon - last_ticon) < 180)
            exit = -(unsigned int)(ticbuttons[0] != 0) & ga_exit;
    }

    return exit;
}

/*
=======================
=
= L_Draw
=
=======================
*/

void L_Draw(void)//L80034E54()
{
	NextTextureCacheIdx();
	DrawStaticImage(&legalspic, 0, y_scroll, palette[16]);
	UpdateDrawOTag();
	DrawRender();
}
