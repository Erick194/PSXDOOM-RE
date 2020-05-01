/* p_base.c */

#include "doomdef.h"
#include "p_local.h"

//completo y revisado

static mobj_t *checkthing;		/* Used for PB_CheckThing */    //pmGp00000c60
static fixed_t testx, testy;                                    //fGp00000910, fGp0000091c
static fixed_t testfloorz, testceilingz, testdropoffz;          //iGp00000984, iGp00000b1c, iGp00000b38
static subsector_t *testsubsec;                                 //psGp00000944
static line_t *ceilingline;                                     //iGp000009a8
static mobj_t *hitthing;                                        //iGp00000b9c
static fixed_t testbbox[4];		/* Bounding box for tests */    //800a8e48
static int testflags;                                           //uGp00000cc8

//psx doom new
extern int checkcount; //*L80077DE4

void P_XYMovement(mobj_t *mo);
void P_FloatChange(mobj_t *mo);
void P_ZMovement(mobj_t *mo);
void P_MobjThinker(mobj_t *mobj);
boolean PB_TryMove(int tryx, int tryy);
void PB_UnsetThingPosition(mobj_t *thing);
void PB_SetThingPosition(mobj_t *thing);
boolean PB_CheckPosition(void);
boolean PB_BoxCrossLine(line_t *ld);
boolean PB_CheckLine(line_t *ld);
boolean PB_CheckThing(mobj_t *thing);
boolean PB_BlockLinesIterator(int x, int y);
boolean PB_BlockThingsIterator(int x, int y);

/*
=================
=
= P_RunMobjBase
=
= Execute base think logic for the critters every tic
=
=================
*/

void P_RunMobjBase(void)//L80013814()
{
	checkcount = 0;

	for (checkthing = mobjhead.next; checkthing != &mobjhead; checkthing = checkthing->next)
	{
		if (!checkthing->player)
		{
			checkthing->latecall = NULL;
			checkcount++;
			P_MobjThinker(checkthing);
		}
	}
}

/*
===================
=
= P_XYMovement
=
===================
*/

#define	STOPSPEED		0x1000
#define	FRICTION		0xd200  //Jag 0xd240

void P_XYMovement(mobj_t *mo)//L800138AC()
{
	fixed_t xleft, yleft;
	fixed_t xuse, yuse;

	//
	// cut the move into chunks if too large
	//

	xleft = xuse = mo->momx & ~7;
	yleft = yuse = mo->momy & ~7;

	while (xuse > MAXMOVE || xuse < -MAXMOVE
		|| yuse > MAXMOVE || yuse < -MAXMOVE)
	{
		xuse >>= 1;
		yuse >>= 1;
	}

	while (xleft || yleft)
	{
		xleft -= xuse;
		yleft -= yuse;
		if (!PB_TryMove(mo->x + xuse, mo->y + yuse))
		{
			// blocked move
			if (mo->flags & MF_SKULLFLY)
			{
				mo->latecall = L_SkullBash;
				mo->extramobj = hitthing;
			}

			// explode a missile
			if (mo->flags & MF_MISSILE)
			{
				if (ceilingline && ceilingline->backsector &&
					ceilingline->backsector->ceilingpic == -1)// hack to prevent missiles exploding against the sky
				{
					mo->latecall = P_RemoveMobj;
					return;
				}

				mo->latecall = L_MissileHit;
				mo->extramobj = hitthing;
				return;
			}

			mo->momx = mo->momy = 0;
			return;
		}
	}

	//
	// slow down
	//
	if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
		return;		// no friction for missiles ever

	if (mo->z > mo->floorz)
		return;		// no friction when airborne

	if ((mo->flags & MF_CORPSE) && (mo->floorz != mo->subsector->sector->floorheight))
        return;		// don't stop halfway off a step

	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
        mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
    {
		mo->momx = 0;
		mo->momy = 0;
	}
	else
    {
		mo->momx = (mo->momx >> 8) * (FRICTION >> 8);
		mo->momy = (mo->momy >> 8) * (FRICTION >> 8);
	}
}

/*
===================
=
= P_FloatChange
=
= Float up or down at a set speed, used by flying monsters
=
===================
*/
//inline
void P_FloatChange(mobj_t *mo)//L80013B0C()
{
	mobj_t *target;
	fixed_t dist, delta;
	fixed_t dx, dy;

	target = mo->target;		/* Get the target object */

	//inline function ??
	//dist = P_AproxDistance(target->x - mo->x, target->y - mo->y);	/* Distance to target */
	dx = abs(target->x - mo->x);
	dy = abs(target->y - mo->y);
	dist = dx+dy;
	if (dx < dy)
		dist -=(dx>>1);
	else
        dist -=(dy>>1);

    delta = (target->z + (mo->height >> 1)) - mo->z;	/* Get the height differance */
	delta *= 3; /* Mul by 3 for a fudge factor */

	if (delta<0)		                /* Delta is signed... */
    {
		if (dist < (-delta))	        /* Negate */
        {
			mo->z -= FLOATSPEED;        /* Adjust the height */
		}
	}
	else if (delta>0)		            /* Delta is signed... */
    {
        if (dist < delta)		        /* Normal compare */
        {
            mo->z += FLOATSPEED;	    /* Adjust the height */
        }
	}
}

/*
===================
=
= P_ZMovement
=
= Move a critter in the Z axis
=
===================
*/

void P_ZMovement(mobj_t *mo)//L80013BD4()
{

	mo->z += mo->momz;		/* Basic z motion */

	if ((mo->flags & MF_FLOAT) && mo->target)   /* float down towards target if too close */
    {
		P_FloatChange(mo);
	}

	//
	// clip movement
	//
	if (mo->z <= mo->floorz)	// hit the floor
	{
		if (mo->momz < 0)
			mo->momz = 0;

		mo->z = mo->floorz;
		if (mo->flags & MF_MISSILE)
		{
			mo->latecall = P_ExplodeMissile;
			return;
		}
	}
	else if (!(mo->flags & MF_NOGRAVITY))
	{
		// apply gravity
		if (mo->momz == 0)
			mo->momz = -GRAVITY;
		else
			mo->momz -= (GRAVITY/2);
	}

	if (mo->z + mo->height > mo->ceilingz) // hit the ceiling
	{
		if (mo->momz > 0)
			mo->momz = 0;

		mo->z = mo->ceilingz - mo->height;
		if (mo->flags & MF_MISSILE)
			mo->latecall = P_ExplodeMissile;
	}
}

/*
===================
=
= P_MobjThinker
=
= Process all the critter logic
=
===================
*/

void P_MobjThinker(mobj_t *mobj)//L80013DB4()
{
	state_t *st;
	statenum_t state;

	// momentum movement
	if (mobj->momx || mobj->momy)
    {
		P_XYMovement(mobj);

        // removed or has a special action to perform?
        if (mobj->latecall)
            return;
    }

	if (mobj->z != mobj->floorz || mobj->momz)
    {
		P_ZMovement(mobj);

        // removed or has a special action to perform?
        if (mobj->latecall)
            return;
    }

	// cycle through states
	if (mobj->tics != -1)
	{
		mobj->tics--;

		// you can cycle through multiple states in a tic
        if (mobj->tics <= 0)
		{
			state = mobj->state->nextstate;
			if (state == S_NULL)
			{
				mobj->latecall = P_RemoveMobj;
			}
			else
			{
				st = &states[state];
				mobj->state = st;
				mobj->tics = st->tics;
				mobj->sprite = st->sprite;
				mobj->frame = st->frame;
				mobj->latecall = st->action;
			}
		}
	}
}

/*
===================
=
= PB_TryMove
=
= Attempt to move to a new position
=
===================
*/

boolean PB_TryMove(int tryx, int tryy)//L80013ED4()
{
	testx = tryx;
	testy = tryy;

	if (!PB_CheckPosition())
		return false;		// solid wall or thing

	if (testceilingz - testfloorz < checkthing->height)
		return false;			// doesn't fit
	if (testceilingz - checkthing->z < checkthing->height)
		return false;			// mobj must lower itself to fit
	if (testfloorz - checkthing->z > 24 * FRACUNIT)
		return false;			// too big a step up
	if (!(testflags&(MF_DROPOFF | MF_FLOAT)) && testfloorz - testdropoffz > 24 * FRACUNIT)
		return false;			// don't stand over a dropoff

	//
	// the move is ok, so link the thing into its new position
	//

	PB_UnsetThingPosition(checkthing);
	checkthing->floorz = testfloorz;
	checkthing->ceilingz = testceilingz;
	checkthing->x = tryx;
	checkthing->y = tryy;
	PB_SetThingPosition(checkthing);
	return true;
}

/*
===================
=
= PB_UnsetThingPosition
=
===================
*/

void PB_UnsetThingPosition(mobj_t *thing)//L80013FB4()
{
	int blockx, blocky;

	// inert things don't need to be in blockmap
	// unlink from subsector
	if (thing->snext)
		thing->snext->sprev = thing->sprev;
	if (thing->sprev)
		thing->sprev->snext = thing->snext;
	else
		thing->subsector->sector->thinglist = thing->snext;

	if (!(testflags & MF_NOBLOCKMAP))
	{
		// inert things don't need to be in blockmap
		// unlink from block map
		if (thing->bnext)
			thing->bnext->bprev = thing->bprev;
		if (thing->bprev)
			thing->bprev->bnext = thing->bnext;
		else
		{
			blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
			blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;

			// Prevent buffer overflow if the map object is out of bounds.
            // This is part of the fix for the famous 'linedef deletion' bug.
            // From StationDoom by BodbDearg
			#if FIX_LINEDEFS_DELETION
            if (blockx>=0 && blockx < bmapwidth
             && blocky>=0 && blocky <bmapheight)
            {
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
            }
            #else
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
			#endif
		}
	}
}

/*
===================
=
= PB_SetThingPosition
=
===================
*/

void PB_SetThingPosition(mobj_t *thing)//L800140B0()
{
	sector_t     *sec;
	int           blockx, blocky;
	mobj_t      **link;

	//
	// link into subsector
	//

	thing->subsector = testsubsec;
	sec = thing->subsector->sector;

	thing->sprev = NULL;
	thing->snext = sec->thinglist;

	if (sec->thinglist)
		sec->thinglist->sprev = thing;

	sec->thinglist = thing;

	//
	// link into blockmap
	//
	if (!(testflags & MF_NOBLOCKMAP))
	{
		// inert things don't need to be in blockmap
		blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
		blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;
		if (blockx >= 0 && blockx < bmapwidth && blocky >= 0 && blocky < bmapheight)
		{
			link = &blocklinks[blocky*bmapwidth + blockx];
			thing->bprev = NULL;
			thing->bnext = *link;
			if (*link)
				(*link)->bprev = thing;
			*link = thing;
		}
		else
		{
			// thing is off the map
			thing->bnext = thing->bprev = NULL;
		}
	}
}

/*
==================
=
= PB_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
basething		a mobj_t
testx,testy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

testsubsec
floorz
ceilingz
testdropoffz		the lowest point contacted (monsters won't move to a dropoff)
hitthing

==================
*/

boolean PB_CheckPosition(void)//L800141B0()
{
	int			xl, xh, yl, yh, bx, by;
	fixed_t		r;

	testflags = checkthing->flags;

	r = checkthing->radius;

	testbbox[BOXTOP   ] = testy + r;
	testbbox[BOXBOTTOM] = testy - r;
	testbbox[BOXRIGHT ] = testx + r;
	testbbox[BOXLEFT  ] = testx - r;

	//
	// the base floor / ceiling is from the subsector that contains the
	// point.  Any contacted lines the step closer together will adjust them
	//
	testsubsec = R_PointInSubsector(testx, testy);
	testfloorz = testdropoffz = testsubsec->sector->floorheight;
	testceilingz = testsubsec->sector->ceilingheight;

	++validcount;

	ceilingline = NULL;
	hitthing = NULL;

	//
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = (testbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
	xh = (testbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
	yl = (testbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
	yh = (testbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

	if (xl<0)
	{
		xl = 0;
	}
	if (yl<0)
	{
		yl = 0;
	}
	if (xh >= bmapwidth)
	{
		xh = bmapwidth - 1;
	}
	if (yh >= bmapheight)
	{
		yh = bmapheight - 1;
	}

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
        {
			if (!PB_BlockLinesIterator(bx, by))
				return false;
			if (!PB_BlockThingsIterator(bx, by))
				return false;
		}
	}

	return true;
}

/*
=================
=
= PB_BoxCrossLine
=
=================
*/

//inline
boolean PB_BoxCrossLine(line_t *ld)//L80014394()
{
	fixed_t x1, x2;
	fixed_t lx, ly;
	fixed_t ldx, ldy;
	fixed_t dx1, dy1;
	fixed_t dx2, dy2;
	boolean side1, side2;

    // entirely outside bounding box of line?
    if(testbbox[BOXRIGHT ] <= ld->bbox[BOXLEFT  ] ||
       testbbox[BOXLEFT  ] >= ld->bbox[BOXRIGHT ] ||
       testbbox[BOXTOP   ] <= ld->bbox[BOXBOTTOM] ||
       testbbox[BOXBOTTOM] >= ld->bbox[BOXTOP   ])
    {
        return false;
    }

	if (ld->slopetype == ST_POSITIVE)
    {
		x1 = testbbox[BOXLEFT];
		x2 = testbbox[BOXRIGHT];
	}
	else
	{
		x1 = testbbox[BOXRIGHT];
		x2 = testbbox[BOXLEFT];
	}

	lx = ld->v1->x;
	ly = ld->v1->y;
	ldx = ld->dx >> FRACBITS;   //ldx = (ld->v2.x - ld->v1.x) >> FRACBITS;
	ldy = ld->dy >> FRACBITS;   //ldy = (ld->v2.y - ld->v1.y) >> FRACBITS;

	dx1 = (x1 - lx) >> 16;
	dy1 = (testbbox[BOXTOP] - ly) >> FRACBITS;
	dx2 = (x2 - lx) >> 16;
	dy2 = (testbbox[BOXBOTTOM] - ly) >> FRACBITS;

	side1 = ldy*dx1 < dy1*ldx;
	side2 = ldy*dx2 < dy2*ldx;

	return (side1 != side2);
}

/*
==================
=
= PB_CheckLine
=
= Adjusts testfloorz and testceilingz as lines are contacted
==================
*/

//inline
boolean PB_CheckLine(line_t *ld)//L800144AC()
{
	fixed_t opentop, openbottom;
	fixed_t lowfloor;
	sector_t	*front, *back;

	/*
	=
	= The moving thing's destination position will cross the given line.
	= If this should not be allowed, return FALSE.
	*/
	if (!ld->backsector)
		return false;		// one sided line

	if (!(testflags & MF_MISSILE) && (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS)))
		return false;		// explicitly blocking

	if ((ld->flags & ML_BLOCKPRJECTILE)) //psx doom new
		return false;

	front = ld->frontsector;
	back = ld->backsector;

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;

	if (front->floorheight > back->floorheight)
	{
		openbottom = front->floorheight;
		lowfloor = back->floorheight;
	}
	else
	{
		openbottom = back->floorheight;
		lowfloor = front->floorheight;
	}

	// adjust floor / ceiling heights
	if (opentop < testceilingz)
	{
		testceilingz = opentop;
		ceilingline = ld;
	}
	if (openbottom > testfloorz)
		testfloorz = openbottom;
	if (lowfloor < testdropoffz)
		testdropoffz = lowfloor;

	return true;
}

/*
==================
=
= PB_CheckThing
=
==================
*/

boolean PB_CheckThing(mobj_t *thing)//L80014598()
{
	fixed_t  blockdist;
	int      delta;
	mobj_t  *mo;

	if (!(thing->flags & MF_SOLID))
		return true; // not blocking

	mo = checkthing;

	blockdist = thing->radius + mo->radius;

	delta = thing->x - testx;
	if (delta < 0)
		delta = -delta;

	if (delta >= blockdist)
		return true; // didn't hit it

	delta = thing->y - testy;
	if (delta < 0)
		delta = -delta;

	if (delta >= blockdist)
		return true; // didn't hit it

	if (thing == mo)
		return true; // don't clip against self

	//
	// check for skulls slamming into things
	//
	if (testflags & MF_SKULLFLY)
	{
		hitthing = thing;
		return false;		// stop moving
	}

	//
	// missiles can hit other things
	//
	if (testflags & MF_MISSILE)
    {
		// see if it went over / under
		if (mo->z > thing->z + thing->height)
			return true;		// overhead

		if (mo->z + mo->height < thing->z)
			return true;		// underneath

		if (mo->target->type == thing->type)	// don't hit same species as originator
        {
			if (thing == mo->target)
				return true;	// don't explode on shooter

			if (thing->type != MT_PLAYER)
				return false;	// explode, but do no damage
								// let players missile other players
		}
		if (!(thing->flags & MF_SHOOTABLE))
			return !(thing->flags & MF_SOLID);		// didn't do any damage
													// damage / explode
		hitthing = thing;
		return false;			// don't traverse any more
	}

	return !(thing->flags & MF_SOLID);
}

/*
==================
=
= PB_BlockLinesIterator
=
==================
*/

boolean PB_BlockLinesIterator(int x, int y) //L800146C4()
{
	int     offset;
	short  *list;
	line_t *ld;

	offset = y*bmapwidth+x;

	offset = *(blockmap + offset);

	for (list = blockmaplump + offset; *list != -1; list++)
	{
		ld = &lines[*list];
		if (ld->validcount == validcount)
			continue; // line has already been checked
		ld->validcount = validcount;

		if (PB_BoxCrossLine(ld))
		{
			if (!PB_CheckLine(ld))
				return false;
		}
	}

	return true; // everything was checked
}

/*
==================
=
= PB_BlockThingsIterator
=
==================
*/

boolean PB_BlockThingsIterator(int x, int y)//L8001498C()
{
	mobj_t *mobj;

	for (mobj = blocklinks[y*bmapwidth+x]; mobj; mobj = mobj->bnext)
	{
		if (!PB_CheckThing(mobj))
			return false;
	}

	return true;
}
