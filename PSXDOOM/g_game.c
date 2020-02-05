/* G_game.c  */

#include "doomdef.h"
#include "p_local.h"

void G_PlayerReborn (int player);

void G_DoReborn (int playernum);

void G_DoLoadLevel (void);


gameaction_t    gameaction;                 //80077CE0 iGp000008d0
skill_t         gameskill;                  //8007807C uGp00000c6c
int             gamemap;                    //80077E74 uGp00000a64
int				nextmap;				    //80077EC4 iGp00000ab4/* the map to go to after the stats */

gametype_t		netgame;                    //80077E88 iGp00000a78

boolean         playeringame[MAXPLAYERS];   //80077ED4 -> uGp00000ac4, 80077ED8 -> uGp00000ac8
player_t        players[MAXPLAYERS];        //800A85CC,800A86F8

int             consoleplayer;          /* player taking events and displaying  */
int             displayplayer;          /* view being displayed  */
int             gametic;
int             totalkills, totalitems, totalsecret;    /* for intermission  */

//char            demoname[32];
boolean         demorecording;          //80077FD4, uGp00000bc4
boolean         demoplayback;           //80077EAC, uGp00000a9c

//Hmm por que funciona aqui??
mapthing_t	deathmatchstarts[10], *deathmatch_p;//80097e4c, 80077E8C
mapthing_t	playerstarts[MAXPLAYERS];//800a8c60

/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel (void) //L80012E04()
{
	int i;

	P_LoadingPlaque(&loadingpic, 95, 109, palette[16]);
    S_Lcd_Load(gamemap);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			if ((gameaction == ga_restart) || (gameaction == ga_warped))
				players[i].playerstate = PST_REBORN;
			else if (players[i].playerstate == PST_DEAD)
				players[i].playerstate = PST_REBORN;
		}
	}

	P_SetupLevel(gamemap, gameskill);
	Z_CheckHeap(mainzone);
	gameaction = ga_nothing;
	//printf("G_DoLoadLevel");
}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Mobj
==============================================================================
*/

/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/

void G_PlayerFinishLevel (int player)//L80012ED4()
{
	player_t *p;

	p = &players[player];

	D_memset (p->powers, 0, sizeof (p->powers));
	D_memset (p->cards, 0, sizeof (p->cards));
	p->mo->flags &= ~(MF_BLENDMASK1|MF_BLENDMASK2|MF_BLENDMASK3); /* cancel invisibility  */ //PC DOOM ~MF_SHADOW
	p->extralight = 0;                      /* cancel gun flashes  */
	p->fixedcolormap = 0;                   /* cancel ir gogles  */
	p->damagecount = 0;                     /* no palette changes  */
	p->bonuscount = 0;
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

void G_PlayerReborn (int player) //L80012F5C()
{
	player_t *p;
	int i;
	int frags, kills, items, secrets;

	p = &players[player];
	frags = p->frags;
	kills = p->killcount;
	items = p->itemcount;
	secrets = p->secretcount;
	D_memset(p, 0, sizeof(*p));
	p->health = MAXHEALTH;
	p->usedown = p->attackdown = true; // don't do anything immediately
	p->playerstate = PST_LIVE;
	p->readyweapon = p->pendingweapon = wp_pistol;
	p->weaponowned[wp_fist] = true;
	p->weaponowned[wp_pistol] = true;
	p->ammo[am_clip] = 50;
	p->frags = frags;
	p->killcount = kills;
	p->itemcount = items;
	p->secretcount = secrets;

	for (i = 0; i < NUMAMMO; i++)
		p->maxammo[i] = maxammo[i];
}

/*
====================
=
= G_CheckSpot
=
= Returns false if the player cannot be respawned at the given mapthing_t spot
= because something is occupying it
====================
*/

void P_SpawnPlayer (mapthing_t *mthing);

#if 0
boolean G_CheckSpot (int playernum, mapthing_t *mthing)
{
	fixed_t         x,y;
	subsector_t *ss;
	int                     an;
	mobj_t		*mo;

	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	if (!P_CheckPosition (players[playernum].mo, x, y) )
		return false;

	ss = R_PointInSubsector (x,y);
	an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT;

/* spawn a teleport fog  */
	mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an], ss->sector->floorheight
	, MT_TFOG);
	S_StartSound (mo, sfx_telept);

	return true;
}
#endif

/*
====================
=
= G_DeathMatchSpawnPlayer
=
= Spawns a player at one of the random death match spots
= called at level load and each death
====================
*/

#if 0
void G_DeathMatchSpawnPlayer (int playernum)
{
	int             i,j;
	int				selections;

	selections = deathmatch_p - deathmatchstarts;
	if (selections < 4)
		I_Error ("Only %i deathmatch spots, 4 required", selections);

	for (j=0 ; j<20 ; j++)
	{
		i = P_Random()%selections;
		if (G_CheckSpot (playernum, &deathmatchstarts[i]) )
		{
			deathmatchstarts[i].type = playernum+1;
			P_SpawnPlayer (&deathmatchstarts[i]);
			return;
		}
	}

/* no good spot, so the player will probably get stuck  */
	P_SpawnPlayer (&playerstarts[playernum]);
}
#endif

/*
====================
=
= G_DoReborn
=
====================
*/

void G_DoReborn (int playernum)//80013044
{
	int i, j;
	int selections;
	fixed_t      x, y;
	subsector_t *ss;
	int          an;
	mobj_t      *mo;
	mapthing_t *mthing;

	if (!netgame)
	{
		gameaction = ga_died;			/* reload the level from scratch  */
		return;
	}

	//printf("G_DoReborn %d\n",playernum);

	/* */
	/* respawn this player while the other players keep going */
	/* */
	if (players[playernum].mo->player != 0)
		players[playernum].mo->player = NULL;   /* dissasociate the corpse  */

	/* spawn at random spot if in death match  */
	if (netgame == gt_deathmatch)
	{
		/*G_DeathMatchSpawnPlayer (playernum);
		return; */
		selections = (int)(deathmatch_p - deathmatchstarts);
		for (j = 0; j < 16; j++)
		{
			i = P_Random() % selections;

			x = deathmatchstarts[i].x << FRACBITS;
			y = deathmatchstarts[i].y << FRACBITS;

			if (P_CheckPosition(players[playernum].mo, x, y))
			{
				deathmatchstarts[i].type = playernum + 1;
				P_SpawnPlayer(&deathmatchstarts[i]);
				playerstarts[i].type = i + 1;//psx
				mthing = &deathmatchstarts[i];
				goto spawnfog;
			}
		}
	}
	else
	{
		x = playerstarts[playernum].x << FRACBITS;
		y = playerstarts[playernum].y << FRACBITS;

		if (!P_CheckPosition(players[playernum].mo, x, y))
		{
			// try to spawn at one of the other players spots
			for (i = 0; i < MAXPLAYERS; i++)
			{
				x = playerstarts[i].x << FRACBITS;
				y = playerstarts[i].y << FRACBITS;

				if (P_CheckPosition(players[playernum].mo, x, y))
				{
					playerstarts[i].type = playernum + 1;
					P_SpawnPlayer(&playerstarts[i]);
					playerstarts[i].type = i + 1;
					mthing = &playerstarts[i];
					goto spawnfog;
				}
			}
		}
	}

	// he's going to be inside something.  Too bad.
	P_SpawnPlayer(&playerstarts[playernum]);
	mthing = &playerstarts[playernum];

spawnfog:
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	ss = R_PointInSubsector(x, y);
	//an = (((ANG45 * (mthing->angle/45)) >> ANGLETOFINESHIFT) * 4) & 0x7000;
	an = ((ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT) * 128 & 0x7000;

	// spawn a teleport fog
	mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an], ss->sector->floorheight, MT_TFOG);
	S_StartSound(mo, sfx_telept); //sfx_telept = 27

#if 0
	if (G_CheckSpot (playernum, &playerstarts[playernum]) )
	{
		P_SpawnPlayer (&playerstarts[playernum]);
		return;
	}
	/* try to spawn at one of the other players spots  */
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (G_CheckSpot (playernum, &playerstarts[i]) )
		{
			playerstarts[i].type = playernum+1;		/* fake as other player  */
			P_SpawnPlayer (&playerstarts[i]);
			playerstarts[i].type = i+1;             /* restore  */
			return;
		}
	/* he's going to be inside something.  Too bad.  */
	P_SpawnPlayer (&playerstarts[playernum]);
#endif
}


/*
====================
=
= G_CompleteLevel
=
====================
*/

void G_CompleteLevel (void)//80013358
{
	gameaction = ga_completed;
}

/*============================================================================  */

/*
====================
=
= G_InitNew
=
====================
*/

extern mobj_t emptymobj;//0x800a9c14
boolean restarlevel; //80077E20, uGp00000a10

void G_InitNew (skill_t skill, int map, gametype_t gametype) //L80013368()
{
	int             i;

	//printf ("G_InitNew, skill %d, map %d\n", skill, map);

	restarlevel = false;
	V_PagFlags &= 1;
	Valloc_Init();
	Z_FreeTags(mainzone, (PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE));
	M_ClearRandom ();

/* force players to be initialized upon first level load          */
	//for (i=0 ; i<MAXPLAYERS ; i++)
	for (i=(MAXPLAYERS-1) ; i >= 0 ; i--)
    {
		players[i].playerstate = PST_REBORN;
    }

/* these may be reset by I_NetSetup */
	gamemap = map;
	netgame = gametype;
	gameskill = skill;

	D_memset(&emptymobj, 0, sizeof(emptymobj));
	players[0].mo = players[1].mo = &emptymobj;	/* for net consistancy checks */

	playeringame[0] = true;
	if (netgame == gt_single)
	{
	    playeringame[1] = false;
		BT_DATA[0] = (buttons_t *)ActualConfiguration;
	}
	else if (netgame <= gt_deathmatch)
	{
		playeringame[1] = true;
	}

	demorecording = false;
	demoplayback = false;

	if (skill == sk_nightmare)
	{
		states[S_SARG_ATK1].tics = 2;
		states[S_SARG_ATK2].tics = 2;
		states[S_SARG_ATK3].tics = 2;
		mobjinfo[MT_SERGEANT].speed = 15;

		mobjinfo[MT_BRUISERSHOT].speed = 40*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 40*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 40*FRACUNIT;
	}
	else
	{
		states[S_SARG_ATK1].tics = 4;
		states[S_SARG_ATK2].tics = 4;
		states[S_SARG_ATK3].tics = 4;
		mobjinfo[MT_SERGEANT].speed = 10;

		mobjinfo[MT_BRUISERSHOT].speed = 30*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	}
}

/*============================================================================  */

/*
=================
=
= G_RunGame
=
= The game should allready have been initialized or laoded
=================
*/

void G_RunGame (void)//L800134FC()
{
	int		i;

	while (1)
	{
        /* load a level */
        G_DoLoadLevel ();

        //printf("RUN P_Start\n");
		/* run a level until death or completion */
		MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);

		restarlevel = false;

        if (gameaction == ga_recorddemo)
        {
            G_RecordDemo();
        }

        if(gameaction == ga_warped)
			continue; // skip intermission

        if ((gameaction == ga_died) || (gameaction == ga_restart))
        {
            restarlevel = true;
			continue;			/* died, so restart the level */
        }

        /* free all animation states graphics*/
        V_PagFlags &= 1;
        Z_FreeTags(mainzone, PU_ANIMATION);

        if (gameaction == ga_exitdemo)
            return;

        /* run a stats intermission */
        MiniLoop(IN_Start,IN_Stop,IN_Ticker,IN_Drawer);

        if ((!netgame && (gamemap == 30)) && (nextmap == 31))
        {
            /* run the finale if needed */
            MiniLoop(END1_Start,END1_Stop,END1_Ticker,END1_Drawer);

            if(gameaction == ga_warped)
                continue; // skip intermission

            if(gameaction == ga_restart)
                continue;
            else
            {
                if (gameaction != ga_exitdemo)
                    startmap = 2;
                return;
            }
        }
        else
        {
            if (nextmap > 60)
            {
                /* run the finale if needed */
                MiniLoop(Cast_Start, Cast_Stop, Cast_Ticker, Cast_Drawer);

                if(gameaction == ga_warped)
                    continue; // skip intermission

                if(gameaction == ga_restart)
                    continue;
                else
                    return;
            }

            gamemap = nextmap;
        }
	}
}


int G_PlayDemoPtr (void)//(int *demo) L800136E8()
{
	int		exit;
	int		skill, map;
	int		config[8];

	skill = BIGLONG(demo_p[0]);
	map = BIGLONG(demo_p[1]);
	demobuffer = demo_p + 2;

	//copy key configuration
	D_memcpy(config, ActualConfiguration, sizeof(config));

	//set new key configuration
	D_memcpy(ActualConfiguration, demobuffer, sizeof(config));
	demobuffer += 8;

	G_InitNew (skill, map, gt_single);
	G_DoLoadLevel ();
	demoplayback = true;
	exit = MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
	demoplayback = false;

	//restore key configuration
	D_memcpy(ActualConfiguration, config, sizeof(config));

	V_PagFlags &= 1;
	Z_FreeTags(mainzone, (PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE));
	return exit;
}

/*
=================
=
= G_RecordDemo
=
=================
*/

void G_RecordDemo (void)//8001380C
{
    #if 0
	demo_p = demobuffer = Z_Malloc (0x8000, PU_STATIC, NULL);

	*demo_p++ = startskill;
	*demo_p++ = startmap;

	G_InitNew (startskill, startmap, gt_single);
	G_DoLoadLevel ();
	demorecording = true;
	MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
	demorecording = false;

	D_printf ("w %x,%x",demobuffer,demo_p);

	while (1)
	{
		G_PlayDemoPtr (demobuffer);
	D_printf ("w %x,%x",demobuffer,demo_p);
	}
    #endif
}
