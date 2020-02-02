/* R_local.h */

#ifndef __R_LOCAL__
#define __R_LOCAL__

/* proper screen size would be 160*100, stretched to 224 is 2.2 scale */
#define	STRETCH				(22*FRACUNIT/10)

#define	CENTERX				(SCREENWIDTH/2)
#define	CENTERY				(SCREENHEIGHT/2)
#define	CENTERXFRAC			(SCREENWIDTH/2*FRACUNIT)
#define	CENTERYFRAC			(SCREENHEIGHT/2*FRACUNIT)
#define	PROJECTION			CENTERXFRAC

#define	ANGLETOSKYSHIFT		22		/* sky map is 256*128*4 maps */

#define	BASEYCENTER			100

#define	CENTERY				(SCREENHEIGHT/2)
#define	WINDOWHEIGHT		(SCREENHEIGHT-SBARHEIGHT)

#define	MINZ				4//(FRACUNIT*4)

#define	FIELDOFVIEW			2048   /* fineangles in the SCREENWIDTH wide window */

/* */
/* Seg flags */
/* */
#define SGF_VISIBLE_COLS    1       /* The seg has at least 1 visible (non fully occluded column) */


/* */
/* lighting constants */
/* */
#define	LIGHTLEVELS			256		/* number of diminishing */
#define	INVERSECOLORMAP		255

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

/*================ used by play and refresh */

typedef struct
{
	fixed_t	x, y, dx, dy;
} divline_t;

typedef struct
{
	fixed_t		x,y;//*, *4
	int yy;//*8
	int vx;//*12
	int vy;//*16
	int xx;//*20
	int index; //*24
} vertex_t;

struct line_s;
struct subsector_s;

typedef	struct
{
	fixed_t		floorheight, ceilingheight;
	VINT		floorpic, ceilingpic;	/* if ceilingpic == -1,draw sky */
	short		colorid;			// Psx Doom New
	short		lightlevel;
	VINT		special, tag;

	VINT		soundtraversed;		/* 0 = untraversed, 1,2 = sndlines -1 */
	mobj_t		*soundtarget;		/* thing that made a sound (or null) */

	VINT	    flags;	            // Psx Doom New
	VINT		blockbox[4];		/* mapblock bounding box for height changes */
	degenmobj_t	soundorg;			/* for any sounds played by the sector */

	int			validcount;			/* if == validcount, already checked */
	mobj_t		*thinglist;			/* list of mobjs in sector */
	void		*specialdata;		/* thinker_t for reversable actions */
	VINT		linecount;
	struct line_s	**lines;			/* [linecount] size */

} sector_t;

typedef struct
{
	fixed_t		textureoffset;		/* add this to the calculated texture col */
	fixed_t		rowoffset;			/* add this to the calculated texture top */
	VINT		toptexture, bottomtexture, midtexture;
	sector_t	*sector;
} side_t;

typedef enum {ST_HORIZONTAL, ST_VERTICAL, ST_POSITIVE, ST_NEGATIVE} slopetype_t;

typedef struct line_s
{
	vertex_t	*v1, *v2;
	fixed_t		dx,dy;				/* v2 - v1 for side checking */
	VINT		flags;
	VINT		special, tag;
	VINT		sidenum[2];			/* sidenum[1] will be -1 if one sided */
	fixed_t		bbox[4];
	slopetype_t	slopetype;			/* to aid move clipping */
	sector_t	*frontsector, *backsector;
	int			validcount;			/* if == validcount, already checked */
	void		*specialdata;		/* thinker_t for reversable actions */
	int			fineangle;			/* to get sine / cosine for sliding */
} line_t;

typedef struct subsector_s
{
	sector_t	*sector;	//*
	short		numlines;	//*4
	short		firstline;	//*6
	//Psx Doom
	short       numleafs;	//*8
	short       leaf;		//*10
    short       unk1;		//*12
	short       unk2;		//*14
} subsector_t;

typedef struct seg_s
{
	vertex_t	*v1, *v2;
	fixed_t		offset;
	angle_t		angle;				/* this is not used (keep for padding) */
	side_t		*sidedef;
	line_t		*linedef;
	sector_t	*frontsector;
	sector_t	*backsector;		/* NULL for one sided lines */
	//New PsxDoom
	short       flag;
	short       xpos1;
	short       xpos2;
	short       pad1;
} seg_t;

typedef struct
{
	//fixed_t		x,y,dx,dy;		//old
	divline_t	line;				/* partition line */
	fixed_t		bbox[2][4];			/* bounding box for each child */
	int			children[2];		/* if NF_SUBSECTOR its a subsector */
} node_t;

typedef struct {
	vertex_t    *vertex;
	seg_t       *seg;//*(A24 + 4)
} leaf_t;

#if 0
typedef struct
{
	char		name[8];		/* for switch changing, etc */
	int			width;
	int			height;
	pixel_t		*data;			/* cached data to draw from */
	int			lumpnum;
	int			usecount;		/* for precaching */
	int			pad;
} texture_t;
#endif // 0

//
// Light Data Psx Doom
//
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char pad1;
} light_t;

/*
==============================================================================

						OTHER TYPES

==============================================================================
*/

/* Sprites are patches with a special naming convention so they can be  */
/* recognized by R_InitSprites.  The sprite and frame specified by a  */
/* thing_t is range checked at run time. */
/* a sprite is a patch_t that is assumed to represent a three dimensional */
/* object and may have multiple rotations pre drawn.  Horizontal flipping  */
/* is used to save space. Some sprites will only have one picture used */
/* for all views.   */

#ifdef MARS

int spritelump[NUMSPRITES];	/* no rotations, so just add frame num... */

#else

typedef struct
{
	boolean		rotate;		/* if false use 0 for any position */
	int			lump[8];	/* lump to use for view angles 0-7 */
	byte		flip[8];	/* flip (1 = flip) to use for view angles 0-7 */
} spriteframe_t;

typedef struct
{
	int				numframes;
	spriteframe_t	*spriteframes;
} spritedef_t;

extern	spritedef_t		sprites[NUMSPRITES];

#endif

/*
===============================================================================

							MAP DATA

===============================================================================
*/

extern	int			numvertexes;
extern	vertex_t	*vertexes;

extern	int			numsegs;
extern	seg_t		*segs;

extern	int			numsectors;
extern	sector_t	*sectors;

extern	int			numsubsectors;
extern	subsector_t	*subsectors;

extern	int			numnodes;
extern	node_t		*nodes;

extern	int			numlines;
extern	line_t		*lines;

extern	int			numsides;
extern	side_t		*sides;

extern	int			numleafs;
extern	leaf_t		*leafs;

/*============================================================================= */

/*------*/
/*R_main*/
/*------*/
extern light_t      *baselight; //*(r28 + 2672) L80077E80
extern unsigned int base_r;	//*(r28 + 2220) L80077CBC//r
extern unsigned int base_g;	//*(r28 + 2640) L80077E60//g
extern unsigned int base_b;	//*(r28 + 2444) L80077D9C//b
extern boolean      viewlighting;    //80078088
extern sector_t     *frontsector;	//80077E38 //psGp00000a28

/*------*/
/*R_data*/
/*------*/
void	R_InitData (void);
//void	R_InitSpriteDefs (char **namelist);

/*--------*/
/*r_phase1*/
/*--------*/
void	R_BSP (void);
void	R_RenderBSPNode (int bspnum);

/*--------*/
/*r_phase2*/
/*--------*/
void R_RenderSKY(void);
void R_Render_Hud_Weapons(void);


/* to get a global angle from cartesian coordinates, the coordinates are */
/* flipped until they are in the first octant of the coordinate system, then */
/* the y (<=x) is scaled and divided by x to get a tangent (slope) value */
/* which is looked up in the tantoangle[] table.  The +1 size is to handle */
/* the case when x==y without additional checking. */
#define	SLOPERANGE	2048
#define	SLOPEBITS	11
#define	DBITS		(FRACBITS-SLOPEBITS)

extern	int	tantoangle[SLOPERANGE+1];


#define	VIEW_3D_H 200
extern	fixed_t	yslope[VIEW_3D_H];

#define	HEIGHTBITS			6
#define	SCALEBITS			9

#define	FIXEDTOSCALE		(FRACBITS-SCALEBITS)
#define	FIXEDTOHEIGHT		(FRACBITS-HEIGHTBITS)


#define	HALF_SCREEN_W       (SCREENWIDTH/2)

extern	fixed_t		viewx, viewy, viewz;    //80077D0C, 80077D10, 80077D18
extern	angle_t		viewangle;              //800780B8
extern	fixed_t		viewcos, viewsin;       //80077EC8, 80077EE0

extern	player_t	*viewplayer;            //80077D60

extern	fixed_t		finetangent[FINEANGLES/2];

extern	int			validcount; //800779F4
//extern	int			framecount;

/* */
/* R_data.c */
/* */

#define	MAXSUBSECTORS	192		/* Maximum number of subsectors to scan */

extern	short		skypalette;
extern	psxobj_t	*skytexturep;

extern	int			firsttex, lasttex, numtextures;
extern	psxobj_t	*textures;

extern	int			*flattranslation;		/* for global animation */
extern	int			*texturetranslation;	/* for global animation */

extern	int			firstflat, lastflat,  numflats;
extern	psxobj_t	*texflats;

extern	int			firstsprite, lastsprite, numsprites;
extern	psxobj_t	*texsprites;


#define MAX_PALETTES 20
extern	short palette[MAX_PALETTES];
extern	short palettebase;
extern	light_t		*lights;

#endif		/* __R_LOCAL__ */

