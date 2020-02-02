#include "doomdef.h"
#include "p_local.h"

/*================== */
/* */
/* out */
/* */
/*================== */

extern	mobj_t  *tmthing;		//80077EB8
extern	fixed_t  tmx, tmy;		//80077F78 , 80077F7C
extern	boolean  checkposonly;	//80077F10

/*================== */
/* */
/* in */
/* */
/*================== */

boolean trymove2;		//80077F64|iGp00000b54	/* Result from P_TryMove2 */
boolean floatok;		//80077EA8|uGp00000a98	/* if true, move would be ok if within tmfloorz - tmceilingz */
fixed_t tmfloorz;		//80078010|fGp00000c00	/* Current floor z for P_TryMove2 */
fixed_t tmceilingz;		//80077D30|fGp00000920	/* Current ceiling z for P_TryMove2 */
mobj_t *movething;		//800780E8|uGp00000cd8  /* Either a skull/missile target or a special pickup */
line_t *blockline;		//8007806C|uGp00000c5c	/* Might be a door that can be opened */

fixed_t		oldx, oldy;	//80078064, 80078068    ||fGp00000c54,fGp00000c58
fixed_t		tmbbox[4];  //800979f0
int			tmflags;	//80077EA4|uGp00000a94
fixed_t		tmdropoffz;	//80077D68|fGp00000958  /* Lowest point contacted */
subsector_t	*newsubsec;	//800780E0|psGp00000cd0	/* Dest subsector */

//PSX NEW
line_t *thingspec[8];		//800A8d0C
int		numthingspec;		//80077EE8|iGp00000ad8

/*
===================
=
= P_TryMove2
=
= Attempt to move to a new position, crossing special lines unless MF_TELEPORT
= is set
=
===================
*/

void P_TryMove2(void)//L8001E48C()
{
	int		side;
	int		oldside;
	line_t	*line;

	trymove2 = false;		// until proven otherwise
	floatok = false;

	oldx = tmthing->x;
	oldy = tmthing->y;

	PM_CheckPosition();

	if (checkposonly)
    {
		checkposonly = false;
		return;
	}

	if (!trymove2)
		return;

	if (!(tmthing->flags & MF_NOCLIP))
    {
		trymove2 = false;

		if (tmceilingz - tmfloorz < tmthing->height)
			return;			// doesn't fit
		floatok = true;
		if ( !(tmthing->flags&MF_TELEPORT) &&tmceilingz - tmthing->z < tmthing->height)
			return;			// mobj must lower itself to fit
		if ( !(tmthing->flags&MF_TELEPORT) && tmfloorz - tmthing->z > 24*FRACUNIT )
			return;			// too big a step up
		if ( !(tmthing->flags&(MF_DROPOFF|MF_FLOAT)) && tmfloorz - tmdropoffz > 24*FRACUNIT )
			return;			// don't stand over a dropoff
	}

	//
	// the move is ok, so link the thing into its new position
	//
	PM_UnsetThingPosition(tmthing);

	tmthing->floorz = tmfloorz;
	tmthing->ceilingz = tmceilingz;
	tmthing->x = tmx;
	tmthing->y = tmy;

	PM_SetThingPosition(tmthing);

	if (!tmthing->player && !(tmthing->flags & (MF_NOCLIP | MF_TELEPORT)))
	{
		while (numthingspec > 0)
		{
			numthingspec--;
			line = thingspec[numthingspec];

			side = P_PointOnLineSide(tmthing->x, tmthing->y, line);
			oldside = P_PointOnLineSide(oldx, oldy, line);

			if (side != oldside)
				P_CrossSpecialLine(line, tmthing);
		}
	}

	trymove2 = true;

	return;
}

#if 0
static boolean PM_CrossCheck(line_t *ld)
{
	if (PM_BoxCrossLine (ld))	{
		if (!PIT_CheckLine(ld)) {
			return true;
		}
	}
	return false;
}
#endif

/*
==================
=
= PM_PointOnLineSide
= Exclusive Psx Doom
=
= Returns 0 or 1
=
==================
*/
//inline
int PM_PointOnLineSide(fixed_t x, fixed_t y, line_t *line)//L8001E6B8()
{
	fixed_t dx, dy;
	fixed_t left, right;

	dx = (x - line->v1->x);
	dy = (y - line->v1->y);

	left  = (line->dy >> 16) * (dx >> 16);
	right = (dy >> 16) *(line->dx >> 16);

	if (right < left)
		return 0; /* front side */
	return 1;    /* back side */
}

/*
=================
=
= PM_UnsetThingPosition
= Exclusive Psx Doom
=
= Unlinks a thing from block map and sectors
=
=================
*/

void PM_UnsetThingPosition(mobj_t *thing)//L8001E704()
{
	int blockx, blocky;

	/* inert things don't need to be in blockmap */
	/* unlink from subsector */
	if (thing->snext)
		thing->snext->sprev = thing->sprev;
	if (thing->sprev)
		thing->sprev->snext = thing->snext;
	else
		thing->subsector->sector->thinglist = thing->snext;

	if (!(thing->flags & MF_NOBLOCKMAP))
	{
		/* inert things don't need to be in blockmap */
		/* unlink from block map */
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
=================
=
= PM_SetThingPosition
= Exclusive Psx Doom
=
= Links a thing into both a block and a subsector based on it's x y
= Sets thing->subsector properly
=
=================
*/

void PM_SetThingPosition(mobj_t *thing)//L8001E800()
{
	sector_t     *sec;
	int           blockx, blocky;
	mobj_t      **link;

	/* */
	/* link into subsector */
	/* */
	thing->subsector = newsubsec;
	if (!(thing->flags & MF_NOSECTOR))
	{
		/* invisible things don't go into the sector links */
		sec = newsubsec->sector;

		thing->sprev = NULL;
		thing->snext = sec->thinglist;
		if (sec->thinglist)
			sec->thinglist->sprev = thing;
		sec->thinglist = thing;
	}

	/* */
	/* link into blockmap */
	/* */
	if (!(thing->flags & MF_NOBLOCKMAP))
	{
		/* inert things don't need to be in blockmap */
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
			/* thing is off the map */
			thing->bnext = thing->bprev = NULL;
		}
	}
}

/*
==================
=
= PM_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
tmthing		a mobj_t (can be valid or invalid)
tmx,tmy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

newsubsec
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)
movething

==================
*/

void PM_CheckPosition (void) //L8001E910()
{
	int			xl,xh,yl,yh,bx,by;

	tmflags = tmthing->flags;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	newsubsec = R_PointInSubsector(tmx,tmy);

	//
	// the base floor / ceiling is from the subsector that contains the
	// point.  Any contacted lines the step closer together will adjust them
	//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;

	++validcount;

	numthingspec = 0;//PSX
	movething = 0;
	blockline = 0;

	if ( tmflags & MF_NOCLIP )
	{
		trymove2 = true;
		return;
	}

	//
	// check things first, possibly picking things up
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>= bmapwidth)
		xh = bmapwidth -1;
	if (yh>= bmapheight)
		yh = bmapheight -1;

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			if (!PM_BlockThingsIterator(bx, by))
			{
				trymove2 = false;
				return;
			}
		}
	}

	//
	// check lines
	//
	xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>= bmapwidth)
		xh = bmapwidth -1;
	if (yh>= bmapheight)
		yh = bmapheight -1;

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			if (!PM_BlockLinesIterator(bx, by))
			{
				trymove2 = false;
				return;
			}
		}
	}

	trymove2 = true;
	return;
}

//=============================================================================


/*
=================
=
= PM_BoxCrossLine
=
=================
*/
//inlne
boolean PM_BoxCrossLine (line_t *ld)//L8001EC00()
{
	fixed_t		x1, y1, x2, y2;
	fixed_t		lx, ly;
	fixed_t		ldx, ldy;
	fixed_t		dx1,dy1;
	fixed_t		dx2,dy2;
	boolean		side1, side2;

	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	||	tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	||	tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	||	tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
		return false;

	y1 = tmbbox[BOXTOP];
	y2 = tmbbox[BOXBOTTOM];

	if (ld->slopetype == ST_POSITIVE)
    {
		x1 = tmbbox[BOXLEFT];
		x2 = tmbbox[BOXRIGHT];
	} else
	{
		x1 = tmbbox[BOXRIGHT];
		x2 = tmbbox[BOXLEFT];
	}

	lx = ld->v1->x;
	ly = ld->v1->y;
	ldx = ld->dx >> FRACBITS;	//(ld->v2->x - lx) >> FRACBITS;
	ldy = ld->dy >> FRACBITS;	//(ld->v2->y - ly) >> FRACBITS;

	dx1 = (x1 - lx) >> FRACBITS;
	dy1 = (y1 - ly) >> FRACBITS;
	dx2 = (x2 - lx) >> FRACBITS;
	dy2 = (y2 - ly) >> FRACBITS;

	side1 = ldy*dx1 < dy1*ldx;
	side2 = ldy*dx2 < dy2*ldx;

	return (side1 != side2);
}

//=============================================================================


/*
==================
=
= PIT_CheckLine
=
= Adjusts tmfloorz and tmceilingz as lines are contacted
==================
*/

boolean PIT_CheckLine (line_t *ld)//L8001ED0C()
{
	fixed_t		pm_opentop, pm_openbottom;
	fixed_t		pm_lowfloor;
	sector_t	*front, *back;

	// a line has been hit

	/*
	=
	= The moving thing's destination position will cross the given line.
	= If this should not be allowed, return false.
	*/
	if (!ld->backsector)
		return false;		// one sided line

	if (!(tmthing->flags & MF_MISSILE) )
	{
		if ( ld->flags & ML_BLOCKING )
			return false;		// explicitly blocking everything
		if ( !tmthing->player && ld->flags & ML_BLOCKMONSTERS )
			return false;		// block monsters only
	}

	front = ld->frontsector;
	back = ld->backsector;

	if (front->ceilingheight == front->floorheight
	|| back->ceilingheight == back->floorheight)
	{
		blockline = ld;
		return false;			// probably a closed door
	}

	if (front->ceilingheight < back->ceilingheight)
		pm_opentop = front->ceilingheight;
	else
		pm_opentop = back->ceilingheight;

	if (front->floorheight > back->floorheight)
	{
		pm_openbottom = front->floorheight;
		pm_lowfloor = back->floorheight;
	}
	else
	{
		pm_openbottom = back->floorheight;
		pm_lowfloor = front->floorheight;
	}

	// adjust floor / ceiling heights
	if (pm_opentop < tmceilingz)
		tmceilingz = pm_opentop;
	if (pm_openbottom > tmfloorz)
		tmfloorz = pm_openbottom;
	if (pm_lowfloor < tmdropoffz)
		tmdropoffz = pm_lowfloor;

	if (ld->special != 0)
	{
	    //New Psx Doom
		if (numthingspec < MAXTHINGSPEC)
		{
			thingspec[numthingspec] = ld;
			numthingspec++;
		}
	}

	return true;
}

/*
==================
=
= PIT_CheckThing
=
==================
*/

boolean PIT_CheckThing(mobj_t *thing)//L8001EE5C()
{
	fixed_t blockdist;
	int			delta;

	if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
		return true;
	blockdist = thing->radius + tmthing->radius;

	delta = thing->x - tmx;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return true;		// didn't hit it

	delta = thing->y - tmy;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return true;		// didn't hit it

	if (thing == tmthing)
		return true;		// don't clip against self

	//
	// check for skulls slamming into things
	//
	if (tmthing->flags & MF_SKULLFLY)
	{
		movething = thing;
		return false;		// stop moving
	}

	//
	// missiles can hit other things
	//
	if (tmthing->flags & MF_MISSILE)
	{
		// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return true;		// overhead
		if (tmthing->z+tmthing->height < thing->z)
			return true;		// underneath
		if (tmthing->target->type == thing->type) // don't hit same species as originator
		{
			if (thing == tmthing->target)
				return true;
			if (thing->type != MT_PLAYER/*&mobjinfo[MT_PLAYER]*/) // let players missile other players
				return false;	// explode, but do no damage
		}
		if (! (thing->flags & MF_SHOOTABLE) )
			return !(thing->flags & MF_SOLID);		// didn't do any damage

		// damage / explode
		movething = thing;
		return false;			// don't traverse any more
	}

	//
	// check for special pickup
	//
	if ((thing->flags&MF_SPECIAL) && (tmflags&MF_PICKUP) )
	{
		movething = thing;
		return true;
	}

	return !(thing->flags & MF_SOLID);
}

/*
===============================================================================

BLOCK MAP ITERATORS

For each line/thing in the given mapblock, call the passed function.
If the function returns false, exit with false without checking anything else.

===============================================================================
*/

/*
==================
=
= PM_BlockLinesIterator
= Exclusive Psx Doom
=
= The validcount flags are used to avoid checking lines
= that are marked in multiple mapblocks, so increment validcount before
= the first call to PM_BlockLinesIterator, then make one or more calls to it
=
==================
*/

boolean PM_BlockLinesIterator(int x, int y)//L8001EFC0()
{
	int     offset;
	short  *list;
	line_t *ld;

	offset = (y*bmapwidth)+x;
	offset = *(blockmap + offset);

	for (list = blockmaplump + offset; *list != -1; list++)
	{
		ld = &lines[*list];
		if (ld->validcount == validcount)
			continue; /* line has already been checked */
		ld->validcount = validcount;

		if (PM_BoxCrossLine(ld))
		{
			if (!PIT_CheckLine(ld))
				return false;
		}
	}

	return true; /* everything was checked */
}

/*
==================
=
= PM_BlockThingsIterator
= Exclusive Psx Doom
=
==================
*/

boolean PM_BlockThingsIterator(int x, int y)//L8001F1A0()
{
	mobj_t *mobj;

	for (mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext)
	{
		if (!PIT_CheckThing(mobj))
			return false;
	}

	return true;
}
