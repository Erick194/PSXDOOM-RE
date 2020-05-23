/* P_map.c */

#include "doomdef.h"
#include "p_local.h"

//int DSPRead (void volatile *adr);

/*============================================================================= */

/*================== */
/* */
/* in */
/* */
/*================== */

mobj_t  *tmthing;		//80077EB8  ||pmGp00000aa8
fixed_t  tmx, tmy;		//80077F78 , 80077F7C       ||fGp00000b68, fGp00000b6c
boolean  checkposonly;	//80077F10  || Gp00000b00

/*================== */
/* */
/* out */
/* */
/*================== */
extern	boolean		trymove2;           //80077f64

extern	boolean		floatok;				/* if true, move would be ok if */
											/* within tmfloorz - tmceilingz */

extern	fixed_t		tmfloorz, tmceilingz, tmdropoffz;

extern	mobj_t	*movething;             //800780E8

/*============================================================================= */

/*
===================
=
= P_TryMove
=
in:
tmthing		a mobj_t (can be valid or invalid)
tmx,tmy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

newsubsec
floatok			if true, move would be ok if within tmfloorz - tmceilingz
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)

movething

==================
*/

void P_TryMove2 (void);

//int checkpostics;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y)//L8001B5D8()
{
    checkposonly = true;

	tmthing = thing;
	tmx = x;
	tmy = y;

	P_TryMove2 ();

	return trymove2;
}


boolean P_TryMove (mobj_t *thing, fixed_t x, fixed_t y)//L8001B614()
{
	int		damage;
	mobj_t	*latchedmovething;

	tmthing = thing;
	tmx = x;
	tmy = y;

	P_TryMove2 ();

	/* */
	/* pick up the specials */
	/* */
	latchedmovething = movething;

	if (latchedmovething)
	{
		if (thing->flags & MF_MISSILE)
		{	/* missile bash into a monster */
			damage = ((P_Random()&7)+1)*thing->info->damage;
			P_DamageMobj (latchedmovething, thing, thing->target, damage);
		}
		else if (thing->flags & MF_SKULLFLY)
		{	/* skull bash into a monster */
			damage = ((P_Random()&7)+1)*thing->info->damage;
			P_DamageMobj (latchedmovething, thing, thing, damage);
			thing->flags &= ~MF_SKULLFLY;
			thing->momx = thing->momy = thing->momz = 0;
			P_SetMobjState (thing, thing->info->spawnstate);
		}
		else	/* pick up  */
			P_TouchSpecialThing (latchedmovething, thing);
	}

	return trymove2;
}


/*
==============================================================================

							USE LINES

==============================================================================
*/

int			usebbox[4]; //800a853c
divline_t	useline;    //800A8528

line_t		*closeline; //plGp00000c88
fixed_t		closedist;  //iGp00000cbc

/*
===============
=
= P_InterceptVector
=
= Returns the fractional intercept point along the first divline
=
===============
*/
//inline
fixed_t P_InterceptVector (divline_t *v2, divline_t *v1)//L8001B73C()
{
	fixed_t	frac, num, den;

	den = (v1->dy>>16)*(v2->dx>>16) - (v1->dx>>16)*(v2->dy>>16);

	if (den == 0)
		return -1;

	num = ((v1->x-v2->x)>>16) *(v1->dy>>16) + ((v2->y-v1->y)>>16) * (v1->dx>>16);

	frac = (num<<16) / den;

	return frac;
}


/*
================
=
= PIT_UseLines
=
================
*/

boolean	PIT_UseLines (line_t *li)//L8001B7E0()
{
	divline_t	dl;
	fixed_t		frac;

	/* */
	/* check bounding box first */
	/* */
	if (usebbox[BOXRIGHT] <= li->bbox[BOXLEFT]
	||	usebbox[BOXLEFT] >= li->bbox[BOXRIGHT]
	||	usebbox[BOXTOP] <= li->bbox[BOXBOTTOM]
	||	usebbox[BOXBOTTOM] >= li->bbox[BOXTOP] )
		return true;

	/* */
	/* find distance along usetrace */
	/* */
	P_MakeDivline (li, &dl);
	frac = P_InterceptVector (&useline, &dl);
	if (frac < 0)
		return true;		/* behind source */
	if (frac > closedist)
		return true;		/* too far away */

	/* */
	/* the line is actually hit, find the distance  */
	/* */
	if (!li->special)
	{
		P_LineOpening (li);
		if (openrange > 0)
			return true;	/* keep going */
	}

	closeline = li;
	closedist = frac;

	return true;			/* can't use for than one special line in a row */
}


/*
================
=
= P_UseLines
=
= Looks for special lines in front of the player to activate
================
*/

void P_UseLines (player_t *player) //L8001B98C()
{
	int			angle;
	fixed_t		x1, y1, x2, y2;
	int			x,y, xl, xh, yl, yh;

	angle = player->mo->angle >> ANGLETOFINESHIFT;
	x1 = player->mo->x;
	y1 = player->mo->y;
	x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
	y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

	useline.x = x1;
	useline.y = y1;
	useline.dx = x2-x1;
	useline.dy = y2-y1;

	if (useline.dx > 0)
	{
		usebbox[BOXRIGHT ] = x2;
		usebbox[BOXLEFT  ] = x1;
	}
	else
	{
		usebbox[BOXRIGHT ] = x1;
		usebbox[BOXLEFT  ] = x2;
	}

	if (useline.dy > 0)
	{
		usebbox[BOXTOP   ] = y2;
		usebbox[BOXBOTTOM] = y1;
	}
	else
	{
		usebbox[BOXTOP   ] = y1;
		usebbox[BOXBOTTOM] = y2;
	}

	yh = (usebbox[BOXTOP   ] - bmaporgy) >> MAPBLOCKSHIFT;
	yl = (usebbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	xh = (usebbox[BOXRIGHT ] - bmaporgx) >> MAPBLOCKSHIFT;
	xl = (usebbox[BOXLEFT  ] - bmaporgx) >> MAPBLOCKSHIFT;

	// Clamp these coords to the valid range of the blockmap to avoid potential undefined behavior near map edges
	#if RANGE_CHECKS == 1
    if (xl<0)
        xl = 0;
    if (yl<0)
        yl = 0;

    if (xh>= bmapwidth)
        xh = bmapwidth -1;

    if (yh>= bmapheight)
        yh = bmapheight -1;
    #endif // RANGE_CHECKS

	closeline = NULL;
	closedist = FRACUNIT;
	validcount++;

	for (y = yl; y <= yh; y++)
	{
		for (x = xl; x <= xh; x++)
        {
			P_BlockLinesIterator(x, y, PIT_UseLines);
        }
	}

	/* */
	/* check closest line */
	/* */
	if (!closeline)
		return;

	if (!closeline->special)
		S_StartSound (player->mo, sfx_noway);
	else
		P_UseSpecialLine (player->mo, closeline);
}



/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t		*bombsource;    //pmGp0000090c
mobj_t		*bombspot;      //pmGp00000bb8
int			bombdamage;     //iGp000008b4

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

boolean PIT_RadiusAttack (mobj_t *thing)//L8001BBC8()
{
	fixed_t		dx, dy, dist;

	if (!(thing->flags & MF_SHOOTABLE))
		return true;

    if ((thing->type == MT_CYBORG) || (thing->type == MT_SPIDER))
		return true;

	dx = abs(thing->x - bombspot->x);
	dy = abs(thing->y - bombspot->y);

	dist = dx>dy ? dx : dy;
	dist = dist - thing->radius >> FRACBITS;

	if (dist < 0)
		dist = 0;

	if (dist >= bombdamage)
		return true;		/* out of range */

	if (P_CheckSight(thing, bombspot) != 0) // must be in direct path */
    {
		P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }

	return true;
}


/*
=================
=
= P_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage)//L8001BCBC()
{
	int			x,y, xl, xh, yl, yh;
	fixed_t		dist;

	dist = (damage/*+MAXRADIUS*/)<<FRACBITS;

	yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
	yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
	xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
	xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;

	// Clamp these coords to the valid range of the blockmap to avoid potential undefined behavior near map edges
	#if RANGE_CHECKS == 1
    if (xl<0)
        xl = 0;
    if (yl<0)
        yl = 0;

    if (xh>= bmapwidth)
        xh = bmapwidth -1;

    if (yh>= bmapheight)
        yh = bmapheight -1;
    #endif // RANGE_CHECKS

	bombspot = spot;
	bombsource = source;
	bombdamage = damage;

	for (y = yl; y <= yh; y++)
	{
		for (x = xl; x <= xh; x++)
        {
			P_BlockThingsIterator(x, y, PIT_RadiusAttack);
        }
	}
}


/*============================================================================ */

//int			sightcounts[2];

/*=================== */
/* */
/* IN */
/* */
/* A line will be traced from the middle of shooter in the direction of */
/* attackangle until either a shootable mobj is within the visible */
/* aimtopslope / aimbottomslope range, or a solid wall blocks further */
/* tracing.  If no thing is targeted along the entire range, the first line */
/* that blocks the midpoint of the trace will be hit. */
/*=================== */

mobj_t		*shooter;       //pmGp00000acc
angle_t		attackangle;    //aGp0000099c
fixed_t		attackrange;    //fGp000009b4
fixed_t		aimtopslope;    //uGp00000a14
fixed_t		aimbottomslope; //uGp00000d0c

/*=================== */
/* */
/* OUT */
/* */
/*=================== */

extern	line_t		*shootline;                 //800780f4
extern	mobj_t		*shootmobj;                 //800780f8
extern	fixed_t		shootslope;					/* between aimtop and aimbottom */  //80077d78
extern	fixed_t		shootx, shooty, shootz;		/* location for puff/blood */       //80077DF0,80077DFC,80077e00

mobj_t	*linetarget;			/* shootmobj latched in main memory */  //iGp00000904   80077d14

//int		shoottics;

/*
=================
=
= P_AimLineAttack
=
=================
*/

fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, fixed_t distance)//L8001BD9C()
{
    aimtopslope = 100*FRACUNIT/160;	/* can't shoot outside view angles */
    aimbottomslope = -100*FRACUNIT/160;

    validcount++;

    attackangle = angle;
    attackrange = distance;
    shooter = t1;

	P_Shoot2 ();

	linetarget = shootmobj;

	if (shootmobj)
		return shootslope;
	return 0;
}


/*
=================
=
= P_LineAttack
=
= If slope == MAXINT, use screen bounds for attacking
=
=================
*/

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage)//L8001BE10()
{
	line_t	*shootline2;
	int		shootx2, shooty2, shootz2;

    if (slope == MAXINT)
    {
        aimtopslope = 100*FRACUNIT/160;	/* can't shoot outside view angles */
		aimbottomslope = -100*FRACUNIT/160;
    }
    else
    {
        aimtopslope = slope+1;
        aimbottomslope = slope-1;
    }

    validcount++;

    attackangle = angle;
    attackrange = distance;
    shooter = t1;

	P_Shoot2 ();

	shootline2 = shootline;
	shootx2 = shootx;
	shooty2 = shooty;
	shootz2 = shootz;
	linetarget = shootmobj;

	/* */
	/* shoot thing */
	/* */
	if (linetarget)
	{
		if (linetarget->flags & MF_NOBLOOD)
			P_SpawnPuff (shootx2,shooty2,shootz2);
		else
			P_SpawnBlood (shootx2,shooty2,shootz2, damage);

		P_DamageMobj (linetarget, t1, t1, damage);
		return;
	}

	/* */
	/* shoot wall */
	/* */
	if (shootline2)
	{
		if (shootline2->special)
			P_ShootSpecialLine (t1, shootline2);

		if (shootline2->frontsector->ceilingpic == -1)
		{
			if (shootz2 > shootline2->frontsector->ceilingheight)
				return;		/* don't shoot the sky! */
			if (shootline2->backsector && shootline2->backsector->ceilingpic == -1)
				return;		/* it's a sky hack wall */
		}

		P_SpawnPuff (shootx2,shooty2,shootz2);
	}
}



