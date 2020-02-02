/* W_wad.c */

#include "doomdef.h"
#include "r_local.h"

/*=============== */
/*   TYPES */
/*=============== */


typedef struct
{
	char		identification[4];		/* should be IWAD */
	int			numlumps;
	int			infotableofs;
} wadinfo_t;


//byte		*wadfileptr;
int			wadfilenum;	//80078078

/*============= */
/* GLOBALS */
/*============= */

lumpinfo_t	*lumpinfo;				//80077FEC /* points directly to rom image */
int			numlumps;				//80078014
lumpcache_t	*lumpcache;				//80078060
byte		*lumpencode;			//80078114 psxdoom
boolean		disableload = false;	//80077A14 psxdoom
byte		*mapfileptr;			//80077D44 psxdoom
int			mapnumlumps;			//80077F44 psxdoom
lumpinfo_t  *maplump;				//80077EF0 psxdoom

#define WINDOW_SIZE	4096
#define LOOKAHEAD_SIZE	16

#define LENSHIFT 4		/* this must be log2(LOOKAHEAD_SIZE) */

unsigned char *decomp_input;
unsigned char *decomp_output;
extern int decomp_start;

/*
============================================================================

						LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_Init
=
====================
*/

void W_Init (void)//L80031264()
{
	byte *wadfileptr;
	int infotableofs, i;

	ClearFiles();
	wadfilenum = OpenFile((char*)_PSXDOOM_WAD);

	ReadFile(wadfilenum, wadfileptr, sizeof(wadinfo_t));

	if (D_strncasecmp(((wadinfo_t*)wadfileptr)->identification, "IWAD", 4))
		I_Error("W_Init: invalid main IWAD id");

	numlumps = BIGLONG(((wadinfo_t*)wadfileptr)->numlumps);
	lumpinfo = (lumpinfo_t *) Z_Malloc(numlumps * sizeof(lumpinfo_t), PU_STATIC, 0);

	infotableofs = BIGLONG(((wadinfo_t*)wadfileptr)->infotableofs);

	SeekFile(wadfilenum, infotableofs, PSXCD_SEEK_SET);
	ReadFile(wadfilenum, lumpinfo, numlumps * sizeof(lumpinfo_t));

	lumpcache = (lumpcache_t *) Z_Malloc(numlumps * sizeof(lumpcache_t), PU_STATIC, 0);
	lumpencode = (byte *) Z_Malloc(numlumps, PU_STATIC, 0);

	D_memset(lumpcache, NULL, numlumps * sizeof(lumpcache_t));
	D_memset(lumpencode, 0, numlumps);
}

/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/

int	W_CheckNumForName (char *name)//L80031374()
{
	char	name8[12];
	int		v1,v2;
	lumpinfo_t	*lump_p;

	/* make the name into two integers for easy compares */
	D_memset (name8,0,sizeof(name8));
	D_strncpy (name8,name,8);
	name8[8] = 0;			/* in case the name was a full 8 chars */
	D_strupr(name8);		/* case insensitive */

	v1 = *(int *)name8;
	v2 = *(int *)&name8[4];


	/* scan backwards so patch lump files take precedence */

	lump_p = lumpinfo + numlumps;

	/* used for stripping out the hi bit of the first character of the */
	/* name of the lump */

#define HIBIT (1<<7)

	while (lump_p-- != lumpinfo)
		if (*(int *)&lump_p->name[4] == v2
		&&  (*(int *)lump_p->name & ~HIBIT) == v1)
			return lump_p - lumpinfo;


	return -1;
}

/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int	W_GetNumForName (char *name)//L80031428()
{
	int	i;

	i = W_CheckNumForName (name);
	if (i != -1)
		return i;

	I_Error ("W_GetNumForName: %s not found!",name);
	return -1;
}

/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength (int lump)//L80031518()
{
	if (lump >= numlumps)
		I_Error ("W_LumpLength: %i >= numlumps",lump);
	return BIGLONG(lumpinfo[lump].size);
}

/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/
//inline
void W_ReadLump (int lump, void *dest, boolean decodelump)//L80031568()
{
	/*byte *input;
	lumpinfo_t	*l;
	int lumpsize;

	if (lump >= numlumps)
		I_Error ("W_ReadLump: %i >= numlumps",lump);

	l = lumpinfo + lump;
	if(decodelump)
	{
		if ((l->name[0] & 0x80))
		{
			input = Z_Alloc(BIGLONG(l->size), PU_STATIC, 0);

			lumpsize = BIGLONG(lumpinfo[lump + 1].filepos) - BIGLONG(l->filepos);
			SeekFile(wadfilenum, BIGLONG(l->filepos), PSXCD_SEEK_SET);
			ReadFile(wadfilenum, input, lumpsize);

			decode((unsigned char *)input, (unsigned char *)dest);

			Z_Free(input);
			return;
		}
	}

	lumpsize = BIGLONG(lumpinfo[lump + 1].filepos) - BIGLONG(l->filepos);
	SeekFile(wadfilenum, BIGLONG(l->filepos), PSXCD_SEEK_SET);
	ReadFile(wadfilenum, dest, lumpsize);*/

	lumpinfo_t	*l;
	byte		*input;
	int			lumpsize;

	if (numlumps <= lump)
		I_Error("W_ReadLump: %i >= numlumps", lump);

	l = &lumpinfo[lump];
	if ((decodelump == 0) ||/*&&*/ ((l->name[0] & 0x80) == 0))
	{
		lumpsize = BIGLONG(lumpinfo[lump + 1].filepos) - BIGLONG(l->filepos);
		//printf("no comp SeekFile[%d] lumpsize[%d]", BIGLONG(l->filepos),lumpsize);
		SeekFile(wadfilenum, BIGLONG(l->filepos), PSXCD_SEEK_SET);
		ReadFile(wadfilenum, dest, lumpsize);
	}
	else /* compressed */
	{
		input = Z_Alloc(BIGLONG(l->size), PU_STATIC, 0);

		lumpsize = BIGLONG(lumpinfo[lump + 1].filepos) - BIGLONG(l->filepos);
		//printf("comp SeekFile[%d] lumpsize[%d]", BIGLONG(l->filepos),lumpsize);
		SeekFile(wadfilenum, BIGLONG(l->filepos), PSXCD_SEEK_SET);
		ReadFile(wadfilenum, input, lumpsize);

		decode((unsigned char *)input, (unsigned char *)dest);

		Z_Free2(mainzone, input);
	}
}

/*
====================
=
= W_CacheLumpNum
=
====================
*/

void	*W_CacheLumpNum (int lump, int tag, boolean decodelump)//L8003167C()
{
    int lumpsize;
    lumpcache_t *lc;

	if (numlumps <= (unsigned)lump)
		I_Error("W_CacheLumpNum: %i >= numlumps", lump);

	lc = &lumpcache[lump];
	if (!lc->cache)
	{	/* read the lump in */

		//if (disableload != 0)
			//I_Error("cache miss on lump %i", lump);

		if (decodelump == 0)
			lumpsize = BIGLONG(lumpinfo[lump + 1].filepos) - BIGLONG(lumpinfo[lump].filepos);
		else
			lumpsize = BIGLONG(lumpinfo[lump].size);

		Z_Malloc(lumpsize, tag, &lc->cache);

		W_ReadLump(lump, lc->cache, decodelump);

		if ((lumpinfo[lump].name[0] & 0x80) == 0)
			lumpencode[lump] = 1;
		else
			lumpencode[lump] = (byte)decodelump;
	}

	return lc->cache;
}

/*
====================
=
= W_CacheLumpName
=
====================
*/

void	*W_CacheLumpName (char *name, int tag, boolean decodelump)//L800318B4()
{
	return W_CacheLumpNum (W_GetNumForName(name), tag, decodelump);
}


/*
============================================================================

MAP LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_OpenMapWad
=
= Exclusive Psx Doom
====================
*/

byte *W_OpenMapWad(char *mapname)//L800319D4()
{
	int mapfile, mapsize, infotableofs;

	mapfile = OpenFile(mapname);
	mapsize = SeekFile(mapfile, 0, PSXCD_SEEK_END);

	mapfileptr = Z_Alloc(mapsize, PU_STATIC, 0);

	SeekFile(mapfile, 0, PSXCD_SEEK_SET);
	ReadFile(mapfile, mapfileptr, mapsize);
	CloseFile(mapfile);

	if (D_strncasecmp(((wadinfo_t*)mapfileptr)->identification, "IWAD", 4))
		I_Error("W_OpenMapWad: invalid map IWAD id");

	mapnumlumps = BIGLONG(((wadinfo_t*)mapfileptr)->numlumps);
	infotableofs = BIGLONG(((wadinfo_t*)mapfileptr)->infotableofs);

	maplump = (lumpinfo_t*)(mapfileptr + infotableofs);

	return mapfileptr;
}

/*
====================
=
= W_MapLumpLength
=
= Exclusive Psx Doom
====================
*/

int W_MapLumpLength(int lump)//L80031AA4()
{
	if (lump >= mapnumlumps)
		I_Error("W_MapLumpLength: %i out of range", lump);

	return maplump[lump].size;
}

/*
====================
=
= W_MapGetNumForName
=
= Exclusive Psx Doom
====================
*/

int W_MapGetNumForName(char *name)//L80031AF4()
{
	char	name8[16];
	int		v1, v2;
	lumpinfo_t	*lump_p;

	/* make the name into two integers for easy compares */
	D_memset(name8, 0, sizeof(name8));
	D_strncpy(name8, name, 8);
	name8[8] = 0;			/* in case the name was a full 8 chars */
	D_strupr(name8);		/* case insensitive */

	v1 = *(int *)name8;
	v2 = *(int *)&name8[4];

	/* scan backwards so patch lump files take precedence */

	lump_p = &maplump[mapnumlumps];

	/* used for stripping out the hi bit of the first character of the */
	/* name of the lump */

#define HIBIT (1<<7)

	while (lump_p-- != maplump)
		if (*(int *)&lump_p->name[4] == v2
			&& (*(int *)lump_p->name & ~HIBIT) == v1)
			return lump_p - maplump;

	return -1;
}

/*
====================
=
= W_ReadMapLump
=
= Exclusive Psx Doom
====================
*/

void W_ReadMapLump(int lump, void *dest, int decodelump)//L80031BB0()
{
	lumpinfo_t *l;
	int lumpsize;

	if (lump >= mapnumlumps)
		I_Error("W_ReadMapLump: lump %d out of range", mapnumlumps);

	l = &maplump[lump];

	if (decodelump)
	{
		if (l->name[0] & 0x80) // compressed
		{
			decode((unsigned char *)(mapfileptr + BIGLONG(l->filepos)), (unsigned char *)dest);
			return;
		}
	}

	lumpsize = BIGLONG((l + 1)->filepos) - BIGLONG(l->filepos);
	D_memcpy(dest, mapfileptr + BIGLONG(l->filepos), lumpsize);
}


/*
============================================================================

DECODE BASED ROUTINES

============================================================================
*/

/*
== == == == == == == == == ==
=
= decode
=
= Exclusive Psx Doom from Jaguar Doom
== == == == == == == == == ==
*/

void decode(unsigned char *input, unsigned char *output)//L80031C60()
{
	int getidbyte = 0;
	int len;
	int pos;
	int i;
	unsigned char *source;
	int idbyte = 0;

	while (1)
	{

		/* get a new idbyte if necessary */
		if (!getidbyte) idbyte = *input++;
		getidbyte = (getidbyte + 1) & 7;

		if (idbyte & 1)
		{
			/* decompress */
			pos = *input++ << LENSHIFT;
			pos = pos | (*input >> LENSHIFT);
			source = output - pos - 1;
			len = (*input++ & 0xf) + 1;
			if (len == 1) break;
			for (i = 0; i<len; i++)
				*output++ = *source++;
		}
		else {
			*output++ = *input++;
		}

		idbyte = idbyte >> 1;
	}
}

/*
== == == == == == == == == ==
=
= decodedsize
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

int decodedsize(unsigned char *input)//L80031D18()
{
	int getidbyte = 0;
	int len;
	byte idbyte;
	int accum = 0;

	while (1)
	{
		/* get a new idbyte if necessary */
		if (!getidbyte) idbyte = *input++;
		getidbyte = (getidbyte + 1) & 7;

		if (idbyte & 1)
		{
			/* decompress */
			input++;
			len = (*input++ & 0xf) + 1;
			if (len == 1) break;
			accum += len;
		}
		else
		{
			accum++;
			input++;
		}

		idbyte = idbyte >> 1;
	}

	return accum;
}
