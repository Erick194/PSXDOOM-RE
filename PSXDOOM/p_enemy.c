/* P_enemy.c */

#include "doomdef.h"
#include "p_local.h"

void A_Fall (mobj_t *actor);

/*
===============================================================================

							ENEMY THINKING

enemies are allways spawned with targetplayer = -1, threshold = 0
Most monsters are spawned unaware of all players, but some can be made preaware

===============================================================================
*/

/*
================
=
= P_CheckMeleeRange
=
================
*/
//inline
boolean P_CheckMeleeRange (mobj_t *actor)//L80015C40()
{
	mobj_t		*pl;
	fixed_t		dist;

	if (!(actor->flags&MF_SEETARGET))
		return false;

	if (!actor->target)
		return false;

	pl = actor->target;
	dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);
	if (dist >= MELEERANGE)
		return false;

	return true;
}

/*
================
=
= P_CheckMissileRange
=
================
*/
//inline
boolean P_CheckMissileRange (mobj_t *actor)//L80015CB4()
{
	fixed_t		dist;

	if (!(actor->flags & MF_SEETARGET))
		return false;

	if (actor->flags & MF_JUSTHIT)
	{	/* the target just hit the enemy, so fight back! */
		actor->flags &= ~MF_JUSTHIT;
		return true;
	}

	if (actor->reactiontime)
		return false;		/* don't attack yet */

	dist = P_AproxDistance ( actor->x-actor->target->x, actor->y-actor->target->y) - 64*FRACUNIT;
	if (!actor->info->meleestate)
		dist -= 128*FRACUNIT;		/* no melee attack, so fire more */

	dist >>= 16;

	if (actor->type == MT_SKULL)
		dist >>= 1;

	if (dist > 200)
		dist = 200;

	if (P_Random() < dist)
		return false;

	return true;
}


/*
================
=
= P_Move
=
= Move in the current direction
= returns false if the move is blocked
================
*/

fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};//80066e90
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};//80066eb0

extern	line_t *blockline;//8007806c

boolean P_Move (mobj_t *actor)//L80015D98()
{
	fixed_t	tryx, tryy;
	boolean		good;
	line_t		*blkline;

	if (actor->movedir == DI_NODIR)
		return false;

	tryx = actor->x + actor->info->speed * xspeed[actor->movedir];
	tryy = actor->y + actor->info->speed * yspeed[actor->movedir];

	if (!P_TryMove (actor, tryx, tryy) )
	{	/* open any specials */
		if (actor->flags & MF_FLOAT && floatok)
		{	/* must adjust height */
			if (actor->z < tmfloorz)
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;

			actor->flags |= MF_INFLOAT;
			return true;
		}

		blkline = blockline;//(line_t *)DSPRead (&blockline);
		if (!blkline || !blkline->special)
			return false;

		actor->movedir = DI_NODIR;
		good = false;
        /* if the special isn't a door that can be opened, return false */
		if (P_UseSpecialLine (actor, blkline))
			good = true;
		return good;
	}
	else
		actor->flags &= ~MF_INFLOAT;

	if (!(actor->flags & MF_FLOAT))
		actor->z = actor->floorz;
	return true;
}


/*
==================================
=
= TryWalk
=
= Attempts to move actoron in its current (ob->moveangle) direction.
=
= If blocked by either a wall or an actor returns FALSE
= If move is either clear or blocked only by a door, returns TRUE and sets
= If a door is in the way, an OpenDoor call is made to start it opening.
=
==================================
*/
//inline
boolean P_TryWalk (mobj_t *actor)//L80015F00()
{
	if (!P_Move (actor))
		return false;

	actor->movecount = P_Random()&15;
	return true;
}


/*
================
=
= P_NewChaseDir
=
================
*/

dirtype_t opposite[] = //80066ed0
{DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST, DI_EAST, DI_NORTHEAST,
DI_NORTH, DI_NORTHWEST, DI_NODIR};

dirtype_t diags[] = {DI_NORTHWEST,DI_NORTHEAST,DI_SOUTHWEST,DI_SOUTHEAST};//80066ef4

void P_NewChaseDir (mobj_t *actor)//L80015F4C()
{
	fixed_t		deltax,deltay;
	dirtype_t	d[3];
	dirtype_t	tdir, olddir, turnaround;

	if (!actor->target)
		I_Error ("P_NewChaseDir: called with no target");

	olddir = actor->movedir;
	turnaround=opposite[olddir];

	deltax = actor->target->x - actor->x;
	deltay = actor->target->y - actor->y;

	if (deltax>10*FRACUNIT)
		d[1]= DI_EAST;
	else if (deltax<-10*FRACUNIT)
		d[1]= DI_WEST;
	else
		d[1]=DI_NODIR;

	if (deltay<-10*FRACUNIT)
		d[2]= DI_SOUTH;
	else if (deltay>10*FRACUNIT)
		d[2]= DI_NORTH;
	else
		d[2]=DI_NODIR;

/* try direct route */
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
		if (actor->movedir != turnaround && P_TryWalk(actor))
			return;
	}

/* try other directions */
	if (P_Random() > 200 ||  abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]==turnaround)
		d[1]=DI_NODIR;

	if (d[2]==turnaround)
		d[2]=DI_NODIR;

	if (d[1]!=DI_NODIR)
	{
		actor->movedir = d[1];
		if (P_TryWalk(actor))
			return;     /*either moved forward or attacked*/
	}

	if (d[2]!=DI_NODIR)
	{
		actor->movedir =d[2];
		if (P_TryWalk(actor))
			return;
	}

/* there is no direct path to the player, so pick another direction */

	if (olddir!=DI_NODIR)
	{
		actor->movedir =olddir;
		if (P_TryWalk(actor))
			return;
	}

	if (P_Random()&1) 	/*randomly determine direction of search*/
	{
		for (tdir=DI_EAST ; tdir<=DI_SOUTHEAST ; tdir++)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
				if ( P_TryWalk(actor) )
					return;
			}
		}
	}
	else
	{
		for (tdir=DI_SOUTHEAST ; (int)tdir >= (int)DI_EAST;tdir--)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
				if ( P_TryWalk(actor) )
				return;
			}
		}
	}

	if (turnaround !=  DI_NODIR)
	{
		actor->movedir =turnaround;
		if ( P_TryWalk(actor) )
			return;
	}

	actor->movedir = DI_NODIR;		/* can't move */
}


/*
================
=
= P_LookForPlayers
=
= If allaround is false, only look 180 degrees in front
= returns true if a player is targeted
================
*/

boolean P_LookForPlayers (mobj_t *actor, boolean allaround)//L800162CC()
{
	angle_t		an;
	fixed_t		dist;
	mobj_t		*mo;
	int		 plyrnd;

	if (!(actor->flags & MF_SEETARGET))
	{	/* pick another player as target if possible */
newtarget:
		/*if (playeringame[1] && actor->target == players[0].mo)
			actor->target = players[1].mo;
		else
			actor->target = players[0].mo;
		return false;*/

		plyrnd = 0;
		if (playeringame[1])
		{
			plyrnd = P_Random() & 1;
			if (players[plyrnd].health <= 0)
			{
				plyrnd = plyrnd ^ 1;
			}
		}

		actor->target = players[plyrnd].mo;
		return false;
	}

	mo = actor->target;
	if (!mo || mo->health <= 0)
		goto newtarget;

	if (actor->subsector->sector->soundtarget == actor->target)
		allaround = true;		/* ambush guys will turn around on a shot */

	if (!allaround)
	{
		an = R_PointToAngle2 (actor->x, actor->y, mo->x, mo->y) - actor->angle;
		if (an > ANG90 && an < ANG270)
		{
			dist = P_AproxDistance (mo->x - actor->x, mo->y - actor->y);
			/* if real close, react anyway */
			if (dist > MELEERANGE)
				return false;		/* behind back */
		}
	}

	//actor->threshold = 60;
	return true;
}


/*
===============================================================================

						ACTION ROUTINES

===============================================================================
*/

/*
==============
=
= A_Look
=
= Stay in state until a player is sighted
=
==============
*/

void A_Look (mobj_t *actor)//L8001644C()
{
	mobj_t	*targ;
	int		sound;

	/* if current target is visible, start attacking */
	if (!P_LookForPlayers(actor, false))
	{
		/* if the sector has a living soundtarget, make that the new target */
		actor->threshold = 0;		/* any shot will wake up */
		targ = actor->subsector->sector->soundtarget;
		if (targ == NULL || !(targ->flags & MF_SHOOTABLE) || (actor->flags & MF_AMBUSH))
			return;
		actor->target = targ;
	}

	/* go into chase state */
	if (actor->info->seesound)
	{
		switch (actor->info->seesound)
		{
		case sfx_posit1:
		case sfx_posit2:
		case sfx_posit3:
			sound = sfx_posit1+(P_Random()&1);
			break;
		case sfx_bgsit1:
		case sfx_bgsit2:
			sound = sfx_bgsit1+(P_Random()&1);
			break;
		default:
			sound = actor->info->seesound;
			break;
		}

		if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
			S_StartSound(NULL, sound);	// full volume
		else
			S_StartSound(actor, sound);
	}

	P_SetMobjState (actor, actor->info->seestate);
}


/*
==============
=
= A_Chase
=
= Actor has a melee attack, so it tries to close as fast as possible
=
==============
*/

void A_Chase (mobj_t *actor)//L80016578()
{
	int		delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	/* */
	/* modify target threshold */
	/* */
	if  (actor->threshold)
		actor->threshold--;

	/* */
	/* turn towards movement direction if not there yet */
	/* */
	if (actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);
		if (delta > 0)
			actor->angle -= ANG90/2;
		else if (delta < 0)
			actor->angle += ANG90/2;
	}

	if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{	/* look for a new target */
		if (P_LookForPlayers(actor,true))
			return;		/* got a new target */
		P_SetMobjState (actor, actor->info->spawnstate);
		return;
	}

	/* */
	/* don't attack twice in a row */
	/* */
	if (actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		P_NewChaseDir (actor);
		return;
	}

	/* */
	/* check for melee attack */
	/* */
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if (actor->info->attacksound)
			S_StartSound (actor, actor->info->attacksound);
		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}

	/* */
	/* check for missile attack */
	/* */
	if ( (gameskill == sk_nightmare || !actor->movecount) && actor->info->missilestate
	&& P_CheckMissileRange (actor))
	{
		P_SetMobjState (actor, actor->info->missilestate);
		if (gameskill != sk_nightmare)
			actor->flags |= MF_JUSTATTACKED;
		return;
	}

	/* */
	/* chase towards player */
	/* */
	if (--actor->movecount<0 || !P_Move (actor))
		P_NewChaseDir (actor);

	/* */
	/* make active sound */
	/* */
	if (actor->info->activesound && P_Random () < 3)
		S_StartSound (actor, actor->info->activesound);
}

/*============================================================================= */

/*
==============
=
= A_FaceTarget
=
==============
*/
//inline
void A_FaceTarget (mobj_t *actor)//L800168C0()
{
	if (!actor->target)
		return;

	actor->flags &= ~MF_AMBUSH;
	actor->angle = R_PointToAngle2 (actor->x, actor->y , actor->target->x, actor->target->y);

	if (actor->target->flags & (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3))
		actor->angle += (P_Random() - P_Random()) << 21;
}


/*
==============
=
= A_PosAttack
=
==============
*/

void A_PosAttack (mobj_t *actor)//L80016964()
{
	int		angle, damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	angle = actor->angle;

	S_StartSound (actor, sfx_pistol);
	angle += (P_Random()-P_Random())<<20;
	damage = ((P_Random()&7)+1)*3;
	P_LineAttack (actor, angle, MISSILERANGE, MAXINT, damage);
}

/*
==============
=
= A_SPosAttack
=
==============
*/

void A_SPosAttack (mobj_t *actor)//L80016A6C()
{
	int		i;
	int		angle, bangle, damage;

	if (!actor->target)
		return;

	S_StartSound (actor, sfx_shotgn);
	A_FaceTarget (actor);
	bangle = actor->angle;

	for (i=0 ; i<3 ; i++)
	{
		angle = bangle + ((P_Random()-P_Random())<<20);
		damage = ((P_Random() % 5) + 1) * 3;
		P_LineAttack (actor, angle, MISSILERANGE, MAXINT, damage);
	}
}

/*
==============
=
= A_CPosAttack
=
==============
*/

void A_CPosAttack(mobj_t* actor)//L80016BBC()
{
	int		angle;
	int		bangle;
	int		damage;
	int		slope;

	if (!actor->target)
		return;

	S_StartSound(actor, sfx_pistol);
	A_FaceTarget(actor);
	bangle = actor->angle;

	slope = P_AimLineAttack(actor, bangle, MISSILERANGE);

	angle = bangle + ((P_Random() - P_Random()) << 20);
	damage = ((P_Random() % 5) + 1) * 3;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

/*
==============
=
= A_CPosRefire
=
==============
*/

void A_CPosRefire(mobj_t* actor)//L80016D08()
{
	A_FaceTarget(actor);

	if (P_Random() < 40)
		return;

	if (!actor->target || actor->target->health <= 0
		|| !P_CheckSight(actor, actor->target))
	{
		P_SetMobjState(actor, actor->info->seestate);
	}
}

/*
==============
=
= A_SpidAttack
=
==============
*/

void A_SpidAttack(mobj_t *actor)//L80016E04()
{
	int i;
	int angle, bangle, damage;

	if (!actor->target)
		return;

	S_StartSound(actor, sfx_pistol);
	A_FaceTarget(actor);
	bangle = actor->angle;

	for (i = 0; i < 3; i++)
	{
		angle = bangle + ((P_Random() - P_Random()) << 20);
		damage = ((P_Random() % 5) + 1) * 3;
		P_LineAttack(actor, angle, MISSILERANGE, MAXINT, damage);
	}
}

/*
==============
=
= A_SpidRefire
=
==============
*/

void A_SpidRefire (mobj_t *actor)//L80016F54()
{
/* keep firing unless target got out of sight */
	A_FaceTarget (actor);
	if (P_Random () < 10)
		return;
	if (!actor->target || actor->target->health <= 0 || !(actor->flags&MF_SEETARGET) )
		P_SetMobjState (actor, actor->info->seestate);
}

/*
==============
=
= A_BspiAttack
=
==============
*/

void A_BspiAttack(mobj_t *actor)//L80017054()
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);

	// launch a missile
	P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);
}


/*
==============
=
= A_TroopAttack
=
==============
*/

void A_TroopAttack (mobj_t *actor)//L80017108()
{
	int		damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		S_StartSound (actor, sfx_claw);
		damage = ((P_Random()&7)+1)*3;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
	}
/* */
/* launch a missile */
/* */
	P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}

/*
==============
=
= A_SargAttack
=
==============
*/

void A_SargAttack (mobj_t *actor)//L80017248()
{
	int		damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	damage = ((P_Random()&7)+1)*4;
	P_LineAttack (actor, actor->angle, MELEERANGE, 0, damage);
}

/*
==============
=
= A_HeadAttack
=
==============
*/

void A_HeadAttack (mobj_t *actor)//L80017318()
{
	int		damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		damage = ((P_Random()&7)+1)*8;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
	}
/* */
/* launch a missile */
/* */
	P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}

/*
==============
=
= A_CyberAttack
=
==============
*/

void A_CyberAttack (mobj_t *actor)//L8001744C()
{
	if (!actor->target)
		return;

	A_FaceTarget (actor);
	P_SpawnMissile (actor, actor->target, MT_ROCKET);
}

/*
==============
=
= A_BruisAttack
=
==============
*/

void A_BruisAttack (mobj_t *actor)//L80017500()
{
	int		damage;

	if (!actor->target)
		return;

	if (P_CheckMeleeRange (actor))
	{
		S_StartSound (actor, sfx_claw);
		damage = ((P_Random()&7)+1)*11;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
	}
/* */
/* launch a missile */
/* */
	P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}

/*
==============
=
= A_SkelMissile
=
==============
*/

void A_SkelMissile(mobj_t *actor) //L800175C8()
{
	mobj_t*	mo;

	if (!actor->target)
		return;

	A_FaceTarget(actor);
	actor->z += 16 * FRACUNIT;	// so missile spawns higher
	mo = P_SpawnMissile(actor, actor->target, MT_TRACER);
	actor->z -= 16 * FRACUNIT;	// back to normal

	mo->x += mo->momx;
	mo->y += mo->momy;
	mo->tracer = actor->target;
}

/*
==============
=
= A_Tracer
=
==============
*/

#define	TRACEANGLE 0xC000000

void A_Tracer(mobj_t *actor) //L800176C8()
{
	angle_t	exact;
	fixed_t	dist;
	fixed_t	slope;
	mobj_t *dest;
	mobj_t *th;

	if (!(gametic & 3))
	{
		// spawn a puff of smoke behind the rocket
		P_SpawnPuff(actor->x, actor->y, actor->z);

		th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, MT_SMOKE);

		th->momz = FRACUNIT;
		th->tics -= P_Random() & 3;

		if (th->tics < 1)
			th->tics = 1;

		// adjust direction
		dest = actor->tracer;

		if (!dest || dest->health <= 0)
			return;

		// change angle
		exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

		if (exact != actor->angle)
		{
			if (exact - actor->angle > 0x80000000)
			{
				actor->angle -= TRACEANGLE;
				if (exact - actor->angle < 0x80000000)
					actor->angle = exact;
			}
			else
			{
				actor->angle += TRACEANGLE;
				if (exact - actor->angle > 0x80000000)
					actor->angle = exact;
			}
		}

		exact = actor->angle >> ANGLETOFINESHIFT;
		actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
		actor->momy = FixedMul(actor->info->speed, finesine[exact]);

		// change slope
		dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
		dist = dist / actor->info->speed;

		if (dist <= 0)
			dist = 1;

		//slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;
		slope = (dest->z - (actor->z + -(40 * FRACUNIT)))/ dist;

		if (slope < actor->momz)
			actor->momz -= FRACUNIT / 8;
		else
			actor->momz += FRACUNIT / 8;
	}
}

/*
==============
=
= A_SkelWhoosh
=
==============
*/

void A_SkelWhoosh(mobj_t* actor) //L80017918()
{
	if (!actor->target)
		return;
	A_FaceTarget(actor);
	S_StartSound(actor, sfx_skeswg);
}

/*
==============
=
= A_SkelFist
=
==============
*/

void A_SkelFist(mobj_t*	actor) //L800179C8()
{
	int	damage;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	if (P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 10) + 1) * 6;
		S_StartSound(actor, sfx_skepch);
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

/*
==============
=
= A_FatRaise
=
==============
*/

#define	FATSPREAD	(ANG90/8)

void A_FatRaise(mobj_t *actor)//L80017B28()
{
	A_FaceTarget(actor);
	S_StartSound(actor, sfx_manatk);
}

/*
==============
=
= A_FatAttack1
=
==============
*/

void A_FatAttack1(mobj_t *actor)//L80017BD8()
{
	mobj_t  *mo;
	mobj_t  *target;
	int	    an;

	A_FaceTarget(actor);

	// Change direction  to ...
	target = actor->target;
	actor->angle += FATSPREAD;
	P_SpawnMissile(actor, target, MT_FATSHOT);

	mo = P_SpawnMissile(actor, target, MT_FATSHOT);
	mo->angle += FATSPREAD;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

/*
==============
=
= A_FatAttack2
=
==============
*/

void A_FatAttack2(mobj_t *actor)//L80017D14()
{
    mobj_t  *mo;
	mobj_t  *target;
	int	    an;

	A_FaceTarget(actor);

	// Now here choose opposite deviation.
	target = actor->target;
	actor->angle -= FATSPREAD;
	P_SpawnMissile(actor, target, MT_FATSHOT);

	mo = P_SpawnMissile(actor, target, MT_FATSHOT);
	mo->angle -= FATSPREAD * 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

/*
==============
=
= A_FatAttack3
=
==============
*/

void A_FatAttack3(mobj_t *actor)//L80017E50()
{
    mobj_t  *mo;
	mobj_t  *target;
	int	    an;

	A_FaceTarget(actor);
	target = actor->target; // [9/2/2021] haleyjd: add missing assignment

	mo = P_SpawnMissile(actor, target, MT_FATSHOT);
	mo->angle -= FATSPREAD / 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);

	mo = P_SpawnMissile(actor, target, MT_FATSHOT);
	mo->angle += FATSPREAD / 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}


/*
==================
=
= SkullAttack
=
= Fly at the player like a missile
=
==================
*/

#define	SKULLSPEED		(40*FRACUNIT)

void A_SkullAttack (mobj_t *actor)//L80017FE4()
{
	mobj_t			*dest;
	angle_t			an;
	int				dist;

	if (!actor->target)
		return;

	dest = actor->target;
	actor->flags |= MF_SKULLFLY;

	S_StartSound (actor, actor->info->attacksound);
	A_FaceTarget (actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
	actor->momy = FixedMul (SKULLSPEED, finesine[an]);
	dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
	dist = dist / SKULLSPEED;
	if (dist < 1)
		dist = 1;
	actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}

/*
==============
=
= A_PainShootSkull
=
==============
*/
//inline
void A_PainShootSkull(mobj_t *actor, angle_t angle)//L80018194()
{
	fixed_t	x;
	fixed_t	y;
	fixed_t	z;

	mobj_t*	newmobj;
	angle_t	an;
	int		prestep;
	int		count;

	#if FIX_PE_SKULL_LIMIT == 1
	mobj_t	*mo;
	#else
	thinker_t*	currentthinker;
	#endif // FIX_PE_SKULL_LIMIT


	// count total number of skull currently on the level
	count = 0;
	#if FIX_PE_SKULL_LIMIT == 1
	for (mo=mobjhead.next ; mo != &mobjhead ; mo=mo->next)
	{
		if ((mo->type == MT_SKULL))
        {
            count++;
        }
	}

	#else
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		if ((currentthinker->function == P_MobjThinker) && ((mobj_t *)currentthinker)->type == MT_SKULL)
        {
			count++;
        }
		currentthinker = currentthinker->next;
	}
	#endif // FIX_PE_SKULL_LIMIT

	// if there are allready 20 skulls on the level,
	// don't spit another one
	if (count > 20)
		return;

	// okay, there's playe for another one
	an = angle >> ANGLETOFINESHIFT;

	prestep = mobjinfo[MT_SKULL].radius + (4 * FRACUNIT) + actor->info->radius;
	//r16 = *L8005E37C + 262144 + *(*(r18 + 88) + 64);
	//prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;

	x = actor->x + FixedMul(prestep, finecosine[an]);
	y = actor->y + FixedMul(prestep, finesine[an]);
	z = actor->z + 8 * FRACUNIT;

	newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

	// Check for movements.
	if (!P_TryMove(newmobj, newmobj->x, newmobj->y))
	{
		// kill it immediately
		P_DamageMobj(newmobj, actor, actor, 10000);
		return;
	}

	newmobj->target = actor->target;
	A_SkullAttack(newmobj);
}

/*
==============
=
= A_PainAttack
=
==============
*/

void A_PainAttack(mobj_t *actor)//L800182E8()
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);
	A_PainShootSkull(actor, actor->angle);
}

/*
==============
=
= A_PainDie
=
==============
*/

void A_PainDie(mobj_t *actor)//L800184B8()
{
	A_Fall(actor);
	A_PainShootSkull(actor, actor->angle + ANG90);
	A_PainShootSkull(actor, actor->angle + ANG180);
	A_PainShootSkull(actor, actor->angle + ANG270);
}

/*
==============
=
= A_Scream
=
==============
*/

void A_Scream (mobj_t *actor)//L800188BC()
{
	int		sound;

	switch (actor->info->deathsound)
	{
	case 0:
		return;

	case sfx_podth1:
	case sfx_podth2:
	case sfx_podth3:
		sound = sfx_podth1 + (P_Random ()&1);
		break;

	case sfx_bgdth1:
	case sfx_bgdth2:
		sound = sfx_bgdth1 + (P_Random ()&1);
		break;

	default:
		sound = actor->info->deathsound;
		break;
	}

	// Check for bosses.
	if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
	{
		// full volume
		S_StartSound(NULL, sound);
	}
	else
		S_StartSound(actor, sound);
}

/*
==============
=
= A_XScream
=
==============
*/

void A_XScream (mobj_t *actor)//L8001892C()
{
	S_StartSound (actor, sfx_slop);
}

/*
==============
=
= A_Pain
=
==============
*/

void A_Pain (mobj_t *actor)//L8001894C()
{
	if (actor->info->painsound)
		S_StartSound (actor, actor->info->painsound);
}

/*
==============
=
= A_Fall
=
==============
*/

void A_Fall (mobj_t *actor)//L80018984()
{
/* actor is on ground, it can be walked over */
	actor->flags &= ~MF_SOLID;
}


/*
================
=
= A_Explode
=
================
*/

void A_Explode (mobj_t *thingy)//L80018998()
{
	P_RadiusAttack(thingy, thingy->target, 128);
}


/*
================
=
= A_BossDeath
=
= Possibly trigger special effects
================
*/
extern int enemyspecial;   //80077DB4

void A_BossDeath (mobj_t *mo)//L800189BC()
{
	mobj_t		*mo2;
	line_t		junk;

	if (enemyspecial & 1)
	{
		junk.tag = 666;
		if (mo->type == MT_FATSO)
			goto RunAction;
	}
	if (enemyspecial & 2)
	{
		junk.tag = 667;
		if (mo->type == MT_BABY)
			goto RunAction;
	}
	if (enemyspecial & 4)
	{
		junk.tag = 668;
		if (mo->type == MT_SPIDER)
			goto RunAction;
	}
	if (enemyspecial & 8)
	{
		junk.tag = 669;
		if (mo->type == MT_KNIGHT)
			goto RunAction;
	}
	if (enemyspecial & 16)
	{
		junk.tag = 670;
		if (mo->type == MT_CYBORG)
			goto RunAction;
	}
	if (enemyspecial & 32)
	{
		junk.tag = 671;
		if (mo->type == MT_BRUISER)
			goto RunAction;
	}

	//No Find Enemy Special Flag
	return;

RunAction:
	/* */
	/* scan the remaining thinkers to see if all bosses are dead */
	/* */

	/* FIXME */
	for (mo2=mobjhead.next ; mo2 != &mobjhead ; mo2=mo2->next)
	{
		if ((mo2 != mo) && (mo2->type == mo->type) && (mo2->health > 0))
			return;		/* other boss not dead */
	}

	/* */
	/* victory! */
	/* */
	switch (junk.tag)// Get case number
	{
	case 666:
		enemyspecial &= ~1;
		EV_DoFloor(&junk, lowerFloorToLowest);
		break;
	case 667:
		enemyspecial &= ~2;
		EV_DoFloor(&junk, raiseFloor24);
		break;
	case 668:
		enemyspecial &= ~4;
		EV_DoFloor(&junk, lowerFloorToLowest);
		break;
	case 669:
		enemyspecial &= ~8;
		EV_DoFloor(&junk, lowerFloorToLowest);
		break;
	case 670:
		enemyspecial &= ~16;
		EV_DoDoor(&junk, Open);
		break;
	case 671:
		enemyspecial &= ~32;
		EV_DoFloor(&junk, lowerFloorToLowest);
		break;
	default:
		break;
	}
}

/*
================
=
= A_Hoof
=
================
*/

void A_Hoof (mobj_t *mo)//L80018BDC()
{
	S_StartSound(mo, sfx_hoof);
	A_Chase(mo);
}

/*
================
=
= A_Metal
=
================
*/

void A_Metal (mobj_t *mo)//L80018C10()
{
	S_StartSound(mo, sfx_metal);
	A_Chase(mo);
}

/*
================
=
= A_BabyMetal
=
================
*/

void A_BabyMetal(mobj_t* mo)//L80018C44()
{
	S_StartSound(mo, sfx_bspwlk);
	A_Chase(mo);
}

/*============================================================================= */

/* a move in p_base.c crossed a special line */
#if 0
void L_CrossSpecial (mobj_t *mo)
{
	line_t	*line;

	line = (line_t *)(mo->extradata & ~1);

	P_CrossSpecialLine (line, mo);
}
#endif

/*
================
=
= L_MissileHit
=
================
*/

/* a move in p_base.c caused a missile to hit another thing or wall */
void L_MissileHit (mobj_t *mo)//L80018C78()
{
	int	damage;
	mobj_t	*missilething;

	missilething = mo->extramobj;
	if (missilething)
	{
		damage = ((P_Random()&7)+1)*mo->info->damage;
		P_DamageMobj (missilething, mo, mo->target, damage);
	}
	P_ExplodeMissile (mo);
}

/*
================
=
= L_SkullBash
=
================
*/

/* a move in p_base.c caused a flying skull to hit another thing or a wall */
void L_SkullBash (mobj_t *mo)//L80018CEC()
{
	int	damage;
	mobj_t	*skullthing;

	skullthing = mo->extramobj;

	if (skullthing)
	{
		damage = ((P_Random()&7)+1)*mo->info->damage;
		P_DamageMobj (skullthing, mo, mo, damage);
	}

	mo->flags &= ~MF_SKULLFLY;
	mo->momx = mo->momy = mo->momz = 0;
	P_SetMobjState (mo, mo->info->spawnstate);
}
