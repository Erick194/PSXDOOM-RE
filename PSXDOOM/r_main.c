/* r_main.c */

#include "doomdef.h"
#include "r_local.h"

/*===================================== */

/* */
/* subsectors */
/* */
//subsector_t		*vissubsectors[MAXVISSSEC], **lastvissubsector;

/* */
/* walls */
/* */
//viswall_t	viswalls[MAXWALLCMDS], *lastwallcmd;

/* */
/* planes */
/* */
//visplane_t	visplanes[MAXVISPLANES], *lastvisplane;

/* */
/* sprites */
/* */
//vissprite_t	vissprites[MAXVISSPRITES], *lastsprite_p, *vissprite_p;

/* */
/* openings / misc refresh memory */
/* */
//unsigned short	openings[MAXOPENINGS], *lastopening;


/*===================================== */

//boolean		phase1completed;

//pixel_t		*workingscreen;


fixed_t		viewx, viewy, viewz;    //80077D0C|uGp000008fc, 80077D10|uGp00000900, 80077D18|uGp00000908
angle_t		viewangle;              //800780B8|uGp00000ca8
fixed_t		viewcos, viewsin;       //80077EC8|iGp00000ab8, 80077EE0|iGp00000ad0
player_t	*viewplayer;            //80077D60|ppGp00000950

int			validcount = 1;		/* increment every time a check is made */ //800779F4
//int			framecount;		    /* incremented every frame */

/* */
/* sky mapping */
/* */
int			skytexture;
boolean     rendersky;//8007801C|iGp00000c0c

subsector_t *solidsubsectors[MAXSUBSECTORS];	//800A8F98                  /* List of valid ranges to scan through */
subsector_t **endsubsector;				        //80077E90|ppsGp00000a80    /* Pointer to the first free entry */
int numdrawsubsectors;                          //80077F14|iGp00000b04

byte solidcols[256];				    //800A8D2C psxdoom

/* */
/* precalculated math */
/* */
fixed_t		*finecosine = &finesine[FINEANGLES/4];//80077A00


/* */
/* News */
/* */
light_t         *baselight;             //pbGp00000a70 L80077E80
unsigned int    base_r;	                //iGp000008ac L80077CBC//r
unsigned int    base_g;	                //iGp00000a50 L80077E60//g
unsigned int    base_b;	                //iGp0000098c L80077D9C//b
boolean         viewlighting;           //80078088|iGp00000c78
sector_t        *frontsector;	        //80077E38|psGp00000a28
light_t         *lights;		        //80077E94|pbGp00000a84
short           palette[MAX_PALETTES];	//800A8E68
short           palettebase;            //80077DA8

/*============================================================================= */

/*
==============
=
= R_Init
=
==============
*/

static MATRIX R_Matrix;//80086358

void R_Init (void) //L80030480()
{
    R_InitData ();

	//Initialize Matrix
	R_Matrix.t[0] = 0;
	R_Matrix.t[1] = 0;
	R_Matrix.t[2] = 0;
	SetTransMatrix(&R_Matrix);

	R_Matrix.m[0][0] = 0;
	R_Matrix.m[0][1] = 0;
	R_Matrix.m[0][2] = 0;
	R_Matrix.m[1][0] = 0;
	R_Matrix.m[1][1] = 4096;
	R_Matrix.m[1][2] = 0;
	R_Matrix.m[2][0] = 0;
	R_Matrix.m[2][1] = 0;
	R_Matrix.m[2][2] = 0;
	SetRotMatrix(&R_Matrix);
}

/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView(void)//L80030504()
{
	RECT    area;
	angle_t angle;
	DR_TWIN *texwindow = (DR_TWIN*) getScratchAddr(128);//1F800200

	if (!viewlighting)
    {
		base_r = 128;
		base_g = 128;
		base_b = 128;
		baselight = &lights[0];
	}

	viewplayer = &players[consoleplayer];

	viewx = viewplayer->mo->x & (~FRACMASK);
	viewy = viewplayer->mo->y & (~FRACMASK);
	viewz = viewplayer->viewz & (~FRACMASK);

	viewangle = viewplayer->mo->angle;
	viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];
	viewsin = finesine[viewangle >> ANGLETOFINESHIFT];

	R_Matrix.m[0][0] =  viewsin >> 4;
	R_Matrix.m[0][2] = -viewcos >> 4;
	R_Matrix.m[2][0] =  viewcos >> 4;
	R_Matrix.m[2][2] =  viewsin >> 4;
	SetRotMatrix(&R_Matrix);

	R_BSP();

	numdrawsubsectors = (endsubsector - solidsubsectors);

	DrawRender();

	if (rendersky)
		R_RenderSKY();

    while (endsubsector--, (solidsubsectors-1) < endsubsector)
    {
        frontsector = (*endsubsector)->sector;

        if (viewlighting)
        {
            baselight = &lights[frontsector->colorid];

            base_r = (frontsector->lightlevel * baselight->r) >> 8;
            base_g = (frontsector->lightlevel * baselight->g) >> 8;
            base_b = (frontsector->lightlevel * baselight->b) >> 8;

            if (viewplayer->extralight != 0)
            {
                base_r += viewplayer->extralight;
                base_g += viewplayer->extralight;
                base_b += viewplayer->extralight;

                if (base_r > 255) { base_r = 255; }
                if (base_g > 255) { base_g = 255; }
                if (base_b > 255) { base_b = 255; }
            }
        }

        R_RenderAll(*endsubsector);
    }

	R_Render_Hud_Weapons();

	setRECT(&area, 0, 0, 0, 0);
	SetTexWindow(texwindow, &area);
	W_AddPrim(texwindow);   /* add to order table*/
}

/*============================================================================= */

/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/

extern	int	tantoangle[SLOPERANGE + 1];

inline int SlopeDiv(unsigned num, unsigned den)//L80030A28()
{
	unsigned ans;
	if (den < 512)
		return SLOPERANGE;
	ans = (num << 3) / (den >> 8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)//L80030A70()
{
	int		x;
	int		y;

	x = x2 - x1;
	y = y2 - y1;

	if ((!x) && (!y))
		return 0;

	if (x >= 0)
	{	/* x >=0 */
		if (y >= 0)
		{	/* y>= 0 */
			if (x>y)
				return tantoangle[SlopeDiv(y, x)];     /* octant 0 */
			else
				return ANG90 - 1 - tantoangle[SlopeDiv(x, y)];  /* octant 1 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return -tantoangle[SlopeDiv(y, x)];  /* octant 8 */
			else
				return ANG270 + tantoangle[SlopeDiv(x, y)];  /* octant 7 */
		}
	}
	else
	{	/* x<0 */
		x = -x;
		if (y >= 0)
		{	/* y>= 0 */
			if (x>y)
				return ANG180 - 1 - tantoangle[SlopeDiv(y, x)]; /* octant 3 */
			else
				return ANG90 + tantoangle[SlopeDiv(x, y)];  /* octant 2 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return ANG180 + tantoangle[SlopeDiv(y, x)];  /* octant 4 */
			else
				return ANG270 - 1 - tantoangle[SlopeDiv(x, y)];  /* octant 5 */
		}
	}
}


/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/
inline int	R_PointOnSide(int x, int y, node_t *node)//L80030D84()
{
	fixed_t	dx, dy;
	fixed_t	left, right;

	if (!node->line.dx)
	{
		if (x <= node->line.x)
			return node->line.dy > 0;
		return node->line.dy < 0;
	}
	if (!node->line.dy)
	{
		if (y <= node->line.y)
			return node->line.dx < 0;
		return node->line.dx > 0;
	}

	dx = (x - node->line.x);
	dy = (y - node->line.y);

	left = (node->line.dy >> 16) * (dx >> 16);
	right = (dy >> 16) * (node->line.dx >> 16);

	if (right < left)
		return 0;		/* front side */
	return 1;			/* back side */
}



/*
==============
=
= R_PointInSubsector
=
==============
*/
struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y)//L80030E2C()
{
	node_t	*node;
	int		side, nodenum;

	if (!numnodes)				/* single subsector is a special case */
		return subsectors;

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	return &subsectors[nodenum & ~NF_SUBSECTOR];
}
