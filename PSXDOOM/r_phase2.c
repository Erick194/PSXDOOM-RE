
//Renderer phase 2

#include "doomdef.h"
#include "r_local.h"

void R_RenderSKY(void)//L8002BF4C
{
	RECT    rect;
	DR_MODE *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT    *skysprt = (SPRT*) getScratchAddr(128);//1F800200

	if (skytexturep->index == -1)
	{
		rect.x = (skytexturep->vramx >> 1) + (skytexturep->vtpage & 15) * 64;
		rect.y = (skytexturep->vramy) + (skytexturep->vtpage & 16) * 16;
		rect.w = 32;
		rect.h = 128;
		LoadImage(&rect, (unsigned long *)(byte*)(lumpcache[skytexturep->lump].cache+8));
		skytexturep->index = TextureCacheIdx;
	}

	setRECT(&rect, skytexturep->vramx, skytexturep->vramy, skytexturep->w, 128);
	SetDrawMode(drawmode, 0, 0, skytexturep->vtpage, &rect);
	W_AddPrim(drawmode);// add to order table

	/* make sprite sky*/
	setSprt(skysprt);
	setShadeTex(skysprt, 1);
	setXY0(skysprt, 0, 0);
	setWH(skysprt, 256, skytexturep->h);
	setUV0(skysprt, skytexturep->vramx - (char)(viewangle >> ANGLETOSKYSHIFT), skytexturep->vramy);
	skysprt->clut = skypalette;

	W_AddPrim(skysprt);// add to order table
}

#define VERTEXES 32
vertex_t newvertexes[VERTEXES];//0x80097acc
int numnewvertexes; //80078044, uGp00000c34

#define MAXLEAF 21
typedef struct
{
	int num_leafs;	    //*
	leaf_t lf[MAXLEAF];	//*4
} vleaf_t;

#define SKIP_NUM_LEAFS (sizeof(int)/4)
#define V_LEAF_SIZE (sizeof(vleaf_t)/4)

void R_FrontZClip(vleaf_t *vlf_in, vleaf_t **vlf_out);
int R_EdgeClipSide(int mode, vleaf_t **vlf);
int R_LeftEdgeClip(vleaf_t *vlf_in, vleaf_t **vlf_out);
int R_RightEdgeClip(vleaf_t *vlf_in, vleaf_t **vlf_out);

/*WALL*/
void R_WallPrep(leaf_t *lf);
void R_Render_Wall(leaf_t *lf, psxobj_t *texture, int topheight, int bottomheight, int top_V, int bottom_V, boolean translucent);

/*PLANE*/
typedef enum PlaneType {FLOOR, CEILING} PlaneType;
void R_PlanePrep(vleaf_t *vlf1, PlaneType mode);
void R_Render_Plane(vleaf_t *vlf1, int zheight, psxobj_t *psxobj);

/*SPRITE*/
void R_Render_Sprite(subsector_t *sub);

/*WEAPON*/
void R_Render_Hud_Weapons(void);


void R_RenderAll(subsector_t *sub)//L8002C5C8
{
	SVECTOR		 v0;
	VECTOR		 v1;
	long		 flag[4];
	leaf_t		 *lf;
	leaf_t		 *leafs_ptr;//0x1F8000AC
	vleaf_t		 *leafs_side[2], *leafs_s;//0x1F8000A8
	vertex_t	 *vrt;
	int			 i, sideclip, num_leafs, side;
	unsigned long *memaddr;

	memaddr = (unsigned long *)getScratchAddr(42);//0x1F8000A8

	leafs_ptr =     (leaf_t  *)(memaddr + SKIP_NUM_LEAFS); //0x1F8000AC    Memory leafs_side[0]->lf
	leafs_side[0] = (vleaf_t *)(memaddr);		           //0x1F8000A8    Memory Base1
	leafs_side[1] = (vleaf_t *)(memaddr + V_LEAF_SIZE);	   //0x1F800154    Memory Base2

	num_leafs = sub->numleafs;

	lf = &leafs[sub->leaf];
	for (i = 0; i < num_leafs; i++)
	{
		vrt = lf->vertex;

        leafs_ptr->seg = lf->seg;
        leafs_ptr->vertex = vrt;

		if (vrt->index != TextureCacheIdx)
		{
			v0.vy = 0;
			v0.vx = (vrt->x - viewx) >> 16;
			v0.vz = (vrt->y - viewy) >> 16;
			RotTrans(&v0, &v1, flag);

            vrt->vx = v1.vx;
            vrt->vy = v1.vz;

            if (v1.vz > 3)
			{
                vrt->yy = 0x800000 / v1.vz;
                vrt->xx = ((0x800000 / v1.vz) * vrt->vx >> 0x10) + 0x80;
			}

			vrt->index = TextureCacheIdx;
		}

        lf++;
        leafs_ptr++;
	}

	leafs_side[0]->num_leafs = num_leafs; //initial count leafs

	numnewvertexes = 0;
	side = 0;

	lf = leafs_side[0]->lf;
	for (i = 0; i < leafs_side[0]->num_leafs; i++)
	{
		if (lf->vertex->vy < 4)
		{
		    if (side == 0)
                leafs_s = leafs_side[1];
            else
                leafs_s = leafs_side[0];

            R_FrontZClip(leafs_side[0], &leafs_s);
            side ^= 1;
            break;
		}
		lf++;
	}

	sideclip = R_EdgeClipSide(0, &leafs_side[side]);
	if (sideclip > -1)
    {
        if (sideclip > 0)
        {
            if (side == 0)
                leafs_s = leafs_side[1];
            else
                leafs_s = leafs_side[0];

            sideclip = R_LeftEdgeClip(leafs_side[side], &leafs_s);
            side ^= 1;
            if (sideclip < 3)
                return;
        }

        sideclip = R_EdgeClipSide(1, &leafs_side[side]);
        if (sideclip > -1)
        {
            if (sideclip > 0)
            {

                if (side == 0)
                    leafs_s = leafs_side[1];
                else
                    leafs_s = leafs_side[0];

                sideclip = R_RightEdgeClip(leafs_side[side], &leafs_s);
                side ^= 1;
                if (sideclip < 3)
                    return;
            }

            //Walls
            leafs_s = leafs_side[side];
            lf = leafs_s->lf;
            lf[leafs_s->num_leafs].vertex = leafs_s->lf[0].vertex;
            lf[leafs_s->num_leafs].seg = leafs_s->lf[0].seg;

            for (i = 0; i < leafs_s->num_leafs; i++)
            {
                if ((lf->seg) && (lf->seg->flag & 1))
                {
                    R_WallPrep(lf);
                }
                lf++;
            }

            if (frontsector->floorheight < viewz)
            {
                R_PlanePrep(leafs_side[side],FLOOR);
            }

            if ((frontsector->ceilingpic != -1) && (viewz < frontsector->ceilingheight))
            {
                R_PlanePrep(leafs_side[side],CEILING);
            }

            //Sprites*/
            R_Render_Sprite(sub);
        }
    }
}

void R_FrontZClip(vleaf_t *vlf_in, vleaf_t **vlf_out)//L8002C95C()
{
	vertex_t    *vrt;
    leaf_t      *lfin, *lfout, *lfnext;
	int num_leafs, leafs_cnt, i;
	int delta, y1, y2;
	vleaf_t *vlf;

    lfin = vlf_in->lf;
    lfout = (*vlf_out)->lf;

	num_leafs = vlf_in->num_leafs;
	leafs_cnt = 0;

	for (i = 0; i < num_leafs; i++)
	{
        if (i == (num_leafs-1))
            lfnext = vlf_in->lf;
		else
			lfnext = (lfin+1);

        y1 = 4 - lfin->vertex->vy;
        y2 = 4 - lfnext->vertex->vy;

        if (y1 == 0)
        {
			lfout->vertex = lfin->vertex;
			lfout->seg = lfin->seg;

        checkpoint:
			leafs_cnt += 1;
            lfout++;
            if (leafs_cnt >= MAXLEAF)
                I_Error("FrontZClip: Point Overflow");
        }
        else
        {
            if (y1 < 0)
            {
                lfout->vertex = lfin->vertex;
                lfout->seg = lfin->seg;

                leafs_cnt += 1;
                lfout++;
                if (leafs_cnt >= MAXLEAF)
                    I_Error("FrontZClip: Point Overflow");
            }

            if ((y2 != 0) && (~y2 >> 31 == y1 >> 31))
            {
                vrt = &newvertexes[numnewvertexes];
                numnewvertexes++;

                if (numnewvertexes >= VERTEXES)
                    I_Error("FrontZClip: exceeded max new vertexes\n");

                delta = (y1 << 16) / (y1 - y2);

                vrt->vy = 4;
                vrt->vx = ((delta * (lfnext->vertex->vx - lfin->vertex->vx)) >> 16) + lfin->vertex->vx;

                vrt->x = ((delta * (lfnext->vertex->x - lfin->vertex->x)) >> 16) + lfin->vertex->x;
                vrt->y = ((delta * (lfnext->vertex->y - lfin->vertex->y)) >> 16) + lfin->vertex->y;

                vrt->yy = (0x800000 / vrt->vy);
                vrt->xx = ((vrt->yy * vrt->vx) >> 16) + 128;

                vrt->index = TextureCacheIdx;

                if ((y1 < 1) || (y2 > -1))
                    lfout->seg = NULL;
                else
                    lfout->seg = lfin->seg;

                lfout->vertex = vrt;

                goto checkpoint;
            }
        }

        lfin++;
	}

	(*vlf_out)->num_leafs = leafs_cnt;
}

int R_EdgeClipSide(int mode, vleaf_t **vlf)//L8002CC38()
{
	leaf_t	*lf;
	int num_leafs, leafcnt, i, side;
	unsigned long *addr;

    lf = (*vlf)->lf;
    addr = (unsigned long *)getScratchAddr(0);	//0x1F800000
    num_leafs = (*vlf)->num_leafs;
    leafcnt = 0;

	if (mode == 0)
	{
		for (i = 0; i < num_leafs; i++)
		{
		    if (lf->vertex->vx >= -lf->vertex->vy)
			{
				addr[0] = 0;
                leafcnt++;
			}
			else
            {
				addr[0] = 1;
                leafcnt--;
			}
            lf++;
            addr++;
		}
	}
	else
	{
		for (i = 0; i < num_leafs; i++)
		{
		    if (lf->vertex->vy >= lf->vertex->vx)
			{
				addr[0] = 0;
                leafcnt++;
			}
			else
            {
				addr[0] = 1;
                leafcnt--;
			}

            lf++;
            addr++;
		}
	}

	addr[0] = *getScratchAddr(0);	//*0x1F800000

    if (leafcnt == num_leafs)
        side = 0;
    else if (leafcnt != -num_leafs)
        side = 1;
    else
        side = -1;

    return side;
}

int R_LeftEdgeClip(vleaf_t *vlf_in, vleaf_t **vlf_out)//L8002CD38()
{
    vertex_t    *vrt;
	leaf_t      *lfin, *lfout, *lfnext;
	int num_leafs, leafs_cnt, i;
	unsigned long *addr;
	int vx1, vx2, vy1, delta;

	lfout = (*vlf_out)->lf;
	lfin = vlf_in->lf;

    leafs_cnt = 0;
	num_leafs = vlf_in->num_leafs;

	addr = (unsigned long *)getScratchAddr(0);	//0x1F800000

	for (i = 0; i < num_leafs; i++)
	{
        if (addr[0] == 0)
        {
            lfout->vertex = lfin->vertex;
            lfout->seg = lfin->seg;
            lfout++;
            leafs_cnt++;
        }

        if (addr[0] == 0)
        {
            if (addr[1] == 1)
            {
            checknext:

                if (i < (num_leafs-1))
                    lfnext = (lfin+1);
                else
                    lfnext = vlf_in->lf;

                vrt = &newvertexes[numnewvertexes];
                numnewvertexes++;

                if (numnewvertexes >= VERTEXES)
                    I_Error("LeftEdgeClip: exceeded max new vertexes\n");

                vx1 = (lfin->vertex->vx + lfin->vertex->vy) << 16;
                vx2 = (lfin->vertex->vx + lfin->vertex->vy) - (lfnext->vertex->vx + lfnext->vertex->vy);
                delta = (vx1 / vx2);

                vy1 = ((delta * (lfnext->vertex->vy - lfin->vertex->vy)) >> 16) + lfin->vertex->vy;
                vrt->vy = vy1;
                vrt->vx = -vy1;

                vrt->x = ((delta * (lfnext->vertex->x - lfin->vertex->x)) >> 16) + lfin->vertex->x;
                vrt->y = ((delta * (lfnext->vertex->y - lfin->vertex->y)) >> 16) + lfin->vertex->y;

                vrt->yy = (0x800000 / vrt->vy);
                vrt->xx = ((vrt->yy * vrt->vx) >> 16) + 128;

                vrt->index = TextureCacheIdx;

                lfout->vertex = vrt;
                lfout->seg = lfin->seg;
                lfout++;

                leafs_cnt++;
                if (leafs_cnt >= MAXLEAF)
                    I_Error("LeftEdgeClip: Point Overflow");
            }
        }
        else if (addr[1] == 0) goto checknext;

        lfin++;
        addr++;
	}

    (*vlf_out)->num_leafs = leafs_cnt;

    return leafs_cnt;
}

int R_RightEdgeClip(vleaf_t *vlf_in, vleaf_t **vlf_out)//L8002CFDC()
{
    vertex_t    *vrt;
	leaf_t      * lfin, *lfout, *lfnext;
	int num_leafs, leafs_cnt, i;
	unsigned long *addr;
	int vx1, vx2, vy1, delta;

	lfout = (*vlf_out)->lf;
	lfin = vlf_in->lf;

    leafs_cnt = 0;
	num_leafs = vlf_in->num_leafs;

	addr = (unsigned long *)getScratchAddr(0);	//0x1F800000

	for (i = 0; i < num_leafs; i++)
	{
        if (addr[0] == 0)
        {
            lfout->vertex = lfin->vertex;
            lfout->seg = lfin->seg;
            lfout++;
            leafs_cnt++;
        }

        if (addr[0] == 0)
        {
            if (addr[1] == 1)
            {
            checknext:

                if (i < (num_leafs-1))
                    lfnext = (lfin+1);
                else
                    lfnext = vlf_in->lf;

                vrt = &newvertexes[numnewvertexes];
                numnewvertexes++;

                if (numnewvertexes >= VERTEXES)
                    I_Error("RightEdgeClip: exceeded max new vertexes\n");

                vx1 = (lfin->vertex->vx - lfin->vertex->vy) << 16;
                vx2 = (lfin->vertex->vx - lfin->vertex->vy) - (lfnext->vertex->vx - lfnext->vertex->vy);
                delta = (vx1 / vx2);

                vy1 = ((delta * (lfnext->vertex->vy - lfin->vertex->vy)) >> 16) + lfin->vertex->vy;
                vrt->vy = vy1;
                vrt->vx = vy1;

                vrt->x = ((delta * (lfnext->vertex->x - lfin->vertex->x)) >> 16) + lfin->vertex->x;
                vrt->y = ((delta * (lfnext->vertex->y - lfin->vertex->y)) >> 16) + lfin->vertex->y;

                vrt->yy = (0x800000 / vrt->vy) + 1;
                vrt->xx = ((vrt->yy * vrt->vx) >> 16) + 128;

                vrt->index = TextureCacheIdx;

                lfout->vertex = vrt;
                lfout->seg = lfin->seg;
                lfout++;

                leafs_cnt++;
                if (leafs_cnt >= MAXLEAF)
                    I_Error("RightEdgeClip: Point Overflow");
            }
        }
        else if (addr[1] == 0) goto checknext;

        lfin++;
        addr++;
	}

    (*vlf_out)->num_leafs = leafs_cnt;

    return leafs_cnt;
}

//--------------------------------------

void R_WallPrep(leaf_t *lf)//L8002D27C
{
	psxobj_t *texture;
	sector_t *back_sector;
	seg_t	*seg;
	line_t   *li;
	fixed_t f_ceilingheight;
	fixed_t f_floorheight;
	fixed_t b_ceilingheight;
	fixed_t b_floorheight;
	fixed_t m_top;
	fixed_t m_bottom;
	fixed_t height;
	fixed_t top_V, bottom_V;

	seg = lf->seg;
	li = seg->linedef;

    f_ceilingheight = (viewz - frontsector->ceilingheight) >> 16;
	f_floorheight = (viewz - frontsector->floorheight) >> 16;

	m_top = f_ceilingheight;//set middle top
	m_bottom = f_floorheight;//set middle bottom

	li->flags |= ML_MAPPED;

	back_sector = seg->backsector;
	if (back_sector)
	{
		b_ceilingheight = (viewz - back_sector->ceilingheight) >> 16;
		b_floorheight = (viewz - back_sector->floorheight) >> 16;

		if (back_sector->ceilingheight < frontsector->ceilingheight)
		{
			if (back_sector->ceilingpic != -1)
			{
				height = b_ceilingheight - f_ceilingheight;

				if (height >= 256)
					height = 255;

				if (li->flags & ML_DONTPEGTOP)
				{
					top_V = (seg->sidedef->rowoffset >> FRACBITS);
					bottom_V = top_V + height;
				}
				else
				{
					bottom_V = (seg->sidedef->rowoffset >> FRACBITS) + 255;
					top_V = bottom_V - height;
				}

				m_top = b_ceilingheight;//clip middle top height

				texture = &textures[texturetranslation[seg->sidedef->toptexture]];
				R_Render_Wall(lf, texture, f_ceilingheight, b_ceilingheight, top_V, bottom_V, false);
			}
		}

		if (frontsector->floorheight < back_sector->floorheight)
		{
			height = f_floorheight - b_floorheight;

			if (height >= 256)
                height = 255;

			if (li->flags & ML_DONTPEGBOTTOM)
			{
				top_V = ((b_floorheight - f_ceilingheight) + (seg->sidedef->rowoffset >> FRACBITS)) & ~128;
			}
			else
			{
				top_V = (seg->sidedef->rowoffset >> FRACBITS);
			}
			bottom_V = top_V + height;

			m_bottom = b_floorheight;//clip middle bottom height

			texture = &textures[texturetranslation[seg->sidedef->bottomtexture]];
			R_Render_Wall(lf, texture, b_floorheight, f_floorheight, top_V, bottom_V, false);
		}

		if (!(li->flags & (ML_MIDTRANSLUCENT| ML_MIDMASKED)))
		{
			return;
		}
	}

	height = m_bottom - m_top;
	if (height >= 256)
        height = 255;

	// handle unpegging (bottom of texture at bottom, or top of texture at top)
	if (li->flags & ML_DONTPEGBOTTOM)
	{
		bottom_V = (seg->sidedef->rowoffset >> FRACBITS) + 255;
		top_V = bottom_V - height;
	}
	else
	{
		top_V = (seg->sidedef->rowoffset >> FRACBITS);
		bottom_V = top_V + height;
	}

	texture = &textures[texturetranslation[seg->sidedef->midtexture]];
	R_Render_Wall(lf, texture, m_top, m_bottom, top_V, bottom_V, (li->flags & ML_MIDTRANSLUCENT));
}

//L8002D554
void R_Render_Wall(leaf_t *lf, psxobj_t *texture, int topheight, int bottomheight, int top_V, int bottom_V, boolean translucent)
{
	RECT rect;
	vertex_t  *vrt;
	seg_t     *seg;
	angle_t   angle;

	int xx, x1, x2;
	int yy, y1, y2;

	int ztop, zbot;
	int ystep1, ystep2, xstep1, xstep2, zstep1;
	int vcos, vsin, vx, vy, vstep1, vstep2, vd, vs1, vs2;

	int intensity, r, g, b;
	int v0, v1, u0, vd1, vd2, upos;
	int deltax, deltay, ypos1, ypos2;

	DR_TWIN *texwindow = (DR_TWIN*) getScratchAddr(128);//1F800200
	POLY_FT3 *wallpoly = (POLY_FT3*) getScratchAddr(128);//1F800200

	vrt = lf->vertex;

	x1 = vrt->xx;
	x2 = (lf[1].vertex)->xx;
	xx = (unsigned)(x2 - x1);

	if (xx > 0)
	{
		if (viewplayer->cheats & CF_X_RAY)
			translucent = true;

		if (texture->index == -1)
		{
			decode(lumpcache[texture->lump].cache, tempbuffer);

			rect.x = (texture->vramx >> 1) + (texture->vtpage & 15) * 64;
			rect.y = texture->vramy + (texture->vtpage & 16) * 16;
			rect.w = 32;
			rect.h = 128;
			LoadImage(&rect, (unsigned long *)(byte*)(tempbuffer + 8));
			texture->index = TextureCacheIdx;
		}

		setRECT(&rect, texture->vramx, texture->vramy, texture->w, texture->h);
		SetTexWindow(texwindow, &rect);
		W_AddPrim(texwindow);// add to order table

        //Setup Wall Primitive Polygon
		setPolyFT3(wallpoly);

		if (translucent)
        {
			setSemiTrans(wallpoly, 1);
        }

		wallpoly->clut = palettebase;
		wallpoly->tpage = texture->vtpage;

        y1 = lf->vertex->yy;
        y2 = (lf[1].vertex)->yy;
        yy = (unsigned)(y2 - y1);

        ztop = (topheight * y2) - (topheight * y1);
        zbot = (bottomheight * y2) - (bottomheight * y1);

        seg = lf->seg;
        angle = seg->angle >> ANGLETOFINESHIFT;

        vsin = finesine[angle];
		vcos = finecosine[angle];

		vx = (seg->v1->x - viewx) >> 8;
		vy = (seg->v1->y - viewy) >> 8;

		angle = ((seg->angle - viewangle) + ANG90) >> ANGLETOFINESHIFT;

		vd = ((vx * (vsin >> 8)) - (vy * (vcos >> 8))) >> 8;

		vstep1 = (vd * (finecosine[angle] >> 8)) >> 4;
		vstep2 = finesine[angle] >> 12;

        ystep1 = (topheight * y1) + (100 * FRACUNIT);
        ystep2 = (bottomheight * y1) + (100 * FRACUNIT);

        vs1 = ((x1 - 0x80) * vstep1) + ((vd * 8) * (finesine[angle] >> 8));
        vs2 = ((x1 - 0x80) * vstep2) - ((finecosine[angle] >> 8) * 8);

        deltax = (seg->xpos1 - x1);
        if (x1 < seg->xpos1)
        {
            ystep1 += deltax * (ztop / xx);
            ystep2 += deltax * (zbot / xx);
            vs1 += deltax * vstep1;
            vs2 += deltax * vstep2;
            x1 = seg->xpos1;
        }

        if (seg->xpos2 < x2)
            x2 = seg->xpos2;

        while(x1 < x2)
		{
            ypos1 = (ystep1 >> 16);
            ypos2 = (ystep2 >> 16);

            if ((ypos1 <= 200) && (ypos2 >= 0))
            {
                upos = 0;
                if (vs2 != 0)
                {
                    upos = ((((seg->offset + FRACUNIT + seg->sidedef->textureoffset) -
                                  ((vx * (vcos >> 8)) + (vy * (vsin >> 8))) >> 8) + (vs1 / vs2)) >> 8);
                }

                v0 = top_V;
				v1 = bottom_V;

                deltay = (ypos2 - ypos1);

                if ((deltay > 509))
                {
                    vd = (100 - ypos1) << 16;

                    vd1 = v0 + (((vd/ deltay) * (v1 - v0)) >> 16);
                    vd2 = ((((100 * FRACUNIT) / deltay) * (v1 - v0)) >> 16);

                    if (ypos1 < 0)
                    {
                        ypos1 = 0;
                        v0 = vd1 - vd2;
                    }
                    if (ypos2 >= 201)
                    {
                        ypos2 = 200;
                        v1 = vd1 + vd2;
                    }
                }

                if (!viewlighting)
                {
                    r = base_r;
					g = base_g;
					b = base_b;
                }
                else
                {
                    intensity = y1 >> 8;

                    if (intensity < 64)
                        intensity = 64;
                    else if (intensity > 160)
                        intensity = 160;

                    r = (intensity * base_r) >> 7;
                    g = (intensity * base_g) >> 7;
                    b = (intensity * base_b) >> 7;

                    if (r > 255) { r = 255; }
                    if (g > 255) { g = 255; }
                    if (b > 255) { b = 255; }
                }

                setRGB0(wallpoly, r, g, b);

                //setXY3(p,_x0,_y0,_x1,_y1,_x2,_y2)
				setXY3(wallpoly,
					x1  , ypos1-1,
					x1+1, ypos2+1,
					x1  , ypos2+1);

                //setUV3(p,_u0,_v0,_u1,_v1,_u2,_v2)
				setUV3(wallpoly,
                        upos, v0,
                        upos, v1,
                        upos, v1);

                W_AddPrim(wallpoly);// add to order table
            }

            x1 += 1;
            y1 += (yy / xx);

            vs1 += vstep1;
            vs2 += vstep2;

            ystep1 += (ztop / xx);
            ystep2 += (zbot / xx);
		}
	}
}

void R_PlanePrep(vleaf_t *vlf1, PlaneType planeType)//L8002E178
{
	RECT rect;
	int pic;
	fixed_t zheight;
	psxobj_t *psxobj;
	byte *data;
	DR_TWIN *texwindow = (DR_TWIN*) getScratchAddr(128);//1F800200

    if (planeType == FLOOR)
    {
        pic = frontsector->floorpic;
    }
    //ceiling
    else
    {
        pic = frontsector->ceilingpic;
    }

    //printf("flattranslation[pic] %d\n",flattranslation[pic]);
	psxobj = &texflats[flattranslation[pic]];

	if (psxobj->index == -1)
	{
		if (lumpencode[psxobj->lump] == 0)
		{
		    data = (byte *)tempbuffer;
			decode(lumpcache[psxobj->lump].cache, tempbuffer);
		}
		else
        {
            data = (byte *)lumpcache[psxobj->lump].cache;
        }

		rect.x = (psxobj->vramx >> 1) + (psxobj->vtpage & 15) * 64;
		rect.y = psxobj->vramy + (psxobj->vtpage & 16) * 16;
		rect.w = 32;
		rect.h = 64;
		LoadImage(&rect, (unsigned long *)(byte*)(data + 8));
		psxobj->index = TextureCacheIdx;
	}

	setRECT(&rect, psxobj->vramx, psxobj->vramy, 64, 64);
	SetTexWindow(texwindow, &rect);
	W_AddPrim(texwindow);// add to order table

    if (planeType == FLOOR)
    {
        zheight = (frontsector->floorheight - viewz);
    }
    //ceiling
    else
    {
        zheight = (frontsector->ceilingheight - viewz);
    }

	//R_Render_Plane
	R_Render_Plane(vlf1, zheight >> FRACBITS, psxobj);
}

typedef struct
{
	unsigned int x_pos[2];	//*, 4
} xpos_t;

xpos_t x_table[200];//0x80097eb0

//L8002E5E4
void R_Render_Plane(vleaf_t *vlf1, int zheight, psxobj_t *psxobj)
{
	leaf_t* lf1;
	int num_leafs, i, j, v_index1, v_index2;
	angle_t angle;
	int yy1, yy2;
	int x1, x2, y1, y2, side, tmp, xpos, ypos, linecnt;
	int vcos, vsin, vx, vy;
	int dx1, dx2, dx3, dx4;
	int vx1, vx2, vy1, vy2;
	int intensity, r, g, b;
	int delta, delta1, delta2, delta3;
	unsigned long *v_xx;
	unsigned long *v_yy;
	xpos_t *xtable;

	#if FIX_FLATSPANS == 1
        int origSpanR;
        int origSpanUR;
        int origSpanVR;
    #endif

	POLY_FT3 *planepoly = (POLY_FT3*) getScratchAddr(128);//1F800200

	lf1 = vlf1->lf;

	v_xx = (unsigned long *)getScratchAddr(160);
	v_yy = (unsigned long *)getScratchAddr(160+MAXLEAF);

	num_leafs = vlf1->num_leafs;

	for (i = 0; i < num_leafs; i++)
	{
		v_xx[i] = lf1->vertex->xx;
		v_yy[i] = 99 - ((zheight * lf1->vertex->yy) >> 16);
		lf1++;
	}

	yy1 = 200;
	yy2 = 0;

	for (i = 0; i < num_leafs; i++)
	{
		if (i == (num_leafs - 1))
			v_index1 = 0;
		else
            v_index1 = (i + 1);

		if (v_yy[v_index1] != v_yy[i])
		{
			if ((int)v_yy[i] < (int)v_yy[v_index1])
			{
			    v_index2 = i;
			    side = 1;
			}
			else
			{
				v_index2 = v_index1;
				v_index1 = i;
				side = 0;
			}

			y1 = v_yy[v_index2];
			y2 = v_yy[v_index1];
			x1 = v_xx[v_index2] << 16;
			x2 = v_xx[v_index1] << 16;

			delta = ((x2-x1)/(y2-y1));

			if (y1 < 0)
			{
				x1 -= (y1 * delta);
				y1 = 0;
			}

			if (y2 > 200)
				y2 = 200;

			if (y1 < yy1)
				yy1 = y1;

			if (yy2 < y2)
				yy2 = y2;

            xtable = &x_table[y1];
			while (y1 < y2)
			{
			    xtable->x_pos[side] = (x1 >> 16);
			    xtable++;
				x1 += delta;
				y1 += 1;
			}
		}
	}

	setPolyFT3(planepoly);
	planepoly->clut = palettebase;
	planepoly->tpage = psxobj->vtpage;

	xtable = &x_table[yy1];
	while (yy1 < yy2)
	{
		x1 = xtable->x_pos[0];
		x2 = xtable->x_pos[1];
		xtable++;

		if (x1 != x2)
		{
            tmp = x1;
            if (x2 < x1)
            {
                x1 = x2;
                x2 = tmp;
            }

			angle = (zheight * yslope[yy1]) >> 16;

			vcos = (angle * viewcos);
			vsin = (angle * viewsin);

			x1 -= 2;
			x2 += 2;

			vy = vsin + viewy;
			vx = vcos + viewx;

			if (vsin < 0)
				vsin += 127;

			vcos = (-viewcos * angle);
			if (vcos < 0)
				vcos += 127;

			dx1 = ((x1 - 128) * (vsin >> 7));
			dx2 = ((x1 - 128) * (vcos >> 7));
			dx3 = ((x2 - 128) * (vsin >> 7));
			dx4 = ((x2 - 128) * (vcos >> 7));

			vx1 = (vx + dx1) >> 16;
			vx2 = (vx + dx3) >> 16;
			vy2 = (vy + dx4) >> 16;
			vy1 = (vy + dx2) >> 16;
			//------------------

			if (vx1 < vx2)
			{
				tmp = (vx1 & -64);
				vx1 -= tmp;
				vx2 -= tmp;
			}
			else
			{
				tmp = (vx2 & -64);
				vx2 -= tmp;
				vx1 -= tmp;
			}

			if (vy1 < vy2)
			{
				tmp = (vy1 & -64);
				vy1 -= tmp;
				vy2 -= tmp;
			}
			else
			{
				tmp = (vy2 & -64);
				vy2 -= tmp;
				vy1 -= tmp;
			}

			if (!viewlighting)
            {
                r = base_r;
				g = base_g;
				b = base_b;
            }
            else
			{
				angle >>= 1;

				intensity = 160;
				intensity -= angle;

				if (intensity < 64)
				{
					intensity = 64;
				}
				else if (intensity >= 161)
				{
					intensity = 160;
				}

				r = (intensity * base_r) >> 7;
				g = (intensity * base_g) >> 7;
				b = (intensity * base_b) >> 7;

				if (r > 255) { r = 255; }
				if (g > 255) { g = 255; }
				if (b > 255) { b = 255; }

			}

			setRGB0(planepoly, r, g, b);

			linecnt = 0;

			if((vx1 < 256) == 0 || (vx2 < 256) == 0 || (vy1 < 256) == 0 || (vy2 < 256) == 0)
            {
                xpos = (vx2 - vx1);
                if (xpos < 0)
                    xpos = -xpos;

                xpos >>= 7;

                ypos = (vy2 - vy1);
                if (ypos < 0)
                    ypos = -ypos;

                ypos >>= 7;

                if (linecnt < ypos)
                    linecnt = ypos;
                else
                    linecnt = xpos;
            }

			if (linecnt == 0)
			{
				//setXY3(p,_x0,_y0,_x1,_y1,_x2,_y2)
				setXY3(planepoly,
					x1, yy1,
					x2, yy1,
					x2, yy1+1);

				//setUV3(p,_u0,_v0,_u1,_v1,_u2,_v2)
				setUV3(planepoly,
					vx1, vy1,
					vx2, vy2,
					vx2, vy2);

				W_AddPrim(planepoly);// add to order table
			}
			else
			{
				linecnt += 1;
				delta1 = ((x2 - x1) / linecnt);
				delta2 = ((vx2 - vx1) / linecnt);
				delta3 = ((vy2 - vy1) / linecnt);

				// From PsyDoom: precision fix to prevent cracks at the right side of the screen on large open maps like 'Tower Of Babel'.
                // Store the coords where the last span should end, and use those for the right side of the last span instead of
                // the somewhat truncated/imprecise stepped coords.
                //
                // TODO: make this tweak configurable according to user prefs.
                #if FIX_FLATSPANS == 1
                    origSpanR = x2;
                    origSpanUR = vx2;
                    origSpanVR = vy2;
                #endif

				for (j = 0; j < linecnt; j++)
				{
					x2 = (x1 + delta1);
					vx2 = (vx1 + delta2);
					vy2 = (vy1 + delta3);

					// From PsyDoom: precision fix to prevent cracks at the right side of the screen on large open maps like 'Tower Of Babel'.
                    // TODO: make this tweak configurable according to user prefs.
                    #if FIX_FLATSPANS == 1
                        if ((j + 1) >= linecnt)
                        {
                            x2 = origSpanR;
                            vx2 = origSpanUR;
                            vy2 = origSpanVR;
                        }
                    #endif

					xpos = 0;
					if ((vx1 < vx2) && (vx2 < 256) == 0)
					{
						xpos = vx1 & -128;
						vx1 -= xpos;
						vx2 -= xpos;
					}
					else if((vx2 < vx1) && (vx1 < 256 == 0))
					{
						xpos = vx2 & -128;
						vx2 -= xpos;
						vx1 -= xpos;
					}

					ypos = 0;
					if ((vy1 < vy2) && (vy2 < 256) == 0)
					{
						ypos = vy1 & -128;
						vy1 -= ypos;
						vy2 -= ypos;
					}
					else if ((vy2 < vy1) && (vy1 < 256) == 0)
					{
						ypos = vy2 & -128;
						vy2 -= ypos;
						vy1 -= ypos;
					}

					//setXY3(p,_x0,_y0,_x1,_y1,_x2,_y2)
					setXY3(planepoly,
						x1, yy1,
						x2, yy1,
						x2, yy1 + 1);

					//setUV3(p,_u0,_v0,_u1,_v1,_u2,_v2)
					setUV3(planepoly,
						vx1, vy1,
						vx2, vy2,
						vx2, vy2);

					W_AddPrim(planepoly);// add to order table

					x1 = x2;
					vx1 = vx2 + xpos;
					vy1 = vy2 + ypos;
				}
			}
		}
		yy1++;
	}
}

// A vissprite_t is a thing that will be drawn during a refresh
typedef struct vissprite_s
{
	int     x;
	int     z;
	mobj_t  *thing;
	struct vissprite_s *next;//800A886C || L80096BA0 on tmp
} vissprite_t;

#define MAXVISSPRITES 64
vissprite_t vissprites[MAXVISSPRITES];//800a8860
vissprite_t visspritehead;//80096b94

#define ASPECT_CORRECT ((FRACUNIT * 4) / 5)

void R_Render_Sprite(subsector_t *sub)//L8002F200
{
	mobj_t  *thing;
	fixed_t tx, tz;
	spritedef_t		*sprdef;
	spriteframe_t	*sprframe;

	vissprite_t *vissprite_p;
	vissprite_t *vissprite_p2;

	angle_t      ang;
	unsigned int rot;
	boolean      flip;
	int          lump;
	int			 blendflags ;

	int x, y, w, h;

	psxobj_t	*spr;
	RECT		area;
	SVECTOR		v0;
	VECTOR		v1;
	long		flag[4];

	int count;

	DR_TWIN *texwindow = (DR_TWIN*) getScratchAddr(128);//1F800200
	POLY_FT4 *spritepoly = (POLY_FT4*) getScratchAddr(128);//1F800200

	visspritehead.next = &visspritehead;
	vissprite_p = vissprites;

	count = 0;
	for (thing = sub->sector->thinglist; thing; thing = thing->snext)
	{
		if (thing->subsector != sub)
			continue;

		// transform origin relative to viewpoint
        v0.vy = 0;
        v0.vx = (short)((unsigned int)(thing->x - viewx) >> 0x10);
        v0.vz = (short)((unsigned int)(thing->y - viewy) >> 0x10);
        RotTrans(&v0,&v1,flag);

		tz = v1.vz;

		// thing is behind view plane?
		if (tz < MINZ)
			continue;

		tx = v1.vx;

		// too far off the side?
		if (tx > (tz << 1) || tx < -(tz << 1))
			continue;

		vissprite_p->x = tx;
		vissprite_p->z = 0x800000 / tz;
		vissprite_p->thing = thing;

		vissprite_p2 = &visspritehead;
		if (vissprite_p2->next != &visspritehead)
		{
			do
            {
				if (vissprite_p->z <= vissprite_p2->next->z)
					break;

				vissprite_p2 = vissprite_p2->next;
			} while (vissprite_p2->next != &visspritehead);
		}

		vissprite_p->next = vissprite_p2->next;
		vissprite_p2->next = vissprite_p;
		vissprite_p++;

		count++;
		if (count >= MAXVISSPRITES)
        {
			break;
		}
	}

	if (count != 0)
	{
		setRECT(&area, 0, 0, 0, 0);
		SetTexWindow(texwindow, &area);
		W_AddPrim(texwindow);// add to order table

		setPolyFT4(spritepoly);
		spritepoly->clut = palettebase;

		vissprite_p = visspritehead.next;
		if (vissprite_p != &visspritehead)
		{
			do
            {
				thing = vissprite_p->thing;

				sprdef = &sprites[thing->sprite];
				sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

				if (sprframe->rotate != 0)
				{
					ang = R_PointToAngle2(viewx, viewy, thing->x, thing->y);
					rot = (ang - thing->angle + (unsigned int)(ANG45 / 2) * 9) >> 29;
					lump = sprframe->lump[rot];
					flip = (boolean)(sprframe->flip[rot]);
				}
				else
				{
					lump = sprframe->lump[0];
					flip = (boolean)(sprframe->flip[0]);
				}

				lump = lump - firstsprite;
				spr = &texsprites[lump];

				TextureCache(spr);

				blendflags = (thing->flags & MF_ALL_BLEND_MASKS) >> 28;
				if (blendflags  != 0)
				{
                    setSemiTrans(spritepoly, true);
				}
				else
				{
					setSemiTrans(spritepoly, false);
				}

				//Blend Translucent
				spritepoly->tpage = spr->vtpage | getTPage(0, blendflags >> 1, 0, 0);

				if (thing->frame & FF_FULLBRIGHT)
				{
					setRGB0(spritepoly, 160, 160, 160);
				}
				else
				{
					setRGB0(spritepoly, base_r, base_g, base_b);
				}

                y = -vissprite_p->z * ((thing->z - viewz >> 16) + spr->y);
				y = (y >> 16) + 100;

                w = ((((spr->w * ASPECT_CORRECT) >> 16) * vissprite_p->z) >> 16);
                h = ((spr->h * vissprite_p->z) >> 16);

                if (flip == 0)
                {
                    //setUV4(p,_u0,_v0,_u1,_v1,_u2,_v2,_u3,_v3)
					setUV4(spritepoly,
						spr->vramx             , spr->vramy,
						spr->vramx + spr->w - 1, spr->vramy,
						spr->vramx             , spr->vramy + spr->h - 1,
						spr->vramx + spr->w - 1, spr->vramy + spr->h - 1);

                    x = (vissprite_p->x - ((spr->x * ASPECT_CORRECT) >> 16)) * vissprite_p->z;
					x = (x >> 16) + 128;
                }
                else
                {
                    //setUV4(p,_u0,_v0,_u1,_v1,_u2,_v2,_u3,_v3)
                    setUV4(spritepoly,
						spr->vramx + spr->w - 1, spr->vramy,
						spr->vramx             , spr->vramy,
						spr->vramx + spr->w - 1, spr->vramy + spr->h - 1,
						spr->vramx             , spr->vramy + spr->h - 1);

                    x = (vissprite_p->x + ((spr->x * ASPECT_CORRECT) >> 16)) * vissprite_p->z;
					x = (x >> 16) - (w - 128);
                }

                //setXY4(p,_x0,_y0,_x1,_y1,_x2,_y2,_x3,_y3)
				setXY4(spritepoly,
					x     , y,
					x + w , y,
					x     , y + h,
					x + w , y + h);

				W_AddPrim(spritepoly);// add to order table

				vissprite_p = vissprite_p->next;
			} while (vissprite_p != &visspritehead);
		}
	}
}

void R_Render_Hud_Weapons(void)//L8002FD04
{
	int				i;
	pspdef_t		*psp;
	state_t			*state;
	spritedef_t		*sprdef;
	spriteframe_t	*sprframe;
	int				lump;
	int				flagtranslucent;

	unsigned short	control_tpage;
	byte			r, g, b;
	psxobj_t		*spr;
	RECT			area;

	DR_MODE         *drawmode = (DR_MODE*) getScratchAddr(128);//1F800200
	SPRT            *weaponpic = (SPRT*) getScratchAddr(128);//1F800200

	psp = &viewplayer->psprites[0];

	for (i = 0; i < NUMPSPRITES; i++, psp++)
	{
		if ((state = psp->state) != 0) /* a null state means not active */
		{
			sprdef = &sprites[state->sprite];
			sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
			lump = sprframe->lump[0];

			lump = lump - firstsprite;
            spr = &texsprites[lump];

			TextureCache(spr);

			/* change default tpage directive */
			setRECT(&area, 0, 0, 0, 0);
			flagtranslucent = (viewplayer->mo->flags & (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3)) != 0;
			control_tpage = spr->vtpage | flagtranslucent << 5;
			SetDrawMode(drawmode, 0, 0, control_tpage, &area);
			W_AddPrim(drawmode);// add to order table

			/* make sprite hud*/
			setSprt(weaponpic);
			if (flagtranslucent)
			{
				setSemiTrans(weaponpic, 1);
			}
			setXY0(weaponpic, ((psp->sx >> 16) + 128) - spr->x, ((psp->sy >> 16) + 199) - spr->y);
			setWH(weaponpic, spr->w, spr->h);
			setUV0(weaponpic, spr->vramx, spr->vramy);
			weaponpic->clut = palettebase;

			if (psp->state->frame & FF_FULLBRIGHT)
			{
				r = (baselight->r * 5) / 8;
				g = (baselight->g * 5) / 8;
				b = (baselight->b * 5) / 8;
				setRGB0(weaponpic, r, g, b);
			}
			else
			{
				setRGB0(weaponpic, base_r, base_g, base_b);
			}

			W_AddPrim(weaponpic);// add to order table
		}
	}
}
