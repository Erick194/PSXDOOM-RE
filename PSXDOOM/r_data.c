/* R_data.c */

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

int			firstflat;              //800780DC|iGp00000ccc
int			lastflat;               //80077F98|iGp00000b88
int			numflats;               //80077FE8|iGp00000bd8
psxobj_t	*texflats;				//80077F4C|puGp00000b3c

int			firsttex;				//80078104|iGp00000cf4
int			lasttex;				//80077FC4|iGp00000bb4
int			numtextures;			//80077FFC|iGp00000bec
psxobj_t	*textures;				//80077F50|puGp00000b40

int			firstsprite;			//80077E40|iGp00000a30
int			lastsprite;				//80077D64|iGp00000954
int			numsprites;				//80077D88|iGp00000978
psxobj_t	*texsprites;			//80077CF0|puGp000008e0

int			*flattranslation;		//80077D8C|puGp00000988 /* for global animation */
int			*texturetranslation;	//80077D98|puGp0000097c /* for global animation */

short		skypalette;				//800780F0 psx doom
psxobj_t	*skytexturep;			//80077E7C

#define MAX_PALETTES 20
extern light_t *lights;			    //80077E94
extern short palette[MAX_PALETTES];	//800A8E68
extern short palettebase;			//80077DA8

void R_InitTextures(void);
void R_InitFlats(void);
void R_InitSprite(void);
void R_InitPalettes(void);
/*============================================================================ */

/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData (void)//L8002B878()
{
    //printf("\nR_InitData\n");
	R_InitPalettes();
	R_InitTextures();
	R_InitFlats();
	R_InitSprite();
}

/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

void R_InitTextures(void)//L8002B8B0()
{
	maptexture_t	*mtexture;
	psxobj_t		*texture;
	byte	        *data;
	int				width, height, lump, i;

	firsttex = W_GetNumForName("T_START") + 1;
	lasttex = W_GetNumForName("T_END") - 1;
	numtextures = (lasttex - firsttex) + 1;

	textures = Z_Malloc(numtextures * (sizeof(psxobj_t) + sizeof(int)), PU_STATIC, 0);
	texturetranslation = (int *) textures + (numtextures * (sizeof(psxobj_t)/4));

	data = (byte *)W_CacheLumpName("TEXTURE1", PU_CACHE, true);
	texture = textures;

	mtexture = (maptexture_t *)data;
	for (lump = firsttex; lump <= lasttex; lump++)
	{
		texture->lump = lump;
		texture->vtpage = 0;

		texture->w = BIGSHORT(mtexture->width);
		texture->h = BIGSHORT(mtexture->height);

		width = texture->w + 15;
		if (width < 0) { width = texture->w + 30; }

		height = texture->h + 15;
		if (height < 0) { height = texture->h + 30; }

		texture->vbasex = width >> 4;
		texture->vbasey = height >> 4;

		mtexture++;
		texture++;
	}
	Z_Free(data);

	/* */
	/* translation table for global animation */
	/* */
	for (i = 0; i<numtextures; i++)
		texturetranslation[i] = i;

	Z_FreeTags(mainzone, PU_CACHE);
}

/*
================
=
= R_InitFlats
=
=================
*/

void R_InitFlats(void)//L8002BA20()
{
	psxobj_t    *flat;
	int			lump, i;

	firstflat = W_GetNumForName("F_START") + 1;
	lastflat = W_GetNumForName("F_END") - 1;
	numflats = (lastflat - firstflat) + 1;

	texflats = Z_Malloc(numflats * (sizeof(psxobj_t) + sizeof(int)), PU_STATIC, 0);
	flattranslation = (int *) texflats + (numflats * (sizeof(psxobj_t)/4));

	flat = texflats;
	for (lump = firstflat; lump <= lastflat; lump++)
	{
		flat->lump = lump;
		flat->w = 64;
		flat->h = 64;
		flat->vtpage = 0;
		flat->vbasex = 4;
		flat->vbasey = 4;
		flat++;
	}

	/* translation table for global animation */
	for (i = 0; i<numflats; i++)
		flattranslation[i] = i;
}

/*
================
=
= R_InitSprite
=
= Exclusive Psx Doom
=================
*/

void R_InitSprite(void)//L8002BB24()
{
	maptexture_t	*msprite;
	psxobj_t		*sprite;
	byte	        *data;
	int				width, height, lump, i;

	firstsprite = W_GetNumForName("S_START") + 1;
	lastsprite = W_GetNumForName("S_END") - 1;
	numsprites = (lastsprite - firstsprite) + 1;

	texsprites = Z_Malloc(numsprites * sizeof(psxobj_t), PU_STATIC, 0);

	data = (byte *)W_CacheLumpName("SPRITE1", PU_CACHE, true);

	sprite = texsprites;
	msprite = (maptexture_t *)data;
	for (lump = firstsprite; lump <= lastsprite; lump++)
	{
		sprite->lump = lump;
		sprite->vtpage = 0;

		sprite->x = BIGSHORT(msprite->leftoffset);
		sprite->y = BIGSHORT(msprite->topoffset);

		sprite->w = BIGSHORT(msprite->width);
		sprite->h = BIGSHORT(msprite->height);

		width = sprite->w + 15;
		if (width < 0) { width = sprite->w + 30; }

		height = sprite->h + 15;
		if (height < 0) { height = sprite->h + 30; }

		sprite->vbasex = width >> 4;
		sprite->vbasey = height >> 4;

		msprite++;
		sprite++;
	}

	Z_Free(data);
	Z_FreeTags(mainzone, PU_CACHE);
}

/*============================================================================= */

#define HIBIT (1<<7)

/*
================
=
= R_TextureNumForName
=
================
*/

int	R_TextureNumForName(char *name)//L8002BC74()
{
#if 0
	int		i;

	i = R_CheckTextureNumForName(name);
	if (i == -1)
		I_Error("R_TextureNumForName: %s not found", name);

	return i;
#endif

	int			i;
	lumpinfo_t	*lump_p;
	char	name8[8];
	int		v1, v2;
	int		c;

	/* make the name into two integers for easy compares */
	*(int *)&name8[0] = 0;
	*(int *)&name8[4] = 0;
	for (i = 0; i<8 && name[i]; i++)
	{
		c = name[i];
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
		name8[i] = c;
	}

	v1 = *(int *)&name8[0];
	v2 = *(int *)&name8[4];

	lump_p = &lumpinfo[firsttex];
	for (i = 0; i<numtextures; i++, lump_p++)
	{
		if ((*(int *)&lump_p->name[4] == v2) &&
           ((*(int *)&lump_p->name[0] & ~HIBIT) == v1))
			return i;
	}

	//I_Error("R_TextureNumForName: %s not found", name);
	return -1;
}

/*
================
=
= R_FlatNumForName
=
================
*/

int	R_FlatNumForName (char *name)//L8002BD38()
{
	int			i;
	lumpinfo_t	*lump_p;
	char	name8[8];
	int		v1,v2;
	int		c;

	/* make the name into two integers for easy compares */
	*(int *)&name8[0] = 0;
	*(int *)&name8[4] = 0;
	for (i=0 ; i<8 && name[i] ; i++)
	{
		c = name[i];
		if (c == 0) break;
		if (c >= 'a' && c <= 'z')
			c -= 'a'-'A';
		name8[i] = c;
	}

	v1 = *(int *)&name8[0];
	v2 = *(int *)&name8[4];

	lump_p = &lumpinfo[firstflat];
	for (i=0 ; i<numflats; i++, lump_p++)
	{
		if ((*(int *)&lump_p->name[4] == v2) &&
           ((*(int *)&lump_p->name[0] & ~HIBIT) == v1))
			return i;
	}
	//I_Error ("R_FlatNumForName: %s not found",name);
	return 0;	/* FIXME -1; */
}

/*
================
=
= R_InitPalettes
=
= Exclusive Psx Doom
================
*/

void R_InitPalettes(void)//L8002BDFC()
{
	int lump, i, x, y;
	byte *paldata;
	RECT rect;
	light_t *light;

	//printf("R_InitPalettes\n");

	lights = (light_t*)W_CacheLumpName("LIGHTS", PU_STATIC, true);
	lights[0].r = 0xff;
	lights[0].g = 0xff;
	lights[0].b = 0xff;

	lump = W_GetNumForName("PLAYPAL");

	paldata = (byte *)W_CacheLumpNum(lump, PU_CACHE, true);

	if ((W_LumpLength(lump) / 512) != MAX_PALETTES)
	{
		I_Error("R_InitPalettes: palette foulup\n");
	}

	rect.w = 256;
	rect.h = 1;
	for (i = 0; i < MAX_PALETTES; i++)
	{
		y = i;
		if (y < 0) { y = i + 0xf; }

		y = y >> 4;
		x = y << 8;

		rect.x = x;
		rect.y = i - (y << 4) + 240;

		LoadImage(&rect, (unsigned long *)paldata);
		palette[i] = getClut(rect.x, rect.y);

		paldata += 512;
	}

	palettebase = palette[0];

	Z_FreeTags(mainzone, PU_CACHE);
}


/*
================
=
= R_CheckTextureNumForName
=
================
*/

#if 0 //dont use on Psx Doom
int	R_CheckTextureNumForName (char *name)
{
	int		i,c;
	char	temp[8];
	int		v1, v2;
	texture_t	*texture_p;

	if (name[0] == '-')		/* no texture marker */
		return 0;

	*(int *)&temp[0] = 0;
	*(int *)&temp[4] = 0;

	for (i=0 ; i<8 && name[i] ; i++)
	{
		c = name[i];
		if (c >= 'a' && c<='z')
			c -= ('a'-'A');
		temp[i] = c;
	}

	v1 = *(int *)temp;
	v2 = *(int *)&temp[4];

	texture_p = textures;

	for (i=0 ; i<numtextures ; i++,texture_p++)
		if (*(int *)&texture_p->name[4] == v2
		&&  (*(int *)texture_p->name) == v1)
			return i;

	return 0;	/* FIXME -1; */
}
#endif
