
/* in_main.c -- intermission */
#include "doomdef.h"
#include "st_main.h"
#include "r_local.h"

#define	KVALX			172
#define	KVALY			80
#define	IVALX			172
#define	IVALY			110
#define	SVALX			172
#define	SVALY			140
#define	FVALX			230
#define	FVALY			74

#define	PLAYERONEFACEX		137
#define	PLAYERONEFACEY		30
#define	PLAYERTWOFACEX		217
#define	PLAYERTWOFACEY		30

extern int nextmap;

typedef struct pstats_s
{
	int		killpercent;
	int		itempercent;
	int		secretpercent;
	int		fragcount;
} pstats_t;

//psx
char mapnames[][32] =
{
	//Ultimate Doom
	"Hangar",
	"Plant",
	"Toxin Refinery",
	"Command Control",
	"Phobos Lab",
	"Central Processing",
	"Computer Station",
	"Phobos Anomaly",
	"Deimos Anomaly",
	"Containment Area",
	"Refinery",
	"Deimos Lab",
	"Command Center",
	"Halls of the Damned",
	"Spawning Vats",
	"Hell Gate",
	"Hell Keep",
	"Pandemonium",
	"House of Pain",
	"Unholy Cathedral",
	"Mt. Erebus",
	"Limbo",
	"Tower Of Babel",
	"Hell Beneath",
	"Perfect Hatred",
	"Sever The Wicked",
	"Unruly Evil",
	"Unto The Cruel",
	"Twilight Descends",
	"Threshold of Pain",
	//Doom II
	"Entryway",
	"Underhalls",
	"The Gantlet",
	"The Focus",
	"The Waste Tunnels",
	"The Crusher",
	"Dead Simple",
	"Tricks And Traps",
	"The Pit",
	"Refueling Base",
	"O of Destruction!",
	"The Factory",
	"The Inmost Dens",
	"Suburbs",
	"Tenements",
	"The Courtyard",
	"The Citadel",
	"Nirvana",
	"The Catacombs",
	"Barrels of Fun",
	"Bloodfalls",
	"The Abandoned Mines",
	"Monster Condo",
	"Redemption Denied",
	//Secret Levels
	"Fortress of Mystery",
	"The Military Base",
	"The Marshes",
	"The Mansion",
	"Club Doom"
};

pstats_t	pstats[MAXPLAYERS];//0x80097a04
int			killvalue[2], itemvalue[2], secretvalue[2], fragvalue[2];//800780C4,800780D0,80077E08,8007808C

// used to accelerate or skip a stage
int acceleratestage;//80078100|uGp00000cf0

extern char *passwordChar;//80073B7C
extern byte Passwordbuff[12];//0x80096388
extern boolean doPassword;//80077A6C

#if GH_UPDATES == 1
extern int CurPasswordSlot;//80077A70 iGp00000660
#endif // GH_UPDATES

void IN_Start(void)//L8003C4D4()
{
	int i, l;

	Valloc_Init();
	ImageToVram(&backpic, "BACK", 0);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		killvalue[i] = itemvalue[i] = secretvalue[i] = fragvalue[i] = 0;

		if (totalkills)
			pstats[i].killpercent = (players[i].killcount * 100) / totalkills;
		else
			pstats[i].killpercent = 100;

		if (totalitems)
			pstats[i].itempercent = (players[i].itemcount * 100) / totalitems;
		else
			pstats[i].itempercent = 100;

		if (totalsecret)
			pstats[i].secretpercent = (players[i].secretcount * 100) / totalsecret;
		else
			pstats[i].secretpercent = 100;

		if (netgame == gt_deathmatch)
			pstats[i].fragcount = players[i].frags;
	}

	acceleratestage = 0;
	last_ticon = ticon;

	if (nextmap < LASTLEVEL)
	{
		Encode_Password(Passwordbuff);
		#if GH_UPDATES == 1
		/* This causes passwords to be kept in the password selection menu and not deleted at the next level. */
		CurPasswordSlot = 10;
		#endif // GH_UPDATES
	}

	psxcd_play_at_andloop(CD_TRACK[cdmusic_intermission],CDVolume,0,0,CD_TRACK[cdmusic_intermission],CDVolume,0,0);
	do {} while (psxcd_elapsed_sectors() == 0);
}

void IN_Stop(int exit)//L8003C7D4()
{
	IN_Drawer();
	psxcd_stop();
}

int IN_Ticker(void)//L8003C7FC()
{
	int i, j, playsound;
	int buttons, oldbuttons;

	if ((ticon - last_ticon) > 60)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			buttons = ticbuttons[i];
			oldbuttons = oldticbuttons[i];

			if ((buttons != oldbuttons) && (buttons & PAD_ACTION))
			{
                acceleratestage++;
                if (acceleratestage == 1)
                {
                    for (j = 0; j < MAXPLAYERS; j++)
                    {
                        killvalue[j] = pstats[j].killpercent;
                        itemvalue[j] = pstats[j].itempercent;
                        secretvalue[j] = pstats[j].secretpercent;
                        fragvalue[j] = pstats[j].fragcount;
                    }
                    S_StartSound(NULL, sfx_barexp);
                }

                if (acceleratestage > 1)
                {
                    S_StartSound(NULL, sfx_barexp);
                    return ga_died;
                }
			}

			if (netgame == gt_single)
				break;
		}

		playsound = false;

		if (gamevbls >= gametic)
			return ga_nothing;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (netgame != gt_deathmatch)
			{
				if (killvalue[i] < pstats[i].killpercent)
				{
					killvalue[i] += 2;
					playsound = true;
					if (killvalue[i] > pstats[i].killpercent)
						killvalue[i] = pstats[i].killpercent;
				}

				if (itemvalue[i] < pstats[i].itempercent)
				{
					itemvalue[i] += 2;
					playsound = true;
					if (itemvalue[i] > pstats[i].itempercent)
						itemvalue[i] = pstats[i].itempercent;
				}

				if (secretvalue[i] < pstats[i].secretpercent)
				{
					secretvalue[i] += 2;
					playsound = true;
					if (secretvalue[i] > pstats[i].secretpercent)
						secretvalue[i] = pstats[i].secretpercent;
				}
			}
			else
			{
				if (pstats[i].fragcount < 0)
				{
					if (pstats[i].fragcount < fragvalue[i])
					{
						fragvalue[i] -= 2;
						playsound = true;
						if (pstats[i].fragcount > fragvalue[i])
							fragvalue[i] = pstats[i].fragcount;
					}
				}
				else
				{
					if (fragvalue[i] < pstats[i].fragcount)
					{
						fragvalue[i] += 2;
						playsound = true;
						if (fragvalue[i] > pstats[i].fragcount)
							fragvalue[i] = pstats[i].fragcount;
					}
				}
			}
		}

		// Play Sound sfx_barexp
		if (!playsound &&  (acceleratestage == 0))
		{
            acceleratestage = 1;
            S_StartSound(NULL, sfx_barexp);
		}

		// Play Sound sfx_pistol
		if (!(gametic & 1) && playsound)
		{
            S_StartSound(NULL, sfx_pistol);
		}
	}

	return ga_nothing;
}

void IN_SingleDrawer(void);
void IN_CooperativeDrawer(void);
void IN_DeathMachtDrawer(void);

void IN_Drawer(void)//L8003CBE0()
{
	NextTextureCacheIdx();

	if (netgame == gt_coop)
		IN_CooperativeDrawer();
	else if (netgame == gt_deathmatch)
		IN_DeathMachtDrawer();
	else
		IN_SingleDrawer();

	UpdateDrawOTag();
	DrawRender();
}

/* */
/* Single intermision */
/* */
void IN_SingleDrawer(void)//L8003CC54()
{
	int i;
	char password[16];

	DrawStaticImage(&backpic, 0, 0, palette[0]);

	ST_DrawText(-1, 20, mapnames[gamemap-1]);

	ST_DrawText(-1, 36, "Finished");

	ST_DrawText(57, 65, "Kills");
	ST_DrawText(182, 65, "%");
	ST_DrawValue(170, 65, killvalue[0]);

	ST_DrawText(53, 91, "Items");
	ST_DrawText(182, 91, "%");
	ST_DrawValue(170, 91, itemvalue[0]);

	ST_DrawText(26, 117, "Secrets");
	ST_DrawText(182, 117, "%");
	ST_DrawValue(170, 117, secretvalue[0]);

	if (nextmap < LASTLEVEL)
	{
		ST_DrawText(-1, 145, "Entering");
		ST_DrawText(-1, 161, mapnames[nextmap-1]);

		ST_DrawText(-1, 187, "Password");

		for (i = 0; i < 10; i++)
		{
			password[i] = passwordChar[Passwordbuff[i]];
		}
		password[i] = 0;

		ST_DrawText(-1, 203, password);
	}
}

/* */
/* Network Cooperative intermission */
/* */
void IN_CooperativeDrawer(void)//L8003CE24()
{
	int i;
	char password[16];

	DrawStaticImage(&backpic, 0, 0, palette[0]);

	DrawImage(statuspic.vtpage, palette[16], 139, 20, facedata[0].x, facedata[0].y, facedata[0].w, facedata[0].h);
	ST_DrawText(130, 52, "you");

	DrawImage(statuspic.vtpage, palette[16], 213, 20, facedata[0].x, facedata[0].y, facedata[0].w, facedata[0].h);
	ST_DrawText(208, 52, "him");

	ST_DrawText(57, 79, "Kills");
	ST_DrawText(155, 79, "%");
	ST_DrawText(228, 79, "%");
	ST_DrawValue(143, 79, killvalue[consoleplayer]);
	ST_DrawValue(216, 79, killvalue[(consoleplayer == 0) ? 1 : 0]);

	ST_DrawText(53, 101, "Items");
	ST_DrawText(155, 101, "%");
	ST_DrawText(228, 101, "%");
	ST_DrawValue(143, 101, itemvalue[consoleplayer]);
	ST_DrawValue(216, 101, itemvalue[(consoleplayer == 0) ? 1 : 0]);

	ST_DrawText(26, 123, "Secrets");
	ST_DrawText(155, 123, "%");
	ST_DrawText(228, 123, "%");
	ST_DrawValue(143, 123, secretvalue[consoleplayer]);
	ST_DrawValue(216, 123, secretvalue[(consoleplayer == 0) ? 1 : 0]);

	if (nextmap < LASTLEVEL)
	{
		ST_DrawText(-1, 149, "Entering");
		ST_DrawText(-1, 165, mapnames[nextmap-1]);

		if (players[consoleplayer].health > 0)
		{
			ST_DrawText(-1, 191, "Password");

			for (i = 0; i < 10; i++)
			{
				password[i] = passwordChar[Passwordbuff[i]];
			}
			password[i] = 0;

			ST_DrawText(-1, 207, password);
		}
	}
}

/* */
/* Network Death Macht intermission */
/* */
void IN_DeathMachtDrawer(void)//L8003D1B8()
{
	int i;
	facedata_t *player1, *player2;

	DrawStaticImage(&backpic, 0, 0, palette[0]);

	ST_DrawText(-1, 20, mapnames[gamemap-1]);
	ST_DrawText(-1, 36, "Finished");

	if (fragvalue[1] < fragvalue[0])
	{
		if (consoleplayer == 0)
		{
			player1 = &facedata[EVILFACE];
			player2 = &facedata[DEADFACE];
		}
		else
		{
			player1 = &facedata[DEADFACE];
			player2 = &facedata[EVILFACE];
		}
	}
	else if (fragvalue[0] < fragvalue[1])
	{
		if (consoleplayer == 0)
		{
			player1 = &facedata[DEADFACE];
			player2 = &facedata[EVILFACE];
		}
		else
		{
			player1 = &facedata[EVILFACE];
			player2 = &facedata[DEADFACE];
		}
	}
	else if(fragvalue[1] == fragvalue[0])
	{
		player1 = &facedata[0];
		player2 = &facedata[0];
	}

	DrawImage(statuspic.vtpage, palette[16], 127, 70, player1->x, player1->y, player1->w, player1->h);
	ST_DrawText(118, 102, "you");

	DrawImage(statuspic.vtpage, palette[16], 200, 70, player2->x, player2->y, player2->w, player2->h);
	ST_DrawText(195, 102, "him");

	ST_DrawText(35, 139, "Frags");
	ST_DrawValue(133, 138, fragvalue[consoleplayer]);
	ST_DrawValue(206, 138, fragvalue[(consoleplayer == 0) ? 1 : 0]);

	if (nextmap < LASTLEVEL)
	{
		ST_DrawText(-1, 190, "Entering");
		ST_DrawText(-1, 206, mapnames[nextmap-1]);
	}
}
