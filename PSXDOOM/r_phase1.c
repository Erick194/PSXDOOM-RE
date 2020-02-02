
//Renderer phase 1 - BSP traversal

#include "doomdef.h"
#include "r_local.h"

int checkcoord[12][4] =				//800676C8
{
	{ 3, 0, 2, 1 },/* Above,Left */
	{ 3, 0, 2, 0 },/* Above,Center */
	{ 3, 1, 2, 0 },/* Above,Right */
	{ 0, 0, 0, 0 },
	{ 2, 0, 2, 1 },/* Center,Left */
	{ 0, 0, 0, 0 },/* Center,Center */
	{ 3, 1, 3, 0 },/* Center,Right */
	{ 0, 0, 0, 0 },
	{ 2, 0, 3, 1 },/* Below,Left */
	{ 2, 1, 3, 1 },/* Below,Center */
	{ 2, 1, 3, 0 },/* Below,Right */
	{ 0, 0, 0, 0 }
};

extern subsector_t *solidsubsectors[MAXSUBSECTORS];	//800A8F98 /* List of valid ranges to scan through */
extern subsector_t **endsubsector;				    //80077E90|ppsGp00000a80 /* Pointer to the first free entry */
extern byte solidcols[256];				            //800A8D2C psxdoom

void	R_RenderBSPNode(int bspnum);
boolean R_CheckBBox(fixed_t bspcoord[4]);
void	R_Subsector(int num);
void	R_AddLine(seg_t *line);

//
// Kick off the rendering process by initializing the solidsubsectors array and then
// starting the BSP traversal.
//

void R_BSP(void) //L8002ABB8()
{
	endsubsector = solidsubsectors; /* Init the free memory pointer */
	D_memset(solidcols, 0, 256);
	rendersky = false;
	R_RenderBSPNode(numnodes - 1);  /* Begin traversing the BSP tree for all walls in render range */
}

//
// Recursively descend through the BSP, classifying nodes according to the
// player's point of view, and render subsectors in view.
//

void R_RenderBSPNode(int bspnum)//L8002AC0C()
{
	node_t *bsp;
	//int     side;
	fixed_t	dx, dy;
	fixed_t	left, right;

	//printf("R_RenderBSPNode\n");

	if (bspnum & NF_SUBSECTOR) // reached a subsector leaf?
	{
		if (bspnum == -1)
        {
			R_Subsector(0);
        }
		else
        {
			R_Subsector(bspnum & ~NF_SUBSECTOR);
        }
	}
    else
    {
        bsp = &nodes[bspnum];

        //decide which side the view point is on
        //side = R_PointOnSide(viewx, viewy, bsp);
        dx = (viewx - bsp->line.x);
        dy = (viewy - bsp->line.y);

        left = (bsp->line.dy >> 16) * (dx >> 16);
        right = (dy >> 16) * (bsp->line.dx >> 16);

        // Depending on which side of the halfspace we are on, reverse the traversal order:
        if (right < left)
        {
            if (R_CheckBBox(bsp->bbox[0]))
                R_RenderBSPNode(bsp->children[0]);

            if (R_CheckBBox(bsp->bbox[1]))
                R_RenderBSPNode(bsp->children[1]);
        }
        else
        {
            if (R_CheckBBox(bsp->bbox[1]))
                R_RenderBSPNode(bsp->children[1]);

            if (R_CheckBBox(bsp->bbox[0]))
                R_RenderBSPNode(bsp->children[0]);
        }
        #if 0
        if (right < left)
            side = 0;		/* front side */
        else
            side = 1;		/* back side */

        // recursively render front space
        if (R_CheckBBox(bsp->bbox[side]))
            R_RenderBSPNode(bsp->children[side]);

        // possibly divide back space
        if (R_CheckBBox(bsp->bbox[side ^ 1]))
            R_RenderBSPNode(bsp->children[side ^ 1]);
        #endif // 0
    }
}

//
// Checks BSP node/subtree bounding box. Returns true if some part of the bbox
// might be visible.
//
#define SKIPVIS_FUDGE (FRACUNIT * 2)

boolean R_CheckBBox(fixed_t bspcoord[4])//L8002AD44()
{
	int boxx;
	int boxy;
	int boxpos;

	fixed_t x1, y1, x2, y2;
	byte *solid_cols;
	SVECTOR v0;
	VECTOR v1;
	long flag[4];
	fixed_t vp1, vp2;
	long vx1, vy1, vx2, vy2, delta;
	long Xstart, Xend;
	boolean skipvischeck;

	// find the corners of the box that define the edges from current viewpoint
	if (viewx < bspcoord[BOXLEFT])//if (viewx <= bspcoord[BOXLEFT])
		boxx = 0;
	else if (viewx <= bspcoord[BOXRIGHT])//if (viewx < bspcoord[BOXRIGHT])
		boxx = 1;
	else
		boxx = 2;

	if (viewy > bspcoord[BOXTOP])//if (viewy >= bspcoord[BOXTOP])
		boxy = 0;
	else if (viewy >= bspcoord[BOXBOTTOM])//if (viewy > bspcoord[BOXBOTTOM])
		boxy = 1;
	else
		boxy = 2;

	boxpos = (boxy << 2) + boxx;
	if (boxpos == 5)
		return true;

	x1 = bspcoord[checkcoord[boxpos][0]];
	y1 = bspcoord[checkcoord[boxpos][1]];
	x2 = bspcoord[checkcoord[boxpos][2]];
	y2 = bspcoord[checkcoord[boxpos][3]];

	if (boxpos != 4)
    {
        if (boxpos < 5)
        {
            if (boxpos == 1)
            {
                // Above, Inside
                skipvischeck = (viewy - y1 <= SKIPVIS_FUDGE);
            } else
            {
                // Above, Left or Right
                skipvischeck = false;
            }
        }
        else if (boxpos == 6)
        {
            // Inside, Right
            skipvischeck = (viewx - x1 <= SKIPVIS_FUDGE);
        }
        else if (boxpos == 9)
        {
            // Below, Inside
            skipvischeck = (viewy - y1 >= -SKIPVIS_FUDGE);
        }
        else
        {
            // Below, Left or Right
            skipvischeck = false;
        }
    }
    else
    {
        // Inside, Left
        skipvischeck = (viewx - x1 >= -SKIPVIS_FUDGE);
    }

    // If we decided to skip the check just assume the node is visible
    if (skipvischeck)
        return true;

    v0.vy = 0;
    v0.vx = (short)((x1 - viewx) >> 16);
    v0.vz = (short)((y1 - viewy) >> 16);
    RotTrans(&v0,&v1,flag);

    vy1 = v1.vz;
    vx1 = v1.vx;

    v0.vy = 0;
    v0.vx = (short)((x2 - viewx) >> 16);
    v0.vz = (short)((y2 - viewy) >> 16);
    RotTrans(&v0,&v1,flag);

    vy2 = v1.vz;
    vx2 = v1.vx;

    if ((vx1 < -vy1) && (vx2 < -vy2))
        return false;

    if ((vy1 < vx1) && (vy2 < vx2))
        return false;

    if ((vx1 < -vy1) && (-vy2 < vx2))
    {
        delta = (vx1 + vy1);
        delta = (delta << 16) / ((delta - vx2) - vy2);
        delta = delta * (vy2 - vy1);

        vy1 += (delta >> 16);
        vx1 = -vy1;
    }

    if ((vx1 < vy1) && (vy2 < vx2))
    {
        delta = (vx1 - vy1);
        delta = (delta << 16) / ((delta - vx2) + vy2);
        delta = delta * (vy2 - vy1);

        vy2 = vy1 + (delta >> 16);
        vx2 = vy2;
    }

    if ((vy1 < 0) && (vy2 < 0))
        return false;

    if ((vy1 < 2) && (vy2 < 2))
        return true;

    if (vy1 < 1)
        vy1 = 1;

    if (vy2 < 1)
        vy2 = 1;

    Xstart  = (((vx1 << 7) / vy1) + 0x80);
    Xend    = (((vx2 << 7) / vy2) + 0x80);

    if (Xstart < 0)
        Xstart = 0;

    if (Xend > 256)
        Xend = 256;

    solid_cols = &solidcols[Xstart];
    while (Xstart < Xend)
    {
        if (*solid_cols == 0)
            return true;
        solid_cols++;
        Xstart++;
    }

    return false;
}

//
// Determine floor/ceiling planes, add sprites of things in sector,
// draw one or more segments.
//

void R_Subsector(int num)//L8002B1A8()
{
	subsector_t *sub;
	seg_t       *line;
	int          count;

	if (num >= numsubsectors)
	{
		I_Error("R_Subsector: ss %i with numss = %i", num, numsubsectors);
	}

	if ((endsubsector - solidsubsectors) < MAXSUBSECTORS)
	{
		sub = &subsectors[num];
		frontsector = sub->sector;

		*endsubsector = sub;//copy subsector
		endsubsector++;

		line = &segs[sub->firstline];
		count = sub->numlines;
        do
        {
            R_AddLine(line);	/* Render each line */
            ++line;				/* Inc the line pointer */
        } while (--count);		/* All done? */
	}
}

//
// Clips the given segment and adds any visible pieces to the line list.
//

void R_AddLine(seg_t *line)//L8002B288
{
	sector_t    *backsector;
	vertex_t    *vrt, *vrt2;
	int         x1, y1, x2, y2, xpos1, xpos2, count;
	long        Xstart, Xend, delta;
	byte        *solid_cols;
	SVECTOR     v0;
	VECTOR      v1;
	long	    flag[4];

	line->flag &= ~1;

	vrt = line->v1;
	if (vrt->index != TextureCacheIdx)
	{
        v0.vy = 0;
        v0.vx = (short)((vrt->x - viewx) >> 16);
        v0.vz = (short)((vrt->y - viewy) >> 16);
        RotTrans(&v0,&v1,flag);

        vrt->vx = v1.vx;
        vrt->vy = v1.vz;

		if (vrt->vy >= 4)
        {
            vrt->yy = 0x800000 / vrt->vy;
            vrt->xx = ((vrt->vx * (0x800000 / vrt->vy)) >> 16) + 0x80;
        }

		vrt->index = TextureCacheIdx;

        y1 = vrt->vy;
        x1 = vrt->vx;
	}
	else
	{
        x1 = vrt->vx;
        y1 = vrt->vy;
	}

	vrt2 = line->v2;
	if (vrt2->index != TextureCacheIdx)
	{
        v0.vy = 0;
        v0.vx = (short)((vrt2->x - viewx) >> 16);
        v0.vz = (short)((vrt2->y - viewy) >> 16);
        RotTrans(&v0,&v1,flag);

        vrt2->vx = v1.vx;
        vrt2->vy = v1.vz;

		if (vrt2->vy >= 4)
		{
            vrt2->yy = 0x800000 / vrt2->vy;
            vrt2->xx = ((vrt2->vx * (0x800000 / vrt2->vy)) >> 16) + 0x80;
		}

		vrt2->index = TextureCacheIdx;

		y2 = vrt2->vy;
        x2 = vrt2->vx;
	}
	else
	{
        x2 = vrt2->vx;
        y2 = vrt2->vy;
	}

	if ((x1 >= -y1) || (x2 >= -y2))
    {
        if ((y1 < x1) && (y2 < x2))
            return;

		if (((x2 * y1) - (x1 * y2)) > 0)
        {

            if ((x1 < -y1) && (-y2 < x2))
			{
                delta = (x1 + y1);
                delta = (delta << 16) / ((delta - x2) - y2);
                delta = delta * (y2 - y1);

                y1 += (delta >> 16);
                x1 = -y1;
			}

			if ((x1 < y1) && (y2 < x2))
			{
                delta = (x1 - y1);
                delta = (delta << 16) / ((delta - x2) + y2);
                delta = delta * (y2 - y1);

                y2 = y1 + (delta >> 16);
                x2 = y2;
			}

			if ((y1 >= 3) || (y2 >= 3))
            {
                if ((y1 < 2) && (y2 > 2))
				{
					delta = (2 - y1);
					delta = (delta << 16) / (y2 - y1);
					delta = delta * (x2 - x1);

					y1 = 2;
					x1 += (delta >> 16);
				}
				else if ((y2 < 2) && (y1 > 2))
				{
					delta = (2 - y2);
					delta = (delta << 16) / (y1 - y2);
					delta = delta * (x1 - x2);

					y2 = 2;
					x2 += (delta >> 16);
				}

				Xstart  = (((x1 << 7) / y1) + 0x80);
				Xend    = (((x2 << 7) / y2) + 0x80);

                if (Xstart != Xend)
                {
                    if (frontsector->ceilingpic == -1)
                        rendersky = true;

                    if (Xstart < 0)
                        Xstart = 0;

                    if (Xend > 256)
                        Xend = 256;

                    solid_cols = &solidcols[Xstart];
                    count = Xstart;
                    xpos1 = 256;
                    while (count < Xend)
                    {
                        xpos1 = count;
                        if (*solid_cols == 0) break;
                        solid_cols++;
                        count++;
                    }

                    solid_cols = &solidcols[Xend-1];
                    count = Xend;
                    xpos2 = 0;
                    while (count >= Xstart)
                    {
                        xpos2 = count;
                        if (*solid_cols == 0) break;
                        solid_cols--;
                        count--;
                    }

                    if (xpos1 <= xpos2)
                    {
                        line->xpos1 = (short)xpos1 - 1;
                        line->xpos2 = (short)xpos2 + 1;
                        line->flag |= 1;
                    }

                    if (!(line->linedef->flags & ML_MIDMASKED))
                    {
                        backsector = line->backsector;

                        if(!backsector ||
                            backsector->ceilingheight <= frontsector->floorheight ||
                            backsector->floorheight   >= frontsector->ceilingheight)
                        {
                            solid_cols = &solidcols[Xstart];
                            while (Xstart < Xend)
                            {
                                *solid_cols = 1;
                                Xstart += 1;
                                solid_cols++;
                            }
                        }
                    }
                }
            }
        }
    }
}
