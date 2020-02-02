/* P_inter.c */


#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

#define	BONUSADD		4

/* a weapon is found with two clip loads, a big item has five clip loads */
int		maxammo[NUMAMMO] = {200, 50, 300, 50};//80066F04
int		clipammo[NUMAMMO] = {10, 4, 20, 1};//80066f14

/*
===============================================================================

							GET STUFF

===============================================================================
*/

/*
===================
=
= P_GiveAmmo
=
= Num is the number of clip loads, not the individual count (0= 1/2 clip)
= Returns false if the ammo can't be picked up at all
===================
*/

boolean P_GiveAmmo (player_t *player, ammotype_t ammo, int num)//L8001973C()
{
	int		oldammo;

	if (ammo == am_noammo)
		return false;

	if (ammo > NUMAMMO)
		I_Error ("P_GiveAmmo: bad type %i", ammo);

	if ( player->ammo[ammo] == player->maxammo[ammo]  )
		return false;

	if (num)
		num *= clipammo[ammo];
	else
		num = clipammo[ammo]/2;

	if (gameskill == sk_baby)
		num <<= 1;			/* give double ammo in trainer mode */

	oldammo = player->ammo[ammo];
	player->ammo[ammo] += num;
	if (player->ammo[ammo] > player->maxammo[ammo])
		player->ammo[ammo] = player->maxammo[ammo];

	if (oldammo)
		return true;		/* don't change up weapons, player was lower on */
							/* purpose */

	switch (ammo)
	{
	case am_clip:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_chaingun])
				player->pendingweapon = wp_chaingun;
			else
				player->pendingweapon = wp_pistol;
		}
		break;
	case am_shell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_shotgun])
				player->pendingweapon = wp_shotgun;
		}
		break;
	case am_cell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_plasma])
				player->pendingweapon = wp_plasma;
		}
		break;
	case am_misl:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_missile])
				player->pendingweapon = wp_missile;
		}
	default:
		break;
	}

	return true;
}


/*
===================
=
= P_GiveWeapon
=
= The weapon name may have a MF_DROPPED flag ored in
===================
*/

boolean P_GiveWeapon (player_t *player, weapontype_t weapon, boolean dropped)//L80019924()
{
	boolean		gaveammo, gaveweapon;

	if (netgame == gt_coop && !dropped)
	{	/* leave placed weapons forever on cooperative net games */
		if (player->weaponowned[weapon])
			return false;
		player->weaponowned[weapon] = true;
		P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
		player->pendingweapon = weapon;
		S_StartSound (player->mo, sfx_wpnup);
		return false;
	}

	if (weaponinfo[weapon].ammo != am_noammo)
	{	/* give one clip with a dropped weapon, two clips with a found weapon */
		if (dropped)
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 1);
		else
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
	}
	else
		gaveammo = false;

	if (player->weaponowned[weapon])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[weapon] = true;
		player->pendingweapon = weapon;
		if (player == &players[consoleplayer])
			stbar.specialFace = f_gotgat;
	}

	return gaveweapon || gaveammo;
}



/*
===================
=
= P_GiveBody
=
= Returns false if the body isn't needed at all
===================
*/
//inline
boolean P_GiveBody (player_t *player, int num)//L80019A8C()
{
	if (player->health >= MAXHEALTH)
		return false;

	player->health += num;
	if (player->health > MAXHEALTH)
		player->health = MAXHEALTH;
	player->mo->health = player->health;

	return true;
}


/*
===================
=
= P_GiveArmor
=
= Returns false if the armor is worse than the current armor
===================
*/
//inline
boolean P_GiveArmor (player_t *player, int armortype)//L80019AD8()
{
	int		hits;

	hits = armortype*100;
	if (player->armorpoints >= hits)
		return false;		/* don't pick up */

	player->armortype = armortype;
	player->armorpoints = hits;

	return true;
}


/*
===================
=
= P_GiveCard
=
===================
*/
//inline
void P_GiveCard (player_t *player, card_t card)//L80019B14()
{
	if (player->cards[card])
		return;
	player->bonuscount = BONUSADD;
	player->cards[card] = true;
}


/*
===================
=
= P_GivePower
=
===================
*/
//inline
boolean P_GivePower (player_t *player, powertype_t power)//L80019B40()
{
    if(power >= NUMPOWERS)
        return false;

	switch (power)
	{
	case pw_invulnerability:
		player->powers[power] = INVULNTICS;
		return true;
	case pw_invisibility:
		player->powers[power] = INVISTICS;
		player->mo->flags |= (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3);
		return true;
	case pw_infrared:
		player->powers[power] = INFRATICS;
		return true;
	case pw_ironfeet:
		player->powers[power] = IRONTICS;
		return true;
	case pw_strength:
		P_GiveBody(player, 100);
		player->powers[power] = 1;
		return true;
	default:
		break;
	}
    //pw_allmap
	if (player->powers[power])
		return false;		/* already got it */

	player->powers[power] = 1;
	return true;
}

/*
==================
=
= P_TouchSpecialThing
=
==================
*/

void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher)//L80019C24()
{
	player_t	*player;
	int			i;
	fixed_t		delta;
	int			sound;

	delta = special->z - toucher->z;
	if (delta > toucher->height || delta < -8*FRACUNIT)
		return;			/* out of reach */

	sound = sfx_itemup;
	player = toucher->player;
	if (toucher->health <= 0)
		return;						/* can happen with a sliding player corpse */

	switch (special->sprite)
	{
	/* */
	/* bonus items */
	/* */
	case SPR_BON1:
		player->health+=2;		/* can go over 100% */
		if (player->health > 200)
			player->health = 200;
		player->mo->health = player->health;
		player->message = "You pick up a health bonus.";
		break;
	case SPR_BON2:
		player->armorpoints+=2;		/* can go over 100% */
		if (player->armorpoints > 200)
			player->armorpoints = 200;
		if (!player->armortype)
			player->armortype = 1;
		player->message = "You pick up an armor bonus.";
		break;
	case SPR_SOUL:
		player->health += 100;
		if (player->health > 200)
			player->health = 200;
		player->mo->health = player->health;
		player->message = "Supercharge!";
		sound = sfx_getpow;
		break;
	case SPR_MEGA:
		player->health = 200;
		player->mo->health = 200;
		P_GiveArmor(player, 2);
		player->message = "Mega Sphere!";
		sound = sfx_getpow;
		break;

	/* */
	/* ammo */
	/* */
	case SPR_CLIP:
		if (special->flags & MF_DROPPED)
		{
			if (!P_GiveAmmo (player,am_clip,0))
				return;
		}
		else
		{
			if (!P_GiveAmmo (player,am_clip,1))
				return;
		}
		player->message = "Picked up a clip.";
		break;
	case SPR_AMMO:
		if (!P_GiveAmmo (player, am_clip,5))
			return;
		player->message = "Picked up a box of bullets.";
		break;
	case SPR_ROCK:
		if (!P_GiveAmmo (player, am_misl,1))
			return;
		player->message = "Picked up a rocket.";
		break;
	case SPR_BROK:
		if (!P_GiveAmmo (player, am_misl,5))
			return;
		player->message = "Picked up a box of rockets.";
		break;
	case SPR_CELL:
		if (!P_GiveAmmo (player, am_cell,1))
			return;
		player->message = "Picked up an energy cell.";
		break;
	case SPR_CELP:
		if (!P_GiveAmmo (player, am_cell,5))
			return;
		player->message = "Picked up an energy cell pack.";
		break;
	case SPR_SHEL:
		if (!P_GiveAmmo (player, am_shell,1))
			return;
		player->message = "Picked up 4 shotgun shells.";
		break;
	case SPR_SBOX:
		if (!P_GiveAmmo (player, am_shell,5))
			return;
		player->message = "Picked up a box of shotgun shells.";
		break;
	case SPR_BPAK:
		if (!player->backpack)
		{
			for (i=0 ; i<NUMAMMO ; i++)
				player->maxammo[i] *= 2;
			player->backpack = true;
		}
		for (i=0 ; i<NUMAMMO ; i++)
			P_GiveAmmo (player, i, 1);
		player->message = "You got the backpack!";
		break;


	/* */
	/* weapons */
	/* */
	case SPR_BFUG:
		if (!P_GiveWeapon (player, wp_bfg, false) )
			return;
		player->message = "You got the BFG9000!  Oh, yes.";
		sound = sfx_wpnup;
		break;
	case SPR_MGUN:
		if (!P_GiveWeapon (player, wp_chaingun, special->flags&MF_DROPPED) )
			return;
		player->message = "You got the chaingun!";
		sound = sfx_wpnup;
		break;
	case SPR_CSAW:
		if (!P_GiveWeapon (player, wp_chainsaw, false) )
			return;
		player->message = "A chainsaw!  Find some meat!";
		sound = sfx_wpnup;
		break;
	case SPR_LAUN:
		if (!P_GiveWeapon (player, wp_missile, false) )
			return;
		player->message = "You got the rocket launcher!";
		sound = sfx_wpnup;
		break;
	case SPR_PLAS:
		if (!P_GiveWeapon (player, wp_plasma, false) )
			return;
		player->message = "You got the plasma gun!";
		sound = sfx_wpnup;
		break;
	case SPR_SHOT:
		if (!P_GiveWeapon (player, wp_shotgun, special->flags&MF_DROPPED ) )
			return;
		player->message = "You got the shotgun!";
		sound = sfx_wpnup;
		break;
	case SPR_SGN2:
		if (!P_GiveWeapon(player, wp_supershotgun, special->flags&MF_DROPPED))
			return;
		player->message = "You got the super shotgun!";
		sound = sfx_wpnup;
		break;

		/* */
		/* armor */
		/* */
	case SPR_ARM1:
		if (!P_GiveArmor(player, 1))
			return;
		player->message = "You pick up the armor.";
		break;

	case SPR_ARM2:
		if (!P_GiveArmor(player, 2))
			return;
		player->message = "You got the MegaArmor!";
		break;

		/* */
		/* cards */
		/* leave cards for everyone */
		/* */
	case SPR_BKEY:
		if (!player->cards[it_bluecard])
			player->message = "You pick up a blue keycard.";
		P_GiveCard(player, it_bluecard);
		if (!netgame)
			break;
		return;
    case SPR_RKEY:
		if (!player->cards[it_redcard])
			player->message = "You pick up a red keycard.";
		P_GiveCard(player, it_redcard);
		if (!netgame)
			break;
		return;
	case SPR_YKEY:
		if (!player->cards[it_yellowcard])
			player->message = "You pick up a yellow keycard.";
		P_GiveCard(player, it_yellowcard);
		if (!netgame)
			break;
		return;
	case SPR_BSKU:
		if (!player->cards[it_blueskull])
			player->message = "You pick up a blue skull key.";
		P_GiveCard(player, it_blueskull);
		if (!netgame)
			break;
		return;
    case SPR_RSKU:
		if (!player->cards[it_redskull])
			player->message = "You pick up a red skull key.";
		P_GiveCard(player, it_redskull);
		if (!netgame)
			break;
		return;
	case SPR_YSKU:
		if (!player->cards[it_yellowskull])
			player->message = "You pick up a yellow skull key.";
		P_GiveCard(player, it_yellowskull);
		if (!netgame)
			break;
		return;

		/* */
		/* heals */
		/* */
	case SPR_STIM:
		if (!P_GiveBody(player, 10))
			return;
		player->message = "You pick up a stimpack.";
		break;
	case SPR_MEDI:
		if (!P_GiveBody(player, 25))
			return;
		if (player->health < 25)
			player->message = "You pick up a medikit that you REALLY need!";
		else
			player->message = "You pick up a medikit.";
		break;

		/* */
		/* power ups */
		/* */
	case SPR_PINV:
		P_GivePower(player, pw_invulnerability);
		player->message = "Invulnerability!";
		sound = sfx_getpow;
		break;
	case SPR_PSTR:
		P_GivePower(player, pw_strength);
		player->message = "Berserk!";
		if (player->readyweapon != wp_fist)
			player->pendingweapon = wp_fist;
		sound = sfx_getpow;
		break;
	case SPR_PINS:
		P_GivePower(player, pw_invisibility);
		player->message = "Partial Invisibility!";
		sound = sfx_getpow;
		break;
	case SPR_SUIT:
		P_GivePower(player, pw_ironfeet);
		player->message = "Radiation Shielding Suit";
		sound = sfx_getpow;
		break;
	case SPR_PMAP:
		if (!P_GivePower(player, pw_allmap))
			return;
		player->message = "Computer Area Map";
		sound = sfx_getpow;
		break;
	case SPR_PVIS:
		P_GivePower(player, pw_infrared);
		player->message = "Light Amplification Goggles";
		sound = sfx_getpow;
		break;

	default:
		//I_Error("P_SpecialThing: Unknown gettable thing");
		break;
	}

	if (special->flags & MF_COUNTITEM)
		player->itemcount++;

	P_RemoveMobj (special);
	player->bonuscount += BONUSADD;

	//[GEC] Info: (NULL parameter) This is the reason why items always sound echo when picked up.
	if (player == &players[consoleplayer])
		S_StartSound(NULL, sound);//S_StartSound (toucher, sound);
}

/*
==============
=
= KillMobj
=
==============
*/

void P_KillMobj (mobj_t *source, mobj_t *target)//L8001A514()
{
	mobjtype_t		item;
	mobj_t			*mo;
	boolean         forceXdeath;

	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

	if (target->type != MT_SKULL)
		target->flags &= ~MF_NOGRAVITY;

	target->flags |= MF_CORPSE|MF_DROPOFF;
	target->height >>= 2;

	forceXdeath = false;    //New PsxDoom

	if (target->player)
	{	/* a frag of one sort or another */
		if (!source || !source->player || source->player == target->player)
		{	/* killed self somehow */
			target->player->frags--;
			//if (target->player->frags < 0)
				//target->player->frags = 0;
		}
		else
		{	/* killed by other player */
			source->player->frags++;
		}

		/* else just killed by a monster */
	}
	else if (source && source->player && (target->flags & MF_COUNTKILL) )
	{	/* a deliberate kill by a player */
		source->player->killcount++;		/* count for intermission */
	}
	else if (!netgame && (target->flags & MF_COUNTKILL) )
		players[0].killcount++;			/* count all monster deaths, even */
										/* those caused by other monsters */

	if (target->player)
	{
		target->flags &= ~MF_SOLID;
		target->player->playerstate = PST_DEAD;
		P_DropWeapon (target->player);
		if (target->health < -50)
		{
		    forceXdeath = true; //Force the player to the state of Xdeath

			if (target->player == &players[consoleplayer])
				stbar.gotgibbed = true;
			S_StartSound (target, sfx_slop);
		}
		else
			S_StartSound (target, sfx_pldeth);

		// Psx Doom New
		if (playercounttarget >= MAXTARGET)
			P_RemoveMobj(playertarget[playercounttarget & (MAXTARGET-1)]);

		playertarget[playercounttarget & (MAXTARGET-1)] = target;
		playercounttarget++;
	}

	if (forceXdeath || (target->health < -target->info->spawnhealth) && target->info->xdeathstate)
		P_SetMobjState (target, target->info->xdeathstate);
	else
		P_SetMobjState (target, target->info->deathstate);

	target->tics -= P_Random()&1;
	if (target->tics < 1)
		target->tics = 1;

	/* */
	/* drop stuff */
	/* */
	switch (target->type)
	{
	case MT_POSSESSED:
		item = MT_CLIP;
		break;
	case MT_SHOTGUY:
		item = MT_SHOTGUN;
		break;
	case MT_CHAINGUY:
		item = MT_CHAINGUN;
		break;
	default:
		return;
	}

	mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
	mo->flags |= MF_DROPPED;		/* special versions of items */
}



/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

void P_DamageMobj (mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage)//L8001A838()
{
	unsigned	ang, an;
	int			saved;
	player_t	*player;
	fixed_t		thrust;

	if (!(target->flags & MF_SHOOTABLE))
		return;						/* shouldn't happen... */

	if (target->health <= 0)
		return;

	if (target->flags & MF_SKULLFLY)
	{
		target->momx = target->momy = target->momz = 0;
	}

	player = target->player;
	if (player && gameskill == sk_baby)
		damage >>= 1;				/* take half damage in trainer mode */

	if (player && (damage > 30) && player == &players[consoleplayer])
		stbar.specialFace = f_hurtbad;
	/* */
	/* kick away unless using the chainsaw */
	/* */
	if (inflictor && (!source || !source->player || source->player->readyweapon != wp_chainsaw))
	{
		ang = R_PointToAngle2 (inflictor->x, inflictor->y, target->x, target->y);

		thrust = (damage * ((FRACUNIT >> 2) * 100)) / target->info->mass;

		/* make fall forwards sometimes */
		if ( (damage < 40) && (damage > target->health) && (target->z - inflictor->z > (64*FRACUNIT)) && (P_Random ()&1))
		{
			ang += ANG180;
			thrust *= 4;
		}

		an = ang >> ANGLETOFINESHIFT;
		thrust >>= 16;
		target->momx += thrust * finecosine[an];
		target->momy += thrust * finesine[an];

		if (target->player)//psx new
		{
            if (target->momx > MAXMOVE)
                target->momx = MAXMOVE;
            else if (target->momx < -MAXMOVE)
                target->momx = -MAXMOVE;

            if (target->momy > MAXMOVE)
                target->momy = MAXMOVE;
            else if (target->momy < -MAXMOVE)
                target->momy = -MAXMOVE;
		}
	}
	else
    {
		ang = target->angle;
    }

	/* */
	/* player specific */
	/* */
	if (player)
	{
		if ((player->cheats&CF_GODMODE) || player->powers[pw_invulnerability])
			return;

		if (player == &players[consoleplayer])
		{
			ang -= target->angle;
			if (ang > 0x30000000 && ang <0x80000000)
				stbar.specialFace = f_faceright;
			else if (ang >0x80000000 && ang < 0xd0000000)
				stbar.specialFace = f_faceleft;
		}
		if (player->armortype)
		{
			if (player->armortype == 1)
				saved = damage/3;
			else
				saved = damage/2;
			if (player->armorpoints <= saved)
			{	/* armor is used up */
				saved = player->armorpoints;
				player->armortype = 0;
			}
			player->armorpoints -= saved;
			damage -= saved;
		}
		S_StartSound (target,sfx_plpain);
		player->health -= damage;		/* mirror mobj health here for Dave */
		if (player->health < 0)
			player->health = 0;
		player->attacker = source;
		player->damagecount += (damage>>1);	/* add damage after armor / invuln */
	}

	/* */
	/* do the damage */
	/* */
	target->health -= damage;
	if (target->health <= 0)
	{
		P_KillMobj (source, target);
		return;
	}

	if ( (P_Random () < target->info->painchance) && !(target->flags&MF_SKULLFLY) )
	{
		target->flags |= MF_JUSTHIT;		/* fight back! */
		P_SetMobjState (target, target->info->painstate);
	}

	target->reactiontime = 0;		/* we're awake now...	 */
	if (!target->threshold && source)
	{	/* if not intent on another player, chase after this one */
		target->target = source;
		target->threshold = BASETHRESHOLD;
		if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_NULL)
        {
			P_SetMobjState (target, target->info->seestate);
        }
	}
}

