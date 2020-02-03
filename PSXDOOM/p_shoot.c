#include "doomdef.h"
#include "p_local.h"

//===================
//
// IN
//
// A line will be shootdivd from the middle of shooter in the direction of
// attackangle until either a shootable mobj is within the visible
// aimtopslope / aimbottomslope range, or a solid wall blocks further
// tracing.  If no thing is targeted along the entire range, the first line
// that blocks the midpoint of the shootdiv will be hit.
//===================

line_t *shootline;      //iGp00000ce4
mobj_t *shootmobj;      //iGp00000ce8
fixed_t shootslope;					// between aimtop and aimbottom //iGp00000968
fixed_t shootx, shooty, shootz;		// location for puff/blood      //iGp000009e0, iGp000009ec, iGp000009f0

//===================
//
// TEMPS
//
//===================

fixed_t aimmidslope;         // for detecting first wall hit     iGp000009c8
divline_t shootdiv;          //800a8e58
fixed_t shootx2, shooty2;    //iGp00000a54,iGp00000a60
fixed_t firstlinefrac;       //iGp00000be8

int shootdivpositive;        //uGp00000a88

fixed_t old_frac;            //uGp00000b44
void	*old_value;             //pvGp00000c74
boolean old_isline;          //bGp000008e4
int ssx1,ssy1,ssx2,ssy2;     //iGp00000c08, iGp00000c18, iGp00000c14, iGp00000c24

typedef	struct
{
    vertex_t v1, v2;
} thingline_t;

static thingline_t thingline;//800A8824*/
static vertex_t *thing_line[2] = {&thingline.v1, &thingline.v2};//iGp00000534, iGp00000538

extern mobj_t  *shooter;//*(r28 + 2764)
extern angle_t  attackangle;//*(r28 + 2484)
extern fixed_t  attackrange;//*(r28 + 2460)
extern fixed_t  aimtopslope;//*(r28 + 2580)
extern fixed_t  aimbottomslope;//*(r28 + 3340)

/*
=====================
=
= P_Shoot2
=
=====================
*/

void P_Shoot2(void)//L80023BCC()
{
	mobj_t		*t1;
	unsigned	angle;

	t1 = shooter;

	shootline = 0;
	shootmobj = 0;

	angle = attackangle >> ANGLETOFINESHIFT;

	shootdiv.x = t1->x;
	shootdiv.y = t1->y;
	shootx2 = t1->x + (attackrange>>FRACBITS)*finecosine[angle];
	shooty2 = t1->y + (attackrange>>FRACBITS)*finesine[angle];
	shootdiv.dx = shootx2 - shootdiv.x;
	shootdiv.dy = shooty2 - shootdiv.y;
	shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

	shootdivpositive = (shootdiv.dx ^ shootdiv.dy)>0;

	ssx1 = shootdiv.x >> 16;
	ssy1 = shootdiv.y >> 16;
	ssx2 = shootx2 >> 16;
	ssy2 = shooty2 >> 16;

	aimmidslope = (aimtopslope + aimbottomslope)>>1;

	//
	// cross everything
	//
	old_frac = 0;

	PA_CrossBSPNode(numnodes-1);

	// check the last intercept if needed
	if (!shootmobj)
		PA_DoIntercept(0, false, FRACUNIT);

	//
	// post process
	//
	if (shootmobj)
		return;

	if (!shootline)
		return;

	//
	// calculate the intercept point for the first line hit
	//
	// position a bit closer
	firstlinefrac -= FixedDiv(4*FRACUNIT,attackrange);

	shootx  = shootdiv.x + FixedMul(shootdiv.dx, firstlinefrac);
	shooty  = shootdiv.y + FixedMul(shootdiv.dy, firstlinefrac);
	shootz += FixedMul(aimmidslope, FixedMul(firstlinefrac,attackrange));
}


/*
==================
=
= PA_DoIntercept
=
==================
*/
//inline
boolean PA_DoIntercept(void *value, boolean isline, int frac)//L80023DD4()
{
    void   *value_;
    int     frac_;
    boolean isline_;

    isline_ = isline;
    value_ = value;
    frac_ = frac;

	if (old_frac < frac)
	{
        isline_ = old_isline;
        value_ = old_value;
        frac_ = old_frac;

        old_isline = isline;
        old_frac = frac;
        old_value = value;
	}

	if (frac_ == 0 || (0xffff < frac_))//frac >= FRACUNIT)
		return true;

	if (isline_)
		return PA_ShootLine ((line_t *)value_, frac_);
    else
        return PA_ShootThing ((mobj_t *)value_, frac_);
}


/*
==============================================================================
=
= PA_ShootLine
=
==============================================================================
*/

boolean	PA_ShootLine(line_t *li, fixed_t interceptfrac)//L80023E5C()
{
	fixed_t		slope;
	fixed_t		dist;
	sector_t	*front, *back;
	static fixed_t opentop,openbottom;//80077ee4, 80077d5c

	if ( !(li->flags & ML_TWOSIDED) )
	{
		if (!shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		old_frac = 0;	// don't shoot anything past this
		return false;
	}

	//
	// crosses a two sided line
	//
	front = li->frontsector;
	back = li->backsector;

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;
	if (front->floorheight > back->floorheight)
		openbottom = front->floorheight;
	else
		openbottom = back->floorheight;

	dist = FixedMul(attackrange,interceptfrac);

	if (li->frontsector->floorheight != li->backsector->floorheight)
	{
		slope = FixedDiv(openbottom - shootz , dist);
		if (slope >= aimmidslope && !shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		if (slope > aimbottomslope)
			aimbottomslope = slope;
	}

	if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
	{
		slope = FixedDiv(opentop - shootz , dist);
		if (slope <= aimmidslope && !shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		if (slope < aimtopslope)
			aimtopslope = slope;
	}

	if (aimtopslope <= aimbottomslope)
		return false;		// stop

	return true;		// shot continues
}

/*
==============================================================================
=
= PA_ShootThing
=
==============================================================================
*/

boolean PA_ShootThing(mobj_t *th, fixed_t interceptfrac)//L80024054()
{
	fixed_t		frac;
	fixed_t		dist;
	fixed_t		thingaimtopslope, thingaimbottomslope;

	if (th == shooter)
		return true;		// can't shoot self
	if (!(th->flags&MF_SHOOTABLE))
		return true;		// corpse or something

	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, interceptfrac);

	thingaimtopslope =  FixedDiv(th->z+th->height - shootz , dist);
	if (thingaimtopslope < aimbottomslope)
		return true;		// shot over the thing

	thingaimbottomslope =  FixedDiv(th->z - shootz, dist);
	if (thingaimbottomslope > aimtopslope)
		return true;		// shot under the thing

	//
	// this thing can be hit!
	//
	if (thingaimtopslope > aimtopslope)
		thingaimtopslope = aimtopslope;
	if (thingaimbottomslope < aimbottomslope)
		thingaimbottomslope = aimbottomslope;

	// shoot midway in the visible part of the thing
	shootslope = (thingaimtopslope+thingaimbottomslope)/2;

	shootmobj = th;

	// position a bit closer
	frac = interceptfrac - FixedDiv(10*FRACUNIT,attackrange);
	shootx = shootdiv.x + FixedMul(shootdiv.dx, frac);
	shooty = shootdiv.y + FixedMul(shootdiv.dy, frac);
	shootz = shootz + FixedMul(shootslope, FixedMul(frac,attackrange));

	return false;			// don't go any farther
}

/*
=================
=
= PA_SightCrossLine
=
= First checks the endpoints of the line to make sure that they cross the
= sight trace treated as an infinite line.
=
= If so, it calculates the fractional distance along the sight trace that
= the intersection occurs at.  If 0 < intercept < 1.0, the line will block
= the sight.
=================
*/

//inline
fixed_t PA_SightCrossLine(line_t *line)//L800241F4()
{
	int	s1, s2;
	int	p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,dx,dy,ndx,ndy;

	// p1, p2 are line endpoints
	p1x = line->v1->x >> FRACBITS;
	p1y = line->v1->y >> FRACBITS;
	p2x = line->v2->x >> FRACBITS;
	p2y = line->v2->y >> FRACBITS;

	// p3, p4 are sight endpoints
	p3x = ssx1;
	p3y = ssy1;
	p4x = ssx2;
	p4y = ssy2;

	dx = p2x - p3x;
	dy = p2y - p3y;

	ndx = p4x - p3x;		// this can be precomputed if worthwhile
	ndy = p4y - p3y;

	s1 =  (ndy * dx) <  (dy * ndx);

	dx = p1x - p3x;
	dy = p1y - p3y;

	s2 =  (ndy * dx) <  (dy * ndx);

	if (s1 == s2)
		return -1;			// line isn't crossed

	ndx = p1y - p2y;		// vector normal to world line
	ndy = p2x - p1x;

	s1 = ndx*dx + ndy*dy;	// distance projected onto normal

	dx = p4x - p1x;
	dy = p4y - p1y;

	s2 = ndx*dx + ndy*dy;	// distance projected onto normal

	s2 = FixedDiv(s1,(s1+s2));

	return s2;
}



/*
=================
=
= PA_CrossSubsectorPass
=
= Returns true if strace crosses the given subsector successfuly
=================
*/

boolean PA_CrossSubsector(subsector_t *sub)//L800242CC
{
	seg_t		*seg;
	line_t		*line;
	int			count;
	fixed_t		frac;
	mobj_t		*thing;

	//
	// check things
	//
	for (thing = sub->sector->thinglist ; thing ; thing = thing->snext )
	{
		if (thing->subsector != sub)
			continue;

		// check a corner to corner crossection for hit

		if (shootdivpositive)
        {
            thingline.v1.x = thing->x - thing->radius;
            thingline.v1.y = thing->y + thing->radius;
            thingline.v2.x = thing->x + thing->radius;
            thingline.v2.y = thing->y - thing->radius;
		}
		else
		{
            thingline.v1.x = thing->x - thing->radius;
            thingline.v1.y = thing->y - thing->radius;
            thingline.v2.x = thing->x + thing->radius;
            thingline.v2.y = thing->y + thing->radius;
		}

		// inline function
		frac = PA_SightCrossLine ((line_t *)&thing_line);

		if (frac < 0 || frac > FRACUNIT) {
			continue;
		}
		// inline function
		if (!PA_DoIntercept (thing, false, frac))
        {
			return false;
		}
	}

	//
	// check lines
	//
	count = sub->numlines;
	seg = &segs[sub->firstline];

	for ( ; count ; seg++, count--)
	{
		line = seg->linedef;

		if (line->validcount == validcount)
			continue;		// already checked other side
		line->validcount = validcount;

		// inline function
		frac = PA_SightCrossLine (line);

		if (frac < 0 || frac > FRACUNIT)
			continue;

        // inline function
		if (!PA_DoIntercept (line, true, frac))
        {
			return false;
        }
	}

	return true;    // passed the subsector ok
}

/*
=====================
=
= PA_DivlineSide
=
=====================
*/
//inline
int PA_DivlineSide(fixed_t x, fixed_t y, divline_t *line)//L800246F0()
{
	fixed_t dx, dy;

	x = (x - line->x) >> FRACBITS;
	y = (y - line->y) >> FRACBITS;

	dx = x * (line->dy >> FRACBITS);
	dy = y * (line->dx >> FRACBITS);

	return dy < dx ^ 1;
}

/**********************************

Returns true if strace crosses the given node successfuly

**********************************/

boolean PA_CrossBSPNode(int bspnum)//L80024734()
{
	node_t *bsp;
	int bsp_num, side;

    if (bspnum & NF_SUBSECTOR)
    {
        bsp_num = (bspnum & ~NF_SUBSECTOR);
        if (bsp_num >= numsubsectors)
        {
            I_Error("PA_CrossSubsector: ss %i with numss = %i", bsp_num, numsubsectors);
        }

        return PA_CrossSubsector(&subsectors[bsp_num]);
    }

    bsp = &nodes[bspnum];

    //
    // decide which side the start point is on
    //
    side = PA_DivlineSide(shootdiv.x, shootdiv.y, &bsp->line);

    // cross the starting side

    if (!PA_CrossBSPNode(bsp->children[side]))
        return false;

    // the partition plane is crossed here
    if (side == PA_DivlineSide(shootx2, shooty2, &bsp->line))
        return true;    // the line doesn't touch the other side

    // cross the ending side
	return PA_CrossBSPNode(bsp->children[side ^ 1]);
}

