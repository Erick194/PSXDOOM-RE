/* P_main.c */

#include "doomdef.h"
#include "p_local.h"

void	P_SpawnMapThing (mapthing_t *mthing);
void	P_Init(void);
void	P_SetupLevel(int map, skill_t skill);
void	P_LoadBlocks(char *filename);
void	P_CacheSprite(spritedef_t *sprdef);
void	P_LoadTextureByWidth(int width);

int			numvertexes;	//80077E44|uGp00000a34
vertex_t	*vertexes;		//8007800C|puGp00000bfc

int			numsegs;		//80077ECC
seg_t		*segs;			//8007805C

int			numsectors;		//80077D80
sector_t	*sectors;		//80077ED0

int			numsubsectors;	//80078048
subsector_t	*subsectors;	//80077D6C

int			numnodes;		//80077FE0
node_t		*nodes;			//80077CD0

int			numlines;		//80077FF0
line_t		*lines;			//80077CDC

int			numsides;		//80077FDC
side_t		*sides;			//80077CCC

int			numleafs;		//80077D90
leaf_t		*leafs;			//80077F34

short		*blockmaplump;			//80077EEC /* offsets in blockmap are from here */
short		*blockmap;
int			bmapwidth, bmapheight;	/* in mapblocks */ //800780A8, 80077CE4
fixed_t		bmaporgx, bmaporgy;		/* origin of block map */ //80077FB4,80077FBC
mobj_t		**blocklinks;			/* for thing chains */ //80077D08

byte		*rejectmatrix;			/* for fast sight rejection */

//mapthing_t	deathmatchstarts[10], *deathmatch_p;//80097e4c, 80077E8C
//mapthing_t	playerstarts[MAXPLAYERS];//800a8c60

/*
=================
=
= P_LoadVertexes
=
=================
*/
//inline
void P_LoadVertexes (int lump)//L80021A64()
{
	byte		*data;
	int			i, lumpSize;
	mapvertex_t	*ml;
	vertex_t	*li;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadVertexes: lump > 64K");

	numvertexes = W_MapLumpLength(lump) / sizeof(mapvertex_t);
	vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	ml = (mapvertex_t *)data;
	li = vertexes;
	for (i=0 ; i<numvertexes ; i++, li++, ml++)
	{
		li->x = LITTLELONG(ml->x);
		li->y = LITTLELONG(ml->y);
		li->index = 0;
	}
}

/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs (int lump)//L80021B38()
{
	byte		*data;
	int			i;
	mapseg_t	*ml;
	seg_t		*li;
	line_t		*ldef;
	int			linedef, side;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadSegs: lump > 64K");

	numsegs = W_MapLumpLength(lump) / sizeof(mapseg_t);
	segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);
	D_memset (segs, 0, numsegs*sizeof(seg_t));

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	ml = (mapseg_t *)data;
	li = segs;
	for (i=0 ; i<numsegs ; i++, li++, ml++)
	{
		li->v1 = &vertexes[LITTLESHORT(ml->v1)];
		li->v2 = &vertexes[LITTLESHORT(ml->v2)];

		li->angle = (LITTLESHORT(ml->angle)) << FRACBITS;
		li->offset = (LITTLESHORT(ml->offset)) << FRACBITS;

		linedef = LITTLESHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;

		side = LITTLESHORT(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];

		li->frontsector = sides[ldef->sidenum[side]].sector;

		if (ldef-> flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side^1]].sector;
		else
			li->backsector = 0;

		if (ldef->v1 == li->v1)
			ldef->fineangle = li->angle >> ANGLETOFINESHIFT;
	}
}


/*
=================
=
= P_LoadSubsectors
=
=================
*/
//inline
void P_LoadSubsectors (int lump)//L80021D70()
{
	byte			*data;
	int				i;
	mapsubsector_t	*ms;
	subsector_t		*ss;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadSubsectors: lump > 64K");

	numsubsectors = W_MapLumpLength (lump) / sizeof(mapsubsector_t);
	subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
	D_memset (subsectors,0, numsubsectors*sizeof(subsector_t));

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	ms = (mapsubsector_t *)data;
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
	{
		ss->numlines = LITTLESHORT(ms->numsegs);
		ss->firstline = LITTLESHORT(ms->firstseg);
		ss->numleafs = 0;
		ss->leaf = 0;
	}
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors (int lump)//L80021E5C()
{
	byte			*data;
	int				i;
	mapsector_t		*ms;
	sector_t		*ss;
	int				skytexture;
	char		    skyname[16];
	skyname[0] = 'S';
	skyname[1] = 'K';
	skyname[2] = 'Y';
	skyname[3] = 0;
	skyname[4] = 0;
	skyname[5] = 0;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadSectors: lump > 64K");

	numsectors = W_MapLumpLength(lump) / sizeof(mapsector_t);
	sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
	D_memset (sectors, 0, numsectors*sizeof(sector_t));

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	ms = (mapsector_t *)data;
	ss = sectors;
	for (i=0 ; i<numsectors ; i++, ss++, ms++)
	{
		ss->floorheight = LITTLESHORT(ms->floorheight)<<FRACBITS;
		ss->ceilingheight = LITTLESHORT(ms->ceilingheight)<<FRACBITS;
		ss->colorid = ms->colorid;
		ss->lightlevel = ms->lightlevel;
		ss->special = LITTLESHORT(ms->special);
		ss->thinglist = NULL;
		ss->tag = LITTLESHORT(ms->tag);
		ss->flags = LITTLELONG(ms->flags);

		ss->floorpic = R_FlatNumForName(ms->floorpic);
		if (!D_strncasecmp(ms->ceilingpic, "F_SKY", 5))
		{
			ss->ceilingpic = -1;
			skyname[3] = ms->ceilingpic[5];
			skyname[4] = ms->ceilingpic[6];
		}
		else
		{
			ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
		}
	}

	skytexturep = NULL;
	if (skyname[3] != 0)
	{
		skytexture = R_TextureNumForName(skyname);
		skytexturep = &textures[skytexture];
	}
}

/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes (int lump)//L8002209C()
{
	byte		*data;
	int			i,j,k;
	mapnode_t	*mn;
	node_t		*no;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadNodes: lump > 64K");

	numnodes = W_MapLumpLength(lump) / sizeof(mapnode_t);
	nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	mn = (mapnode_t *)data;
	no = nodes;
	for (i=0 ; i<numnodes ; i++, no++, mn++)
	{
		no->line.x = LITTLESHORT(mn->x) << FRACBITS;
		no->line.y = LITTLESHORT(mn->y) << FRACBITS;
		no->line.dx = LITTLESHORT(mn->dx) << FRACBITS;
		no->line.dy = LITTLESHORT(mn->dy) << FRACBITS;
		for (j=0 ; j<2 ; j++)
		{
			no->children[j] = (unsigned short)LITTLESHORT(mn->children[j]);
			for (k=0 ; k<4 ; k++)
				no->bbox[j][k] = LITTLESHORT(mn->bbox[j][k]) << FRACBITS;
		}
	}
}

/*
=================
=
= P_LoadThings
=
=================
*/
//inline
void P_LoadThings (int lump)//L80022210()
{
	byte			*data;
	int				i;
	mapthing_t		*mt;
	int				numthings;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadThings: lump > 64K");

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	numthings = W_MapLumpLength(lump) / sizeof(mapthing_t);

	mt = (mapthing_t *)data;
	for (i=0 ; i<numthings ; i++, mt++)
	{
		mt->x = LITTLESHORT(mt->x);
		mt->y = LITTLESHORT(mt->y);
		mt->angle = LITTLESHORT(mt->angle);
		mt->type = LITTLESHORT(mt->type);
		mt->options = LITTLESHORT(mt->options);
		P_SpawnMapThing (mt);

		if (mt->type >= 4096)
			I_Error("P_LoadThings: doomednum:%d >= 4096", mt->type);
	}
}



/*
=================
=
= P_LoadLineDefs
=
= Also counts secret lines for intermissions
=================
*/

void P_LoadLineDefs (int lump)//L80022314()
{
	byte			*data;
	int				i;
	maplinedef_t	*mld;
	line_t			*ld;
	vertex_t		*v1, *v2;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadLineDefs: lump > 64K");

	numlines = W_MapLumpLength(lump) / sizeof(maplinedef_t);
	lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);
	D_memset (lines, 0, numlines*sizeof(line_t));

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	mld = (maplinedef_t *)data;
	ld = lines;
	for (i=0 ; i<numlines ; i++, mld++, ld++)
	{
		ld->flags = LITTLESHORT(mld->flags);
		ld->special = LITTLESHORT(mld->special);
		ld->tag = LITTLESHORT(mld->tag);

		v1 = ld->v1 = &vertexes[LITTLESHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[LITTLESHORT(mld->v2)];

		ld->dx = (v2->x - v1->x);
		ld->dy = (v2->y - v1->y);

		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv (ld->dy , ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}

		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}

		ld->sidenum[0] = LITTLESHORT(mld->sidenum[0]);
		ld->sidenum[1] = LITTLESHORT(mld->sidenum[1]);

		if (ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;

		if (ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;
	}
}

/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs (int lump)//L800225E8()
{
	byte			*data;
	int				i;
	mapsidedef_t	*msd;
	side_t			*sd;

	if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadSideDefs: lump > 64K");

	numsides = W_MapLumpLength(lump) / sizeof(mapsidedef_t);
	sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
	D_memset (sides, 0, numsides*sizeof(side_t));

	data = (byte *)tempbuffer;
	W_ReadMapLump(lump, data, true);

	msd = (mapsidedef_t *)data;
	sd = sides;
	for (i=0 ; i<numsides ; i++, msd++, sd++)
	{
		sd->textureoffset = LITTLESHORT(msd->textureoffset)<<FRACBITS;
		sd->rowoffset = LITTLESHORT(msd->rowoffset)<<FRACBITS;
		sd->sector = &sectors[LITTLESHORT(msd->sector)];

		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
	}
}

/*
=================
=
= P_LoadBlockMap
=
=================
*/

void P_LoadBlockMap (int lump)//L80022764()
{
	int		count;
	int		i;
	int     length;

	length = W_MapLumpLength(lump);
	blockmaplump = Z_Malloc(length, PU_LEVEL, 0);
	W_ReadMapLump(lump, blockmaplump, true);

	blockmap = blockmaplump+4;//skip blockmap header
	count = W_MapLumpLength(lump)/2;
	for (i=0 ; i<count ; i++)
		blockmaplump[i] = LITTLESHORT(blockmaplump[i]);

	bmapwidth = blockmaplump[2];
	bmapheight = blockmaplump[3];
	bmaporgx = blockmaplump[0]<<FRACBITS;
	bmaporgy = blockmaplump[1]<<FRACBITS;

	/* clear out mobj chains */
	count = sizeof(*blocklinks)* bmapwidth*bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	D_memset (blocklinks, 0, count);
}

/*
=================
=
= P_LoadReject
= Include On Psx Doom
=
=================
*/
//inline
void P_LoadReject(int lump)//L80022864()
{
	int length;

	length = W_MapLumpLength(lump);
	rejectmatrix = (byte*)Z_Malloc(length, PU_LEVEL, 0);
	W_ReadMapLump(lump, rejectmatrix, true);
}


/*
=================
=
= P_LoadLeafs
= Exclusive Psx Doom
=
=================
*/

void P_LoadLeafs(int lump)//L800228B8()
{
	int         i, j;
	int         length, size, count;
	int			vertex, seg;
	subsector_t *ss;
	leaf_t      *lf;
	byte		*data;
	short       *mlf;

    if (W_MapLumpLength(lump) > 65536)
		I_Error("P_LoadLeafs: lump > 64K");

    data = tempbuffer;
    W_ReadMapLump(lump, data, true);

    size = 0;
    count = 0;
    mlf = (short *)data;
    length = W_MapLumpLength(lump);
    while ((int)mlf < (int)(data + length))
    {
        count += 1;
        size += (int)*mlf;
        mlf += (int)(*mlf << 1) + 1;
    }

    if (count != numsubsectors)
		I_Error("P_LoadLeafs: leaf/subsector inconsistancy\n");

	leafs = Z_Malloc(size * sizeof(leaf_t), PU_LEVEL, 0);//plGp00000b24 = leafs;

    lf = leafs;
    ss = subsectors;

	numleafs = 0;
    mlf = (short *)data;
	for (i = 0; i < count; i++, ss++)
	{
        ss->numleafs = LITTLESHORT(*mlf++);
		ss->leaf = (short)numleafs;

		for (j = 0; j < (int)ss->numleafs; j++, lf++)
        {
            vertex = LITTLESHORT(*mlf++);

			if (vertex >= numvertexes)
				I_Error("P_LoadLeafs: vertex out of range\n");

			lf->vertex = &vertexes[vertex];

			seg = LITTLESHORT(*mlf++);
			if (seg != -1)
			{
				if (seg >= numsegs)
					I_Error("P_LoadLeafs: seg out of range\n");

				lf->seg = &segs[seg];
			}
			else
			{
				lf->seg = NULL;
			}
        }
        numleafs += (int)ss->numleafs;
	}
}

/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
=================
*/

void P_GroupLines (void)//L80022AF0()
{
	line_t		**linebuffer;
	int			i, j, total;
	sector_t	*sector;
	subsector_t	*ss;
	seg_t		*seg;
	int			block;
	line_t		*li;
	fixed_t		bbox[4];

/* look up sector number for each subsector */
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++)
	{
		seg = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
	}

/* count number of lines in each sector */
	li = lines;
	total = 0;
	for (i=0 ; i<numlines ; i++, li++)
	{
		total++;
		li->frontsector->linecount++;
		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

/* build line tables for each sector	 */
	linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
	sector = sectors;
	for (i=0 ; i<numsectors ; i++, sector++)
	{
		M_ClearBox (bbox);
		sector->lines = linebuffer;
		li = lines;
		for (j=0 ; j<numlines ; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				M_AddToBox (bbox, li->v1->x, li->v1->y);
				M_AddToBox (bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			I_Error ("P_GroupLines: miscounted");

		/* set the degenmobj_t to the middle of the bounding box */
		sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
		sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
		//sector->soundorg.z = (sector->floorheight + sector->ceilingheight) / 2;

		/* link into subsector */
		sector->soundorg.subsec = R_PointInSubsector(sector->soundorg.x, sector->soundorg.y);

		/* adjust bounding box to map blocks */
		block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block >= bmapheight) ? bmapheight-1 : block;
		sector->blockbox[BOXTOP]=block;

		block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block < 0) ? 0 : block;
		sector->blockbox[BOXBOTTOM]=block;

		block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block >= bmapwidth) ? bmapwidth-1 : block;
		sector->blockbox[BOXRIGHT]=block;

		block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = (block < 0) ? 0 : block;
		sector->blockbox[BOXLEFT]=block;
	}
}

/*============================================================================= */

/*
=================
=
= P_Init
=
=================
*/

void P_Init (void)//L80022E00()
{
	int i;
	sector_t    *sector;
	side_t      *sd;

	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		if ((sector->ceilingpic != -1) && (texflats[sector->ceilingpic].vtpage == 0))
		{
            TextureCache(&texflats[sector->ceilingpic]);
		}

		if (texflats[sector->floorpic].vtpage == 0)
		{
			TextureCache(&texflats[sector->floorpic]);
		}
	}

	//V_pages
	PageCount = 2;
	xcount = 0;
	ycount = 0;
	xycount = 0;
	V_PagFlags|= 2;//sky valloc

	spreadfire = NULL;
	skypalette = palette[0];

	if (skytexturep)
	{
		if (lumpinfo[skytexturep->lump].name[4] == '9')// initialize fire sky
		{
			W_CacheLumpNum(skytexturep->lump, PU_ANIMATION, true);

			spreadfire = P_FireSky;
			skypalette = palette[15];

			for (i = 0; i < 64; i++)
				P_FireSky(skytexturep);
		}

		TextureCache(skytexturep);
	}

	P_LoadTextureByWidth(16);
	P_LoadTextureByWidth(64);
	P_InitSwitchList();
	P_LoadTextureByWidth(128);

	sd = sides;
	for (i = 0; i < numsides; i++, sd++)
	{
		if (sides->toptexture == -1)
			sides->toptexture = 0;

		if (sides->midtexture == -1)
			sides->midtexture = 0;

		if (sides->bottomtexture == -1)
			sides->bottomtexture = 0;
	}

	//V_pages
	PageCount = 5;
	xcount = 0;
	ycount = 0;
	xycount = 0;
	V_PagFlags|= 28;//sprites valloc
	Z_FreeTags(mainzone, PU_CACHE);

	P_InitPicAnims();
}

/*
=================
=
= P_SetupLevel
=
=================
*/

extern boolean restarlevel;

void P_SetupLevel(int map, skill_t skill)//L8002306C()
{
	int		i, memory;
	int		map_file, map_index, map_folder;
	byte	*map_ptr;
	int		lumpnum;
	mobj_t	*mobj;
	char	lumpname[16];

	Z_FreeTags(mainzone, PU_CACHE|PU_LEVSPEC|PU_LEVEL);

	if (!restarlevel)
	{
		V_PagFlags &= 1;
		Z_FreeTags(mainzone, PU_ANIMATION);
	}

	Valloc_Init();
	Z_CheckHeap(mainzone);
	M_ClearRandom();

	//printf("P_SetupLevel(%i,%i)\n", map, skill);

	totalkills = totalitems = totalsecret = 0;
	for (i = 0; i<MAXPLAYERS; i++)
	{
		players[i].killcount = 0;
		players[i].secretcount = 0;
		players[i].itemcount = 0;
		players[i].frags = 0;
	}

	//P_InitThinkers();
	thinkercap.prev = thinkercap.next = &thinkercap;
	mobjhead.next = mobjhead.prev = &mobjhead;

	iquehead = 0;
	iquetail = 0;
	playercounttarget = 0;

	map_index = map-1;
    if (map_index < 0)
        map_folder = map + 6;
    else
        map_folder = map_index;

    map_file = (map_index + ((map_folder / 8) * 16));

	map_ptr = W_OpenMapWad((char *)map_file + _MAP01_WAD /*8*/);//MAP%%.WAD

	/* */
	/* look for a regular (development) map first */
	/* */
	lumpname[0] = 'M';
	lumpname[1] = 'A';
	lumpname[2] = 'P';
	lumpname[3] = '0' + map / 10;
	lumpname[4] = '0' + map % 10;
	lumpname[5] = 0;

	lumpnum = W_MapGetNumForName(lumpname);

    if (lumpnum == -1)
    {
        I_Error("P_SetupLevel: %s not found",lumpname);
    }

	/* note: most of this ordering is important	 */
	P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
	P_LoadVertexes(lumpnum + ML_VERTEXES);
	P_LoadSectors(lumpnum + ML_SECTORS);
	P_LoadSideDefs(lumpnum + ML_SIDEDEFS);
	P_LoadLineDefs(lumpnum + ML_LINEDEFS);
	P_LoadSubsectors(lumpnum + ML_SSECTORS);
	P_LoadNodes(lumpnum + ML_NODES);
	P_LoadSegs(lumpnum + ML_SEGS);
	P_LoadLeafs(lumpnum + ML_LEAFS);
	P_LoadReject(lumpnum + ML_REJECT);

	P_GroupLines();

	/* */
	/* Link deathmatch starts*/
	/* */
	deathmatch_p = deathmatchstarts;

	P_LoadThings(lumpnum + ML_THINGS);

	/* set up world state */
	P_SpawnSpecials();
	Z_Free(map_ptr);

	if (!restarlevel)
	{
		P_LoadBlocks((char*)map_file + _MAPTEX01_IMG/*24*/);//MAPTEX%%.IMG
		P_Init();
		P_LoadBlocks((char*)map_file + _MAPSPR01_IMG/*16*/);//MAPSPR%%.IMG
	}

	memory = Z_FreeMemory(mainzone);
	if (memory < 0xc000)
	{
		Z_DumpHeap(mainzone);
		I_Error("P_SetupLevel: not enough free memory %d", memory);
	}

	/* */
	/* randomly spawn the active players */
	/* */
	if (netgame != gt_single)
	{
		Sync_Data_Transmission();

		for (i = 0; i < MAXPLAYERS; i++)
		{
			mobj = P_SpawnMobj(playerstarts[0].x << 16, playerstarts[0].y << 16, 0, MT_PLAYER);
			players[i].mo = mobj;
			G_DoReborn(i);
			P_RemoveMobj(mobj);
		}
	}
	else
	{
		P_SpawnPlayer(&playerstarts[0]);
	}
}

/*
=================
=
= P_LoadBlocks
= Exclusive Psx Doom
= Loads a list of memory blocks containing WAD lumps from the given file.
=
=================
*/

enum compressFlags {Decode,NoDecode,Error};
typedef struct psxblock_s
{
	int     size;               //*     including the header and possibly tiny fragments */
	void    **user;             //*4    NULL if a free block */
	short   tag;                //*8    purgelevel */
	short   id;                 //*10   should be ZONEID */
	short   lump;               //*12
	short   flags;              //*14   0 = Decode || 1 = NoDecode || >2 = Error
	struct psxblock_s   *next;  //*16
	struct psxblock_s	*prev;  //*20
} psxblock_t;

void P_LoadBlocks(char *filename)//L80023698()
{
    #if 1 /*New Version*/
    // Try and load the memory blocks containing lumps from the given file.
    // Retry this a number of times before giving up, if the initial load attempt fails.
    // Presumably this was to try and recover from a bad CD...

	psxblock_t	header, *base, *block;
	int i, file_num, data_size, size;
	boolean loaded_ok;
	byte *ptr;

	i = 0;
	while (true)
    {
        // If there have been too many failed load attempts then issue an error
        if (i >= 4)
			I_Error("P_LoadBlocks: Data Failure");
        i++;

        // Open the blocks file and get it's size
        file_num = OpenFile(filename);
        data_size = SeekFile(file_num, 0, PSXCD_SEEK_END);

        ptr = (byte *)Z_Malloc(data_size - sizeof(psxblock_t), PU_STATIC, 0);

        base = (psxblock_t *)((byte *)ptr - sizeof(psxblock_t));
        header = *base;

        // Read the file contents
        SeekFile(file_num, 0, PSXCD_SEEK_SET);
        ReadFile(file_num, (byte *)base, data_size);
        CloseFile(file_num);

        // Process all of the memory blocks in the file and make sure they are ok.
        // Once they are verified then we can start linking them in with other memory blocks in the heap:
        loaded_ok = true;

        block = base;
        size = data_size;
        do
        {
            if ((((block->id != ZONEID) ||      // Verify the block has a valid zoneid
                  (block->lump >= numlumps)) || // Verify the lump number is valid
                  (block->flags >= Error)) ||   // Verify the compression mode is valid
                  (block->flags == Decode &&    // Verify the decompressed size is valid
                  (decodedsize((byte *)block + sizeof(psxblock_t)) != lumpinfo[block->lump].size)))
			{
				loaded_ok = false;
				break;
			}

			// Advance onto the next block and make sure we haven't gone past the end of the data
            size -= block->size;
            if (size < 0)
            {
                loaded_ok = false;
				break;
            }

            (byte *)block += block->size;
        } while (size != 0);

        // If everything was loaded ok then link the first block into the heap block list and finish up.
        // Will do the rest of the linking in the loop below:
        if (loaded_ok)
        {
            base->prev = header.prev;
            break;
        }

        // Load failed: restore the old alloc header and free the memory block.
        // Will try again a certain number of times to try and counteract unreliable CDs.
        base = &header;
        Z_Free((byte *)ptr);
    }

    // Once all the blocks are loaded and verified then setup all of the block links.
    // Also mark blocks for lumps that are already loaded as freeable.
    do
    {
        // Check if this lump is already loaded
        if (lumpcache[base->lump].cache == NULL)
        {
            // Lump not loaded, set the lump cache entry to point to the newly loaded data.
            // Also save whether the lump is compressed or not:
            base->user = (void *)&lumpcache[base->lump];
            lumpcache[base->lump].cache = (void *)((byte *)base + sizeof(memblock_t));
            lumpencode[base->lump] = base->flags;
        }
        else
        {
            // If the lump is already loaded then mark this memory block as freeable
            base->user = NULL;
            base->tag = 0;
            base->id = 0;
        }

        // Is this the last loade block in the file?
        // If it is then set the size based on where the next block in the heap starts, otherwise just use the size defined in the file.
        data_size -= base->size;
        if (data_size == 0)
        {
            if (header.next)
                base->size = ((int)header.next - (int)base);

            base->next = header.next;
        }
        else
        {
            base->next = (psxblock_t *)((byte *)base + base->size);
        }

        // Set backlinks for the next block
        if (base->next)
            base->next->prev = base;

        // Move onto the next block loaded
        base = base->next;

    } while (data_size != 0);

    // After all that is done, make sure the heap is valid
    Z_CheckHeap(mainzone);

    #else /*Old Version*/
    psxblock_t	header, *base, *block, *next, *prev;
	int i, file_num, data_size, size;
	boolean error;
	byte *ptr;

	i = 0;
	while (true)
    {
        if (i >= 4)
			I_Error("P_LoadBlocks: Data Failure");
        i++;

        error = false;

        file_num = OpenFile(filename);
        data_size = SeekFile(file_num, 0, PSXCD_SEEK_END);

        ptr = (byte *)Z_Malloc(data_size - sizeof(psxblock_t), PU_STATIC, 0);

        base = (psxblock_t *)((byte *)ptr - sizeof(psxblock_t));
        header.size = base->size;
        header.user = base->user;
        header.tag = base->tag;
        header.id = base->id;
        header.lump = base->lump;
        header.flags = base->flags;
        header.next = base->next;
        header.prev = base->prev;

        prev = base->prev;
        next = base->next;

        SeekFile(file_num, 0, PSXCD_SEEK_SET);
		ReadFile(file_num, (byte *)base, data_size);
		CloseFile(file_num);
        block = base;
        size = data_size;

        do
        {
            if ((((block->id != ZONEID) || (block->lump >= numlumps)) || (block->flags > NoDecode)) ||
                (block->flags == Decode &&
                (decodedsize((byte *)block + sizeof(psxblock_t)) != lumpinfo[block->lump].size)))
			{
            error_:
				error = true;
				break;
			}

            size -= block->size;
            if (size < 0) goto error_;

            (byte *)block = (byte *)block + block->size;
        } while (size != 0);

        if (!error)
        {
            base->prev = prev;
            do
            {
                if (lumpcache[base->lump].cache == NULL)
                {
                    base->user = (void *)&lumpcache[base->lump];
                    lumpcache[base->lump].cache = (void *)((byte *)base + sizeof(memblock_t));
                    lumpencode[base->lump] = base->flags;
                }
                else
                {
                    base->user = NULL;
                    base->tag = 0;
                    base->id = 0;
                }

                data_size -= base->size;
                if (data_size == 0)
                {
                    if (next)
                        *(psxblock_t **)&base->size = (psxblock_t *)((int)next - (int)base);

                    base->next = next;
                }
                else
                {
                    base->next = (psxblock_t *)((int)&base->size + base->size);
                }

                if (base->next)
                    base->next->prev = base;

                base = base->next;

            } while (data_size != 0);
            Z_CheckHeap(mainzone);
            return;
        }

        base->size = header.size;
        base->user = header.user;
        base->tag = header.tag;
        base->id = header.id;
        base->lump = header.lump;
        base->flags = header.flags;
        base->next = header.next;
        base->prev = header.prev;
        Z_Free((byte *)ptr);
    }
    #endif
}

/*
=================
=
= P_CacheSprite
= Exclusive Psx Doom
=
=================
*/

void P_CacheSprite(spritedef_t *sprdef)//L80023970()
{
	int i, j, lump;
	spriteframe_t *sprframe;

	sprframe = sprdef->spriteframes;
	for (i = 0; i < sprdef->numframes; i++)
	{
		for (j = 0; j < 8; j++)
		{
			lump = sprframe->lump[j];

			if ((lump < firstsprite) || (lastsprite < lump))
					I_Error("CacheSprite: invalid sprite lump %d", lump);

			W_CacheLumpNum(lump, PU_ANIMATION, false);

			if (sprframe->rotate == false)
				break;
		}
		sprframe++;
	}
}

/*
=================
=
= P_LoadTextureByWidth
= Exclusive Psx Doom
=
=================
*/

void P_LoadTextureByWidth(int width)//L80023A60()
{
    side_t *sd;
	int i, w;

    if (width < 0)
        width += 15;

    xcount = (xcount-1) + (width/16) & -(width/16);

	sd = sides;
	for (i = 0; i < numsides; i++, sd++)
	{
		if (sd->toptexture != -1)
		{
			if ((textures[sd->toptexture].w == width) && (textures[sd->toptexture].vtpage == 0))
				TextureCache(&textures[sd->toptexture]);
		}

		if (sd->midtexture != -1)
		{
			if ((textures[sd->midtexture].w == width) && (textures[sd->midtexture].vtpage == 0))
				TextureCache(&textures[sd->midtexture]);
		}

		if (sd->bottomtexture != -1)
		{
			if ((textures[sd->bottomtexture].w == width) && (textures[sd->bottomtexture].vtpage == 0))
				TextureCache(&textures[sd->bottomtexture]);
		}
	}
}
