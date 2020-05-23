#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/*================================================================== */
/* */
/*							BROKEN LIGHT FLASHING */
/* */
/*================================================================== */
/*================================================================== */

/*================================================================== */
/* */
/*	T_FireFlicker */
/* */
/*	Exclusive Psx Doom From PC Doom */
/* */
/*================================================================== */

void T_FireFlicker(fireflicker_t *flick)//L8001AD0C()
{
	int	amount;

	if (--flick->count)
		return;

	amount = (P_Random() & 3) * 16;

	if (flick->sector->lightlevel - amount < flick->minlight)
		flick->sector->lightlevel = flick->minlight;
	else
		flick->sector->lightlevel = flick->maxlight - amount;

	flick->count = 3;
}

/*================================================================== */
/* */
/*	P_SpawnFireFlicker */
/* */
/*	Exclusive Psx Doom From PC Doom */
/* */
/*================================================================== */

void P_SpawnFireFlicker(sector_t *sector)//L8001AD98()
{
	fireflicker_t *flick;

	sector->special = 0; /* nothing special about it during gameplay */

	flick = Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);
	P_AddThinker(&flick->thinker);
	flick->thinker.function = T_FireFlicker;
	flick->sector = sector;
	flick->maxlight = sector->lightlevel;
	flick->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel) + 16;
	flick->count = 3;
}

/*================================================================== */
/* */
/*	T_LightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials */
/*	that spawn thinkers */
/* */
/*================================================================== */
void T_LightFlash (lightflash_t *flash)//L8001AE24()
{
	if (--flash->count)
		return;

	if (flash->sector->lightlevel == flash->maxlight)
	{
		flash-> sector->lightlevel = flash->minlight;
		flash->count = (P_Random()&flash->mintime)+1;
	}
	else
	{
		flash-> sector->lightlevel = flash->maxlight;
		flash->count = (P_Random()&flash->maxtime)+1;
	}
}

/*================================================================== */
/* */
/*	P_SpawnLightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
void P_SpawnLightFlash (sector_t *sector)//L8001AEAC()
{
	lightflash_t	*flash;

	sector->special = 0;		/* nothing special about it during gameplay */

	flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker (&flash->thinker);
	flash->thinker.function = T_LightFlash;
	flash->sector = sector;
	flash->maxlight = sector->lightlevel;

	flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
	flash->maxtime = 64;
	flash->mintime = 7;
	flash->count = (P_Random()&flash->maxtime)+1;
}

/*================================================================== */
/* */
/*							STROBE LIGHT FLASHING */
/* */
/*================================================================== */

/*================================================================== */
/* */
/*	T_StrobeFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
void T_StrobeFlash (strobe_t *flash)//L8001AF54()
{
	if (--flash->count)
		return;

	if (flash->sector->lightlevel == flash->minlight)
	{
		flash-> sector->lightlevel = flash->maxlight;
		flash->count = flash->brighttime;
	}
	else
	{
		flash-> sector->lightlevel = flash->minlight;
		flash->count =flash->darktime;
	}
}

/*================================================================== */
/* */
/*	P_SpawnLightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
//inline
void P_SpawnStrobeFlash (sector_t *sector,int fastOrSlow, int inSync)//L8001AFB8()
{
	strobe_t	*flash;

	flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker (&flash->thinker);
	flash->sector = sector;
	flash->darktime = fastOrSlow;
	flash->brighttime = STROBEBRIGHT;
	flash->thinker.function = T_StrobeFlash;
	flash->maxlight = sector->lightlevel;
	flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

	if (flash->minlight == flash->maxlight)
		flash->minlight = 0;
	sector->special = 0;		/* nothing special about it during gameplay */

	if (!inSync)
		flash->count = (P_Random()&7)+1;
	else
		flash->count = 1;
}

/*================================================================== */
/* */
/*	P_SpawnLightFlash */
/* */
/*	Exclusive Psx Doom */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
void P_SpawnStrobeFlashFast(sector_t *sector)//L8001B08C()
{
	strobe_t	*flash;

	flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker(&flash->thinker);
	flash->sector = sector;
	flash->darktime = 1;
	flash->brighttime = 1;
	flash->thinker.function = T_StrobeFlash;
	flash->minlight = 10;
	flash->maxlight = sector->lightlevel;
	flash->count = 1;

	if (flash->minlight == flash->maxlight)
		flash->minlight = 0;
	sector->special = 0;		/* nothing special about it during gameplay */
}

/*================================================================== */
/* */
/*	Start strobing lights (usually from a trigger) */
/* */
/*================================================================== */
void EV_StartLightStrobing(line_t *line)//L8001B120()
{
	int	secnum;
	sector_t	*sec;

	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		//inline function
		P_SpawnStrobeFlash (sec,SLOWDARK, 0);
	}
}

/*================================================================== */
/* */
/*	TURN LINE'S TAG LIGHTS OFF */
/* */
/*================================================================== */
void EV_TurnTagLightsOff(line_t	*line)//L8001B230()
{
	int			i;
	int			j;
	int			min;
	sector_t	*sector;
	sector_t	*tsec;
	line_t		*templine;

	sector = sectors;
	for (j = 0;j < numsectors; j++, sector++)
    {
		if (sector->tag == line->tag)
		{
			min = sector->lightlevel;
			for (i = 0;i < sector->linecount; i++)
			{
				templine = sector->lines[i];
				tsec = getNextSector(templine,sector);
				if (!tsec)
					continue;
				if (tsec->lightlevel < min)
					min = tsec->lightlevel;
			}
			sector->lightlevel = min;
		}
    }
}

/*================================================================== */
/* */
/*	TURN LINE'S TAG LIGHTS ON */
/* */
/*================================================================== */
void EV_LightTurnOn(line_t *line, int bright)//L8001B32C()
{
	int			i;
	int			j;
	sector_t	*sector;
	sector_t	*temp;
	line_t		*templine;

	sector = sectors;

	for (i = 0; i < numsectors; i++, sector++)
	{
		if (sector->tag == line->tag)
		{
			/* */
			/* bright = 0 means to search for highest */
			/* light level surrounding sector */
			/* */
			if (!bright)
			{
				for (j = 0; j < sector->linecount; j++)
				{
					templine = sector->lines[j];
					temp = getNextSector(templine, sector);
					if (!temp)
						continue;
					if (temp->lightlevel > bright)
						bright = temp->lightlevel;
				}
			}
			sector->lightlevel = bright;
		}
	}
}

/*================================================================== */
/* */
/*	Spawn glowing light */
/* */
/*================================================================== */
void T_Glow(glow_t *g)//L8001B438()
{
	switch(g->direction)
	{
		case -1:		/* DOWN */
			g->sector->lightlevel -= GLOWSPEED;
			if (g->sector->lightlevel < g->minlight)
			{
				g->sector->lightlevel = g->minlight;
				g->direction = 1;
			}
			break;
		case 1:			/* UP */
			g->sector->lightlevel += GLOWSPEED;
			if (g->maxlight < g->sector->lightlevel)
			{
				g->sector->lightlevel = g->maxlight;
				g->direction = -1;
			}
			break;
	}
}

void P_SpawnGlowingLight(sector_t *sector, glowtype_e type)//L8001B4F0()
{
	glow_t *g;

	g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);
	P_AddThinker(&g->thinker);
	g->sector = sector;
	g->thinker.function = T_Glow;

	switch (type)
	{
	case glowtolower:	//special == 8
		g->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
		g->maxlight = sector->lightlevel;
		g->direction = -1;
		break;
	case glowto10:		//special == 200
		g->minlight = 10;
		g->maxlight = sector->lightlevel;
		g->direction = -1;
		break;
	case glowto255:		//special == 201
		g->minlight = sector->lightlevel;
		g->maxlight = 255;
		g->direction = 1;
		break;
	}

	sector->special = 0;
}

