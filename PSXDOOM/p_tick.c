#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

int	playertics, thinkertics, sighttics, basetics, latetics;
int	tictics;

boolean		gamepaused; //80077CEC, iGp000008dc
//jagobj_t	*pausepic;

/*
===============================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

//thinker_t	thinkercap;	/* both the head and tail of the thinker list */    //80096378
//mobj_t		mobjhead;	/* head and tail of mobj list */                //800A8C74,
int			activethinkers;	/* debug count */                               //80078118, iGp00000d08
int			activemobjs;	/* debug count */

/*
===============
=
= P_InitThinkers
=
===============
*/
#if 0
void P_InitThinkers (void)
{
	thinkercap.prev = thinkercap.next  = &thinkercap;
	mobjhead.next = mobjhead.prev = &mobjhead;
}
#endif // 0

/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker (thinker_t *thinker)//L80028B18()
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker (thinker_t *thinker)//L80028B48()
{
	thinker->function = (think_t)-1;
}

/*
===============
=
= P_RunThinkers
=
===============
*/
//inline
void P_RunThinkers (void)//L80028B54()
{
	thinker_t	*currentthinker;

	activethinkers = 0;

	currentthinker = thinkercap.next;
	if (thinkercap.next != &thinkercap)
    {
        while (currentthinker != &thinkercap)
        {
            if (currentthinker->function == (think_t)-1)
            {	// time to remove it
                currentthinker->next->prev = currentthinker->prev;
                currentthinker->prev->next = currentthinker->next;
                Z_Free (currentthinker);
            }
            else
            {
                if (currentthinker->function)
                {
                    currentthinker->function (currentthinker);
                }
                activethinkers++;
            }
            currentthinker = currentthinker->next;
        }
	}
}

/*
===================
=
= P_RunMobjLate
=
= Run stuff that doesn't happen every tick
===================
*/

//inline
void P_RunMobjLate (void)//L80028C10()
{
	mobj_t	*mo;
	mobj_t	*next;

	for (mo=mobjhead.next ; mo != &mobjhead ; mo=next)
	{
		next = mo->next;	/* in case mo is removed this time */
		if (mo->latecall)
		{
			mo->latecall(mo);
		}
	}
}

/*
==============
=
= P_CheckCheats
=
==============
*/

int		codepos;                //80077E10, iGp00000a00
short	codetmp[8];             //800a8f88
int		m_vframe1[MAXPLAYERS];	//80077D24
int		cht_ticon;              //800780FC
int 	warpmap;                //80078094, iGp00000c84
int     Vram_page;              //80077D00, iGp000008f0

typedef enum
{
	CH_ALLLINES,	//0
	CH_ALLTHINGS,	//1
	CH_GODMODE,		//2
	CH_AMMOWEAPONS,	//3
	CH_NOCLIP,		//4 Enabled in Station Doom
	CH_WARPLEVEL,	//5
	CH_NULL2,		//6
	CH_VRAMVIEWER,	//7 Enabled in GEC Master Edition
	CH_NULL3,		//8
	CH_X_RAY,		//9
	CH_NULL4,		//10
	CH_NULL5,		//11
	NUMCHEATS
} cheatnum_t;

short	cheatcodes[NUMCHEATS][8] = //800675A8
{
	{ PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2, PAD_R2, PAD_L2, PAD_R2, PAD_R1, PAD_SQUARE },
	{ PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2, PAD_R2, PAD_L2, PAD_R2, PAD_R1, PAD_CIRCLE },
	{ PAD_DOWN, PAD_L2, PAD_SQUARE, PAD_R1, PAD_RIGHT, PAD_L1, PAD_LEFT, PAD_CIRCLE },
	{ PAD_CROSS, PAD_TRIANGLE, PAD_L1, PAD_UP, PAD_DOWN, PAD_R2, PAD_LEFT, PAD_LEFT },
	{ PAD_UP, PAD_UP, PAD_UP, PAD_UP, PAD_UP, PAD_UP, PAD_UP, PAD_R1 },
	{ PAD_RIGHT, PAD_LEFT, PAD_R2, PAD_R1, PAD_TRIANGLE, PAD_L1, PAD_CIRCLE, PAD_CROSS },
	{ PAD_LEFT, PAD_LEFT, PAD_LEFT, PAD_LEFT, PAD_LEFT, PAD_LEFT, PAD_LEFT, PAD_LEFT },
	{ PAD_TRIANGLE, PAD_SQUARE, PAD_UP, PAD_LEFT, PAD_DOWN, PAD_RIGHT, PAD_CROSS, PAD_CIRCLE },
	{ PAD_CROSS, PAD_CROSS, PAD_CROSS, PAD_CROSS, PAD_CROSS, PAD_CROSS, PAD_CROSS, PAD_CROSS },
	{ PAD_L1, PAD_R2, PAD_L2, PAD_R1, PAD_RIGHT, PAD_TRIANGLE, PAD_CROSS, PAD_RIGHT },
	{ PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE },
	{ PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE },
};

void P_CheckCheats (void)//L80028C74()
{
	int		 buttons, oldbuttons;
	int		 i, j;
	int		 action;
	player_t *p;
	mobj_t	 *m;

	for (i=(MAXPLAYERS-1) ; i >= 0 ; i--)
	{
		if (!playeringame[i])
			continue;

		buttons = ticbuttons[i];
		oldbuttons = oldticbuttons[i];

		if ((buttons & PAD_START) && !(oldbuttons & PAD_START))
		{
			gamepaused ^= 1;

			if (gamepaused)
			{
				psxcd_pause();
				#if GH_UPDATES == 1
				//Only PSX (FINAL DOOM / DOOM Greatest Hits)
                wess_seq_stop(sfx_sawful);
                wess_seq_stop(sfx_sawhit);
				#endif // GH_UPDATES
				S_Pause();
				codepos = 0;
				cht_ticon = ticon;
				return;
			}

            psxcd_restart(0);
            do {} while (psxcd_seeking_for_play() != 0);
            psxspu_start_cd_fade((FADE_TIME*2), CDVolume);
            S_Resume();
            players[0].cheats &= ~(CF_WARPLEVEL | CF_VRAMVIEWER);
            ticon = cht_ticon;
            ticsinframe = cht_ticon >> 2;
		}

		p = &players[i];
		if (((buttons & PAD_SELECT) && !(oldbuttons & PAD_SELECT)) && gamepaused)
		{
			p->cheats &= ~(CF_WARPLEVEL | CF_VRAMVIEWER);

			DrawRender();

			action = MiniLoop(O_Start,O_Stop,O_Ticker,O_Drawer);
			if(action == ga_exit)
                return;

            gameaction = action;
            if (gameaction == ga_restart || gameaction == ga_exitdemo)
            {
                O_Drawer();
                return;
            }
		}
	}

	if (netgame != gt_single)
		return;

	if (buttons == 0)
		m_vframe1[0] = 0;

	if (p->cheats & CF_WARPLEVEL)
	{
		m_vframe1[0] -= vblsinframe[0];
		if (m_vframe1[0] <= 0)
		{
			if (buttons & PAD_LEFT)
			{
				warpmap -= 1;

				if (warpmap <= 0)
					warpmap = 1;

				m_vframe1[0] = TICRATE;
			}
			else if (buttons & PAD_RIGHT)
			{
				warpmap += 1;

				if (warpmap >= 55)
					warpmap = 54;

				m_vframe1[0] = TICRATE;
			}
		}

		if ((buttons != oldbuttons) && (buttons & PAD_ACTION))
		{
            gameaction = ga_warped;
            p->cheats &= ~CF_WARPLEVEL;
            gamemap = startmap = warpmap;
		}

		return;
	}

	if (p->cheats & CF_VRAMVIEWER)
	{
		if (buttons != oldbuttons)
		{
			if (buttons & PAD_LEFT)
			{
				Vram_page -= 1;
				if (Vram_page < 0)
					Vram_page = 0;
			}
			else if (buttons & PAD_RIGHT)
			{
				Vram_page += 1;
				if (Vram_page >= MAX_DYNAMIC_TPAGE)
					Vram_page = (MAX_DYNAMIC_TPAGE-1);
			}
		}

		return;
	}

    if (!gamepaused)
        return;

    if (buttons == 0)
        return;

    if (buttons == oldbuttons)
        return;

    codetmp[codepos] = buttons;
    codepos += 1;

    //Check code
    for (i = 0; i < NUMCHEATS; i++)
    {
        for (j = 0; j < codepos; j++)
        {
            if (cheatcodes[i][j] != codetmp[j])
                break;
        }

        //Cycle Code Complete
        if (j >= 8)
        {
            switch (i)// Get case number
            {
            case CH_ALLLINES:
                p->cheats ^= CF_ALLLINES;
                stbar.messagedelay = 1;

                if (p->cheats & CF_ALLLINES)
                {
                    stbar.message = "Map All Lines ON.";
                }
                else
                {
                    stbar.message = "Map All Lines OFF.";
                }
                break;

            case CH_ALLTHINGS:
                p->cheats ^= CF_ALLTHINGS;
                stbar.messagedelay = 1;

                if (p->cheats & CF_ALLTHINGS)
                {
                    stbar.message = "Map All Things ON.";
                }
                else
                {
                    stbar.message = "Map All Things OFF.";
                }
                break;

            case CH_GODMODE:
                p->cheats ^= CF_GODMODE;
                stbar.messagedelay = 1;

                if (p->cheats & CF_GODMODE)
                {
                    stbar.message = "All Powerful Mode ON.";
                    p->health = 100;
                    p->mo->health = 100;
                }
                else
                {
                    stbar.message = "All Powerful Mode OFF.";
                }
                break;

            case CH_AMMOWEAPONS:
                for (m = mobjhead.next; m != &mobjhead; m = m->next)
                {
                    switch (m->type)
                    {
                    case MT_MISC4:
                        p->cards[it_bluecard] = true;
                        break;
                    case MT_MISC5:
                        p->cards[it_redcard] = true;
                        break;
                    case MT_MISC6:
                        p->cards[it_yellowcard] = true;
                        break;
                    case MT_MISC7:
                        p->cards[it_yellowskull] = true;
                        break;
                    case MT_MISC8:
                        p->cards[it_redskull] = true;
                        break;
                    case MT_MISC9:
                        p->cards[it_blueskull] = true;
                        break;
                    default:
                        break;
                    }
                }
                p->armorpoints = 200;
                p->armortype = 2;
                for (i = 0; i<NUMWEAPONS; i++) {p->weaponowned[i] = true;}
                for (i = 0; i<NUMAMMO; i++) {p->ammo[i] = p->maxammo[i];}

                stbar.messagedelay = 1;
                stbar.message = "Lots Of Goodies!";
                break;
            case CH_WARPLEVEL:
                p->cheats |= CF_WARPLEVEL;

                if (warpmap >= 55)
                    warpmap = 54;
                else
                    warpmap = gamemap;
                break;
            case CH_X_RAY:
                p->cheats ^= CF_X_RAY;
                break;

            case CH_VRAMVIEWER: //Enabled in [GEC] Master Edition
                p->cheats ^= CF_VRAMVIEWER;
                break;

            case CH_NOCLIP: //Enabled in Station Doom
                p->mo->flags ^= MF_NOCLIP;
                stbar.messagedelay = 1;

                if (p->mo->flags & MF_NOCLIP)
                {
                    stbar.message = "No Clip ON.";
                }
                else
                {
                    stbar.message = "No Clip OFF.";
                }
                break;
            }

            break;//break for i
        }
    }

    i = codepos;
    if (codepos < 0) i = codepos + 7;
    codepos = codepos - ((i/8) * 8);
}

int playernum;//80078110, iGp00000d00

void G_DoReborn (int playernum);//extern

/*
=================
=
= P_Ticker
=
=================
*/

//int		ticphase;

//psx doom new
int checkcount; //*L80077DE4

//extern functions
void P_CheckSights (void);
void P_RunMobjBase (void);

int P_Ticker (void)//800292E4
{
	player_t *pl;

	gameaction = ga_nothing;

	//
	// check for pause and cheats
	//
	P_CheckCheats();

	if ((!gamepaused) && (gamevbls < gametic))
	{
	    P_RunThinkers();
		P_CheckSights();
		P_RunMobjBase();
		P_RunMobjLate();

		P_UpdateSpecials();
		P_RespawnSpecials();
		ST_Ticker(); // update status bar
	}

	//
	// run player actions
	//
	for (playernum = 0, pl = players; playernum < MAXPLAYERS; playernum++, pl++)
	{
		if (playeringame[playernum])
		{
			if (pl->playerstate == PST_REBORN)
            {
				G_DoReborn(playernum);
            }
			AM_Control(pl);
			P_PlayerThink(pl);
		}
	}

	return gameaction; // may have been set to ga_died, ga_completed, or ga_secretexit
}

/*
=============
=
= P_Drawer
=
= draw current display
=============
*/



void P_Drawer (void) //L800294CC()
{
	NextTextureCacheIdx();

	if (players[consoleplayer].automapflags & AF_ACTIVE)
        AM_Drawer();
	else
		R_RenderPlayerView();

	ST_Drawer();
	UpdateDrawOTag();
}

//extern	 int		ticremainder[2];

void P_Start (void)//L80029554()
{
	gamepaused = false;
	validcount = 1;

	AM_Start();
	M_ClearRandom();

	disableload = true;

	if (demoplayback)
		psxcd_play_at_andloop(CD_TRACK[3],CDVolume,0,0,CD_TRACK[3],CDVolume,0,0);
	else
		S_StartMusic();
}

void P_Stop (int exit)//L800295EC()
{
	int i;

	DrawSync(0);
	S_StopAll();
	psxcd_stop();
	S_StopMusic();

	gamepaused = false;
	disableload = false;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			G_PlayerFinishLevel(i);
		}
	}
}

