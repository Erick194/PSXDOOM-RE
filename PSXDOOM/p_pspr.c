/* P_pspr.c */

#include "doomdef.h"
#include "p_local.h"

#define	LOWERSPEED		FRACUNIT*12
#define	RAISESPEED		FRACUNIT*12

#define	WEAPONX		    FRACUNIT
#define WEAPONBOTTOM	(96)*FRACUNIT	//old 128
#define WEAPONTOP		(0)*FRACUNIT	//old 32

#define	BFGCELLS		40			/* plasma cells for a bfg attack */

/*
=================
=
= P_NoiseAlert
=
= If a monster yells at a player, it will alert other monsters to the player
=
=================
*/

mobj_t *soundtarget;//pmGp00000a18

//inline
void P_RecursiveSound (sector_t *sec, int soundblocks)//L8001F8B0()
{
	int			i;
	line_t		*check;
	sector_t	*other;
	sector_t	*front, *back;

	/* wake up all monsters in this sector */
	if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
		return;		/* already flooded */

	sec->validcount = validcount;
	sec->soundtraversed = soundblocks+1;
	sec->soundtarget = soundtarget;

	for (i=0 ;i<sec->linecount ; i++)
	{
		check = sec->lines[i];
		back = check->backsector;
		if (!back)
			continue;		/* single sided */

		front = check->frontsector;
		if (front->floorheight >= back->ceilingheight || front->ceilingheight <= back->floorheight)
			continue;		/* closed door */

		if ( front == sec)
			other = back;
		else
			other = front;

		if (check->flags & ML_SOUNDBLOCK)
		{
			if (!soundblocks)
				P_RecursiveSound (other, 1);
		}
		else
			P_RecursiveSound (other, soundblocks);
	}
}


/*
=================
=
= P_NoiseAlert
=
=================
*/

//inline
void P_NoiseAlert (player_t *player)//L8001F9CC()
{
	sector_t	*sec;

	sec = player->mo->subsector->sector;

	if (player->lastsoundsector == (void *)sec)
		return;		/* don't bother doing it again here */

    soundtarget = player->mo;
	player->lastsoundsector = (void *)sec;

	validcount++;
	P_RecursiveSound (sec, 0);
}


/*
==================
=
= P_SetPsprite
=
==================
*/

//inline
void P_SetPsprite (player_t *player, int position, statenum_t stnum) //L8001FB08()
{
	pspdef_t	*psp;
	state_t	*state;

	psp = &player->psprites[position];

	do
	{
		if (!stnum)
		{
			psp->state = NULL;
			break;		/* object removed itself */
		}
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics;  /* could be 0 */

		/* call action routine */
		if (state->action)
		{
			state->action (player, psp);
			if (!psp->state)
				break;
		}
		stnum = psp->state->nextstate;
	} while (!psp->tics);	/* an initial state of 0 could cycle through */
}


/*
===============================================================================

						PSPRITE ACTIONS

===============================================================================
*/

weaponinfo_t	weaponinfo[NUMWEAPONS] = //80066f24
{
	{	/* fist */
/* ammo 		*/	am_noammo,
/* upstate 		*/	S_PUNCHUP,
/* downstate 	*/	S_PUNCHDOWN,
/* readystate 	*/	S_PUNCH,
/* atkstate 	*/	S_PUNCH1,
/* flashstate 	*/	S_NULL
	},

	{	/* pistol */
/* ammo 		*/	am_clip,
/* upstate 		*/	S_PISTOLUP,
/* downstate 	*/	S_PISTOLDOWN,
/* readystate 	*/	S_PISTOL,
/* atkstate 	*/	S_PISTOL2,
/* flashstate 	*/	S_PISTOLFLASH
	},

	{	/* shotgun */
/* ammo 		*/	am_shell,
/* upstate 		*/	S_SGUNUP,
/* downstate 	*/	S_SGUNDOWN,
/* readystate 	*/	S_SGUN,
/* atkstate 	*/	S_SGUN2,
/* flashstate 	*/	S_SGUNFLASH1
	},

    {	/* super shotgun */
/* ammo 		*/	am_shell,
/* upstate 		*/	S_DSGUNUP,
/* downstate 	*/	S_DSGUNDOWN,
/* readystate 	*/	S_DSGUN,
/* atkstate 	*/	S_DSGUN1,
/* flashstate 	*/	S_DSGUNFLASH1
    },

	{	/* chaingun */
/* ammo 		*/	am_clip,
/* upstate 		*/	S_CHAINUP,
/* downstate 	*/	S_CHAINDOWN,
/* readystate 	*/	S_CHAIN,
/* atkstate 	*/	S_CHAIN1,
/* flashstate 	*/	S_CHAINFLASH1
	},

	{	/* missile */
/* ammo 		*/	am_misl,
/* upstate 		*/	S_MISSILEUP,
/* downstate 	*/	S_MISSILEDOWN,
/* readystate 	*/	S_MISSILE,
/* atkstate 	*/	S_MISSILE1,
/* flashstate 	*/	S_MISSILEFLASH1
	},

	{	/* plasma */
/* ammo 		*/	am_cell,
/* upstate 		*/	S_PLASMAUP,
/* downstate 	*/	S_PLASMADOWN,
/* readystate 	*/	S_PLASMA,
/* atkstate 	*/	S_PLASMA1,
/* flashstate 	*/	S_PLASMAFLASH1
	},

	{	/* bfg */
/* ammo 		*/	am_cell,
/* upstate 		*/	S_BFGUP,
/* downstate 	*/	S_BFGDOWN,
/* readystate 	*/	S_BFG,
/* atkstate 	*/	S_BFG1,
/* flashstate 	*/	S_BFGFLASH1
	},

	{	/* saw */
/* ammo 		*/	am_noammo,
/* upstate 		*/	S_SAWUP,
/* downstate 	*/	S_SAWDOWN,
/* readystate 	*/	S_SAW,
/* atkstate 	*/	S_SAW1,
/* flashstate 	*/	S_NULL
	}
};


/*
================
=
= P_BringUpWeapon
=
= Starts bringing the pending weapon up from the bottom of the screen
= Uses player
================
*/

//inline
void P_BringUpWeapon (player_t *player)//L8001FBB0()
{
	statenum_t	new;

	if (player->pendingweapon == wp_nochange)
		player->pendingweapon = player->readyweapon;

	if (player->pendingweapon == wp_chainsaw)
	{
		if(disableload)// Psx Doom
			S_StartSound(player->mo, sfx_sawup);
	}

	new = weaponinfo[player->pendingweapon].upstate;

	player->pendingweapon = wp_nochange;
	player->psprites[ps_weapon].sx = WEAPONX;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	P_SetPsprite (player, ps_weapon, new);
}

/*
================
=
= P_CheckAmmo
=
= returns true if there is enough ammo to shoot
= if not, selects the next weapon to use
================
*/

boolean P_CheckAmmo (player_t *player)//L8001FCE4()
{
	ammotype_t	ammo;
	int			count;

	ammo = weaponinfo[player->readyweapon].ammo;
	if (player->readyweapon == wp_bfg)
		count = BFGCELLS;
	else if (player->readyweapon == wp_supershotgun)
		count = 2;	// Double barrel.
	else
		count = 1;
	if (ammo == am_noammo || player->ammo[ammo] >= count)
		return true;

	/* out of ammo, pick a weapon to change to */
	do
	{
		if (player->weaponowned[wp_plasma] && player->ammo[am_cell])
			player->pendingweapon = wp_plasma;
		else if (player->weaponowned[wp_supershotgun] && player->ammo[am_shell] > 2)
			player->pendingweapon = wp_supershotgun;
		else if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
			player->pendingweapon = wp_chaingun;
		else if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
			player->pendingweapon = wp_shotgun;
		else if (player->ammo[am_clip])
			player->pendingweapon = wp_pistol;
		else if (player->weaponowned[wp_chainsaw])
			player->pendingweapon = wp_chainsaw;
		else if (player->weaponowned[wp_missile] && player->ammo[am_misl])
			player->pendingweapon = wp_missile;
		else if (player->weaponowned[wp_bfg] && player->ammo[am_cell]>40)
			player->pendingweapon = wp_bfg;
		else
			player->pendingweapon = wp_fist;
	} while (player->pendingweapon == wp_nochange);

	P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);

	return false;
}


/*
================
=
= P_FireWeapon
=
================
*/

void P_FireWeapon (player_t *player)//L8001FF54()
{
	statenum_t	new;

	if (!P_CheckAmmo (player))
		return;

	P_SetMobjState (player->mo, S_PLAY_ATK1);

	player->psprites[ps_weapon].sx = WEAPONX;
	player->psprites[ps_weapon].sy = WEAPONTOP;
	new = weaponinfo[player->readyweapon].atkstate;
	P_SetPsprite (player, ps_weapon, new);
	P_NoiseAlert (player);
}


/*
================
=
= P_DropWeapon
=
= Player died, so put the weapon away
================
*/

void P_DropWeapon (player_t *player)//L8002015C()
{
	P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}


/*
=================
=
= A_WeaponReady
=
= The player can fire the weapon or change to another weapon at this time
=
=================
*/

void A_WeaponReady (player_t *player, pspdef_t *psp)//L80020230()
{
	statenum_t	new;
	int			angle;

	if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
    {
		S_StartSound (player->mo, sfx_sawidl);
    }

	/* */
	/* check for change */
	/*  if player is dead, put the weapon away */
	/* */
	if (player->pendingweapon != wp_nochange || !player->health)
	{
	/* change weapon (pending weapon should allready be validated) */
		new = weaponinfo[player->readyweapon].downstate;
		P_SetPsprite (player, ps_weapon, new);
		return;
	}

	/* */
	/* check for fire */
	/* */
	/* the missile launcher and bfg do not auto fire */
	if (ticbuttons[playernum] & BT_DATA[playernum]->BT_ATTACK)
	{
		P_FireWeapon (player);
		return;
	}

	/* */
	/* bob the weapon based on movement speed */
	/* */
	//angle = (64*gamevbls)&(FINEANGLES-1);
	angle = (64*ticon)&(FINEANGLES-1);//PsxDoom use ticon no gamevbls
	psp->sx = WEAPONX + (player->bob>>16) * finecosine[angle];
	angle &= FINEANGLES/2-1;
	psp->sy = WEAPONTOP + (player->bob>>16) * finesine[angle];
}


/*
=================
=
= A_ReFire
=
= The player can re fire the weapon without lowering it entirely
=
=================
*/

//inline
void A_ReFire (player_t *player, pspdef_t *psp)//L80020418()
{
	/* */
	/* check for fire (if a weaponchange is pending, let it go through instead) */
	/* */
	if ((ticbuttons[playernum] & BT_DATA[playernum]->BT_ATTACK)
	&& player->pendingweapon == wp_nochange && player->health)
	{
		player->refire++;
		P_FireWeapon (player);
	}
	else
	{
		player->refire = 0;
		P_CheckAmmo (player);
	}
}

/*
=================
=
= A_CheckReload
=
=================
*/

void A_CheckReload(player_t *player, pspdef_t *psp)//L800204B4()
{
	P_CheckAmmo(player);
}

/*
=================
=
= A_Lower
=
=================
*/

void A_Lower (player_t *player, pspdef_t *psp)//L800204D4()
{
	psp->sy += LOWERSPEED;
	if (psp->sy < WEAPONBOTTOM )
		return;
	if (player->playerstate == PST_DEAD)
	{
		psp->sy = WEAPONBOTTOM;
		return;		/* don't bring weapon back up */
	}

	/* */
	/* The old weapon has been lowered off the screen, so change the weapon */
	/* and start raising it */
	/* */
	if (!player->health)
	{	/* player is dead, so keep the weapon off screen */
		P_SetPsprite (player,  ps_weapon, S_NULL);
		return;
	}

	player->readyweapon = player->pendingweapon;

	P_BringUpWeapon (player);
}


/*
=================
=
= A_Raise
=
=================
*/

void A_Raise (player_t *player, pspdef_t *psp)//L8002064C()
{
	statenum_t	new;

	psp->sy -= RAISESPEED;

	if (psp->sy > WEAPONTOP )
		return;

	psp->sy = WEAPONTOP;

	/* */
	/* the weapon has been raised all the way, so change to the ready state */
	/* */
	new = weaponinfo[player->readyweapon].readystate;

	P_SetPsprite (player, ps_weapon, new);
}


/*
================
=
= A_GunFlash
=
=================
*/

void A_GunFlash (player_t *player, pspdef_t *psp) //L80020738()
{
	P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);
}


/*
===============================================================================

						WEAPON ATTACKS

===============================================================================
*/


/*
==================
=
= A_Punch
=
==================
*/

void A_Punch (player_t *player, pspdef_t *psp) //L8002080C()
{
	angle_t		angle;
	int			damage;

	damage = ((P_Random ()&7)+1)*3;
	if (player->powers[pw_strength])
		damage *= 10;
	angle = player->mo->angle;
	angle += (angle_t)(P_Random()-P_Random())<<18;
	P_LineAttack (player->mo, angle, MELEERANGE, MAXINT, damage);
    /* turn to face target */
	if (linetarget)
	{
	    S_StartSound(player->mo, sfx_punch);
		player->mo->angle = R_PointToAngle2 (player->mo->x, player->mo->y, linetarget->x, linetarget->y);
	}
}

/*
==================
=
= A_Saw
=
==================
*/

void A_Saw (player_t *player, pspdef_t *psp) //L80020904()
{
	angle_t		angle;
	int			damage;


	damage = ((P_Random ()&7)+1)*3;
	angle = player->mo->angle;
	angle += (angle_t)(P_Random()-P_Random())<<18;
	/* use meleerange + 1 se the puff doesn't skip the flash */
	P_LineAttack (player->mo, angle, MELEERANGE+1, MAXINT, damage);
	if (!linetarget)
	{
		S_StartSound (player->mo, sfx_sawful);
		return;
	}
	S_StartSound (player->mo, sfx_sawhit);

	/* turn to face target */
	angle = R_PointToAngle2 (player->mo->x, player->mo->y, linetarget->x, linetarget->y);
	if (angle - player->mo->angle > ANG180)
	{
		if (angle - player->mo->angle < -ANG90/20)
			player->mo->angle = angle + ANG90/21;
		else
			player->mo->angle -= ANG90/20;
	}
	else
	{
		if (angle - player->mo->angle > ANG90/20)
			player->mo->angle = angle - ANG90/21;
		else
			player->mo->angle += ANG90/20;
	}
	player->mo->flags |= MF_JUSTATTACKED;
}


/*
==================
=
= A_FireMissile
=
==================
*/

void A_FireMissile (player_t *player, pspdef_t *psp) //L80020A7C()
{
	player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SpawnPlayerMissile (player->mo, MT_ROCKET);
}


/*
==================
=
= A_FireBFG
=
==================
*/

void A_FireBFG (player_t *player, pspdef_t *psp) //L80020AE0()
{
	player->ammo[weaponinfo[player->readyweapon].ammo] -= BFGCELLS;
	P_SpawnPlayerMissile (player->mo, MT_BFG);
}


/*
==================
=
= A_FirePlasma
=
==================
*/

void A_FirePlasma (player_t *player, pspdef_t *psp) //L80020B44()
{
	player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate+(P_Random ()&1) );
	P_SpawnPlayerMissile (player->mo, MT_PLASMA);
}

/*
===============
=
= P_BulletSlope
=
===============
*/

fixed_t bulletslope;   //fGp00000a0c
//inline
void P_BulletSlope(mobj_t*	mo)//L80020C6C()
{
	angle_t	an;

	// see which target is to be aimed at
	an = mo->angle;
	bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);

	if (!linetarget)
	{
		an += 1 << 26;
		bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
		if (!linetarget)
		{
			an -= 2 << 26;
			bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
		}
	}
}

/*
===============
=
= P_GunShot
=
===============
*/
//inline
void P_GunShot (mobj_t *mo, boolean accurate)//L80020CF8()
{
	angle_t		angle;
	int			damage;

	damage = ((P_Random ()&3)+1)*4;
	angle = mo->angle;
	if (!accurate)
		angle += (P_Random()-P_Random())<<18;
	P_LineAttack (mo, angle, MISSILERANGE, MAXINT, damage);
}

/*
==================
=
= A_FirePistol
=
==================
*/

void A_FirePistol (player_t *player, pspdef_t *psp) //L80020D88()
{
	S_StartSound (player->mo, sfx_pistol);

	player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);

	P_GunShot (player->mo, !player->refire);
}

/*
==================
=
= A_FireShotgun
=
==================
*/

void A_FireShotgun (player_t *player, pspdef_t *psp) //L80020F14()
{
	angle_t		angle;
	int			damage;
	int			i;
	int			slope;

	S_StartSound (player->mo, sfx_shotgn);

	player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);

	slope = P_AimLineAttack (player->mo, player->mo->angle, MISSILERANGE);

/* shotgun pellets all go at a fixed slope */

	for (i=0 ; i<7 ; i++)
	{
		damage = ((P_Random ()&3)+1)*4;
		angle = player->mo->angle;
		angle += (P_Random()-P_Random())<<18;
		P_LineAttack (player->mo, angle, MISSILERANGE, slope, damage);
	}
}

/*
==================
=
= A_FireShotgun2
=
==================
*/

void A_FireShotgun2(player_t *player, pspdef_t *psp)//L800210C4()
{
    angle_t		angle;
	int			damage;
	int			i;

	S_StartSound(player->mo, sfx_dshtgn);
	P_SetMobjState(player->mo, S_PLAY_ATK2);

	player->ammo[weaponinfo[player->readyweapon].ammo] -= 2;
	P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

	P_BulletSlope(player->mo);

	for (i = 0; i<20; i++)
	{
		damage = 5 * (P_Random() % 3 + 1);
		angle = player->mo->angle;
		angle += (P_Random() - P_Random()) << 19;
		P_LineAttack(player->mo, angle, MISSILERANGE, bulletslope + ((P_Random() - P_Random()) << 5), damage);
	}
}

/*
==================
=
= A_CockSgun
=
==================
*/
#if 0 //No Used In PSX Doom
void A_CockSgun (player_t *player, pspdef_t *psp)
{
	S_StartSound (player->mo, sfx_sgcock);
}
#endif // 0

/*
==================
=
= A_FireCGun
=
==================
*/

void A_FireCGun (player_t *player, pspdef_t *psp) //L8002130C()
{
	S_StartSound (player->mo, sfx_pistol);

	if (!player->ammo[weaponinfo[player->readyweapon].ammo])
		return;

	player->ammo[weaponinfo[player->readyweapon].ammo]--;

	P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate + psp->state - &states[S_CHAIN1]);

	P_GunShot (player->mo, !player->refire);
}


/*============================================================================= */


void A_Light0 (player_t *player, pspdef_t *psp)//L800214F4()
{
	player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)//L800214FC()
{
	player->extralight = 8;
}

void A_Light2 (player_t *player, pspdef_t *psp)//L80021508()
{
	player->extralight = 16;
}

/*
================
=
= A_BFGSpray
=
= Spawn a BFG explosion on every monster in view
=
=================
*/
//inline
void A_BFGSpray (mobj_t *mo) //L80021514()
{
	int			i, j, damage;
	angle_t		an;

	/* offset angles from its attack angle */
	for (i=0 ; i<40 ; i++)
	{
		an = mo->angle - ANG90/2 + ANG90/40*i;
		/* mo->target is the originator (player) of the missile */
		P_AimLineAttack (mo->target, an, 16*64*FRACUNIT);
		if (!linetarget)
			continue;
		P_SpawnMobj (linetarget->x, linetarget->y, linetarget->z + (linetarget->height>>2), MT_EXTRABFG);
		damage = 0;
		for (j=0;j<15;j++)
        {
			damage += (P_Random()&7) + 1;
        }
		P_DamageMobj (linetarget, mo->target,mo->target, damage);
	}
}

/*
================
=
= A_BFGsound
=
=================
*/

void A_BFGsound (player_t *player, pspdef_t *psp)//L80021604()
{
	S_StartSound (player->mo, sfx_bfg);
}

/*
================
=
= A_OpenShotgun2
=
=================
*/

void A_OpenShotgun2(player_t *player, pspdef_t *psp)//L80021628()
{
	S_StartSound(player->mo, sfx_dbopn);
}

/*
================
=
= A_LoadShotgun2
=
=================
*/

void A_LoadShotgun2(player_t *player, pspdef_t *psp)//L8002164C()
{
	S_StartSound(player->mo, sfx_dbload);
}

/*
================
=
= A_CloseShotgun2
=
=================
*/

void A_CloseShotgun2(player_t *player, pspdef_t *psp)//L80021670()
{
	S_StartSound(player->mo, sfx_dbcls);
	A_ReFire(player, psp);
}

/*============================================================================= */

int		ticremainder[MAXPLAYERS];//0x80077ebc

/*
==================
=
= P_SetupPsprites
=
= Called at start of level for each player
==================
*/

void P_SetupPsprites (int curplayer) //L8002172C() //(player_t *player)
{
	int	i;
	player_t *player;

	ticremainder[curplayer] = 0;
	player = &players[curplayer];

	/* remove all psprites */

	for (i=0 ; i<NUMPSPRITES ; i++)
		player->psprites[i].state = NULL;

	/* spawn the gun */
	player->pendingweapon = player->readyweapon;
	P_BringUpWeapon (player);
}



/*
==================
=
= P_MovePsprites
=
= Called every tic by player thinking routine
==================
*/

void P_MovePsprites (player_t *player) //L800218A4()
{
	int			i;
	pspdef_t	*psp;
	state_t		*state;

	ticremainder[playernum] += vblsinframe[playernum];

	while (ticremainder[playernum] >= 4)
	{
		ticremainder[playernum] -= 4;

		psp = &player->psprites[0];
		for (i=0 ; i<NUMPSPRITES ; i++, psp++)
		{
			if ( (state = psp->state) != 0)		/* a null state means not active */
			{
			/* drop tic count and possibly change state */
				if (psp->tics != -1)	/* a -1 tic count never changes */
				{
					psp->tics--;
					if (!psp->tics)
						P_SetPsprite (player, i, psp->state->nextstate);
				}
			}
		}
	}

	player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
	player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}


