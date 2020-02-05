/* P_Spec.c */
#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "st_main.h"

/*
===================
=
= P_InitPicAnims
=
===================
*/

animdef_t		animdefs[MAXANIMS] =//0x80067074
{
	{ false, "BLOOD1", "BLOOD3", 3 },
	{ false, "BSLIME01", "BSLIME04", 3 },
	{ false, "CSLIME01", "CSLIME04", 3 },
	{ false, "ENERG01", "ENERG04", 3 },
	{ false, "LAVA01", "LAVA04", 3 },
	{ false, "WATER01", "WATER04", 3 },
	{ false, "SLIME01", "SLIME03", 3 },
	{ true, "BFALL1", "BFALL1", 3 },
	{ true, "ENERGY01", "ENERGY04", 3 },
	{ true, "FIRE01", "FIRE02", 3 },
	{ true, "FLAME01", "FLAME03", 3 },
	{ true, "LAVWAL01", "LAVWAL03", 3 },
	{ true, "SFALL1", "SFALL4", 3 },
	{ true, "SLIM01", "SLIM04", 3 },
	{ true, "TVSNOW01", "TVSNOW03", 1 },
	{ true, "WARN01", "WARN02", 3 }
};

anim_t	/*anims[MAXANIMS],*/ *lastanim;//800861d4, ||80077F8C, pbGp00000b7c

void P_InitPicAnims (void)//L80025EDC()
{
	int		i, lump;

	/* */
	/*	Init animation */
	/* */
	lastanim = anims;
	for (i=0 ; i < MAXANIMS ; i++)
	{
		if (animdefs[i].istexture)
		{
			lastanim->basepic = R_TextureNumForName (animdefs[i].startname);
			lastanim->picnum = R_TextureNumForName(animdefs[i].endname);

			if (textures[lastanim->basepic].vtpage ==  NULL)
                continue;

            for(lump = lastanim->basepic; lump <= lastanim->picnum; lump++)
            {
                W_CacheLumpNum(lump + firsttex, PU_ANIMATION, false);
                textures[lump].vramx = textures[lastanim->basepic].vramx;
                textures[lump].vramy = textures[lastanim->basepic].vramy;
                textures[lump].vtpage = textures[lastanim->basepic].vtpage;
                textures[lump].vptr = textures[lastanim->basepic].vptr;
            }
		}
		else
		{
			lastanim->basepic = R_FlatNumForName (animdefs[i].startname);
			lastanim->picnum = R_FlatNumForName(animdefs[i].endname);

			if (texflats[lastanim->basepic].vtpage ==  NULL)
                continue;

            for (lump = lastanim->basepic; lump <= lastanim->picnum; lump++)
            {
                W_CacheLumpNum(lump + firstflat, PU_ANIMATION, false);

                texflats[lump].vramx = texflats[lastanim->basepic].vramx;
                texflats[lump].vramy = texflats[lastanim->basepic].vramy;
                texflats[lump].vtpage = texflats[lastanim->basepic].vtpage;
                texflats[lump].vptr = texflats[lastanim->basepic].vptr;
            }
		}

		lastanim->istexture = animdefs[i].istexture;
        lastanim->current = lastanim->basepic;
        lastanim->numpics = (lastanim->picnum - lastanim->basepic) + 1;
        lastanim->tics = animdefs[i].speed;
#if 0
/* FIXME */
		if (lastanim->numpics < 2)
			I_Error ("P_InitPicAnims: bad cycle from %s to %s"
			, animdefs[i].startname, animdefs[i].endname);
#endif
		lastanim++;
	}
}


/*
==============================================================================

							UTILITIES

==============================================================================
*/

/* */
/*	Will return a side_t* given the number of the current sector, */
/*		the line number, and the side (0/1) that you want. */
/* */
side_t *getSide(int currentSector,int line, int side)//L800261BC()
{
	return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}

/* */
/*	Will return a sector_t* given the number of the current sector, */
/*		the line number and the side (0/1) that you want. */
/* */
sector_t *getSector(int currentSector,int line,int side)//L80026218()
{
	return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}

/* */
/*	Given the sector number and the line number, will tell you whether */
/*		the line is two-sided or not. */
/* */
int	twoSided(int sector,int line)//L8002627C()
{
	return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

/*================================================================== */
/* */
/*	Return sector_t * of sector next to current. NULL if not two-sided line */
/* */
/*================================================================== */
sector_t *getNextSector(line_t *line,sector_t *sec)//L800262BC()
{
	if (!(line->flags & ML_TWOSIDED))
		return NULL;

	if (line->frontsector == sec)
		return line->backsector;

	return line->frontsector;
}

/*================================================================== */
/* */
/*	FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindLowestFloorSurrounding(sector_t *sec)//L800262EC()
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		floor = sec->floorheight;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight < floor)
			floor = other->floorheight;
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec)//L80026380()
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		floor = -500*FRACUNIT;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight > floor)
			floor = other->floorheight;
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindNextHighestFloor(sector_t *sec,int currentheight)//L80026418()
{
	int			i;
	int			h;
	int			min;
	line_t		*check;
	sector_t	*other;
	fixed_t		height = currentheight;
	fixed_t		heightlist[20];		/* 20 adjoining sectors max! */

	for (i =0,h = 0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight > height)
			heightlist[h++] = other->floorheight;
	}

	/* */
	/* Find lowest height in list */
	/* */
	min = heightlist[0];
	for (i = 1;i < h;i++)
		if (heightlist[i] < min)
			min = heightlist[i];

	return min;
}

/*================================================================== */
/* */
/*	FIND LOWEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindLowestCeilingSurrounding(sector_t *sec)//L800264FC()
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		height = MAXINT;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->ceilingheight < height)
			height = other->ceilingheight;
	}
	return height;
}

/*================================================================== */
/* */
/*	FIND HIGHEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindHighestCeilingSurrounding(sector_t *sec)//L80026598()
{
	int	i;
	line_t	*check;
	sector_t	*other;
	fixed_t	height = 0;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->ceilingheight > height)
			height = other->ceilingheight;
	}
	return height;
}

/*================================================================== */
/* */
/*	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO */
/* */
/*================================================================== */
int	P_FindSectorFromLineTag(line_t	*line,int start)//L80026630()
{
	int	i;

	for (i=start+1;i<numsectors;i++)
		if (sectors[i].tag == line->tag)
			return i;
	return -1;
}

/*================================================================== */
/* */
/*	Find minimum light from an adjacent sector */
/* */
/*================================================================== */
int	P_FindMinSurroundingLight(sector_t *sector,int max)//L80026698()
{
	int			i;
	int			min;
	line_t		*line;
	sector_t	*check;

	min = max;
	for (i=0 ; i < sector->linecount ; i++)
	{
		line = sector->lines[i];
		check = getNextSector(line,sector);
		if (!check)
			continue;
		if (check->lightlevel < min)
			min = check->lightlevel;
	}
	return min;
}

/*
==============================================================================

							EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/



/*
===============================================================================
=
= P_CrossSpecialLine - TRIGGER
=
= Called every time a thing origin is about to cross
= a line with a non 0 special
=
===============================================================================
*/

void P_CrossSpecialLine (line_t *line, mobj_t *thing)//L8002672C()
{
	int			ok;

	/* */
	/*	Triggers that other things can activate */
	/* */
	if (!thing->player)
	{
		if ((thing->type >= MT_TROOPSHOT) && (thing->type < MT_ARACHPLAZ))
			return;

		ok = 0;
		switch(line->special)
		{
			case 39:	/* TELEPORT TRIGGER */
			case 97:	/* TELEPORT RETRIGGER */
			case 125:	/* TELEPORT MONSTERONLY TRIGGER */
			case 126:	/* TELEPORT MONSTERONLY RETRIGGER */
			case 4:		/* RAISE DOOR */
			case 10:	/* PLAT DOWN-WAIT-UP-STAY TRIGGER */
			case 88:	/* PLAT DOWN-WAIT-UP-STAY RETRIGGER */
				ok = 1;
				break;
		}

		if (!ok)
			return;
	}

	switch (line->special)
	{
		/*==================================================== */
		/* TRIGGERS */
		/*==================================================== */
		case 2:			/* Open Door */
			EV_DoDoor(line, Open);
			line->special = 0;
			break;
		case 3:			/* Close Door */
			EV_DoDoor(line, Close);
			line->special = 0;
			break;
		case 4:			/* Raise Door */
			EV_DoDoor(line, Normal);
			line->special = 0;
			break;
		case 5:			/* Raise Floor */
			EV_DoFloor(line, raiseFloor);
			line->special = 0;
			break;
		case 6:			/* Fast Ceiling Crush & Raise */
			EV_DoCeiling(line, fastCrushAndRaise);
			line->special = 0;
			break;
		case 8:			/* Build Stairs */
			EV_BuildStairs(line, build8);
			line->special = 0;
			break;
		case 10:		/* PlatDownWaitUp */
			EV_DoPlat(line, downWaitUpStay, 0);
			line->special = 0;
			break;
		case 12:		/* Light Turn On - brightest near */
			EV_LightTurnOn(line, 0);
			line->special = 0;
			break;
		case 13:		/* Light Turn On 255 */
			EV_LightTurnOn(line, 255);
			line->special = 0;
			break;
		case 16:		/* Close Door 30 */
			EV_DoDoor(line, Close30ThenOpen);
			line->special = 0;
			break;
		case 17:		/* Start Light Strobing */
			EV_StartLightStrobing(line);
			line->special = 0;
			break;
		case 19:		/* Lower Floor */
			EV_DoFloor(line, lowerFloor);
			line->special = 0;
			break;
		case 22:		/* Raise floor to nearest height and change texture */
			EV_DoPlat(line, raiseToNearestAndChange, 0);
			line->special = 0;
			break;
		case 25:		/* Ceiling Crush and Raise */
			EV_DoCeiling(line, crushAndRaise);
			line->special = 0;
			break;
		case 30:		/* Raise floor to shortest texture height on either side of lines */
			EV_DoFloor(line, raiseToTexture);
			line->special = 0;
			break;
		case 35:		/* Lights Very Dark */
			EV_LightTurnOn(line, 35);
			line->special = 0;
			break;
		case 36:		/* Lower Floor (TURBO) */
			EV_DoFloor(line, turboLower);
			line->special = 0;
			break;
		case 37:		/* LowerAndChange */
			EV_DoFloor(line, lowerAndChange);
			line->special = 0;
			break;
		case 38:		/* Lower Floor To Lowest */
			EV_DoFloor(line, lowerFloorToLowest);
			line->special = 0;
			break;
		case 39:		/* TELEPORT! */
			EV_Teleport(line, thing);
			line->special = 0;
			break;
		case 40:		/* RaiseCeilingLowerFloor */
			EV_DoCeiling(line, raiseToHighest);
			EV_DoFloor(line, lowerFloorToLowest);
			line->special = 0;
			break;
		case 44:		/* Ceiling Crush */
			EV_DoCeiling(line, lowerAndCrush);
			line->special = 0;
			break;
		case 52:		/* EXIT! */
			P_ExitLevel();
			line->special = 0;
			break;
		case 53:		/* Perpetual Platform Raise */
			EV_DoPlat(line, perpetualRaise, 0);
			line->special = 0;
			break;
		case 54:		/* Platform Stop */
			EV_StopPlat(line);
			line->special = 0;
			break;
		case 56:		/* Raise Floor Crush */
			EV_DoFloor(line, raiseFloorCrush);
			line->special = 0;
			break;
		case 57:		/* Ceiling Crush Stop */
			EV_CeilingCrushStop(line);
			line->special = 0;
			break;
		case 58:		/* Raise Floor 24 */
			EV_DoFloor(line, raiseFloor24);
			line->special = 0;
			break;
		case 59:		/* Raise Floor 24 And Change */
			EV_DoFloor(line, raiseFloor24AndChange);
			line->special = 0;
			break;
		case 104:		/* Turn lights off in sector(tag) */
			EV_TurnTagLightsOff(line);
			line->special = 0;
			break;
		case 108:		/*  Blazing Door Raise (faster than TURBO!) */
			EV_DoDoor(line, BlazeRaise);
			line->special = 0;
			break;
		case 109:		/*  Blazing Door Open (faster than TURBO!) */
			EV_DoDoor(line, BlazeOpen);
			line->special = 0;
			break;
		case 100:		/* Build Stairs Turbo 16 */
			EV_BuildStairs(line, turbo16);
			line->special = 0;
			break;
		case 110:		/*  Blazing Door Close (faster than TURBO!) */
			EV_DoDoor(line, BlazeClose);
			line->special = 0;
			break;
		case 119:		/*  Raise floor to nearest surr. floor */
			EV_DoFloor(line, raiseFloorToNearest);
			line->special = 0;
			break;
		case 121:		/*  Blazing PlatDownWaitUpStay */
			EV_DoPlat(line, blazeDWUS, 0);
			line->special = 0;
			break;
		case 124:		/* Secret EXIT */
			P_SecretExitLevel(line->tag);
			break;
		case 125:		/* TELEPORT MonsterONLY */
			if (!thing->player)
			{
				EV_Teleport(line, thing);
				line->special = 0;
			}
			break;
		case 141:		/* Silent Ceiling Crush & Raise */
			EV_DoCeiling(line, silentCrushAndRaise);
			line->special = 0;
			break;
		case 142:		/* Play Track Club Doom PSX DOOM EXCLUSIVE */
            S_StopMusic();
            psxcd_play_at_andloop(CD_TRACK[5],CDVolume,0,0,CD_TRACK[5],CDVolume,0,0);
            line->special = 0;
			break;
	/*==================================================== */
	/* RE-DOABLE TRIGGERS */
	/*==================================================== */
		case 72:		/* Ceiling Crush */
			EV_DoCeiling(line, lowerAndCrush);
			break;
		case 73:		/* Ceiling Crush and Raise */
			EV_DoCeiling(line, crushAndRaise);
			break;
		case 74:		/* Ceiling Crush Stop */
			EV_CeilingCrushStop(line);
			break;
		case 75:		/* Close Door */
			EV_DoDoor(line, Close);
			break;
		case 76:		/* Close Door 30 */
			EV_DoDoor(line, Close30ThenOpen);
			break;
		case 77:		/* Fast Ceiling Crush & Raise */
			EV_DoCeiling(line, fastCrushAndRaise);
			break;
		case 79:		/* Lights Very Dark */
			EV_LightTurnOn(line, 35);
			break;
		case 80:		/* Light Turn On - brightest near */
			EV_LightTurnOn(line, 0);
			break;
		case 81:		/* Light Turn On 255 */
			EV_LightTurnOn(line, 255);
			break;
		case 82:		/* Lower Floor To Lowest */
			EV_DoFloor(line, lowerFloorToLowest);
			break;
		case 83:		/* Lower Floor */
			EV_DoFloor(line, lowerFloor);
			break;
		case 84:		/* LowerAndChange */
			EV_DoFloor(line, lowerAndChange);
			break;
		case 86:		/* Open Door */
			EV_DoDoor(line, Open);
			break;
		case 87:		/* Perpetual Platform Raise */
			EV_DoPlat(line, perpetualRaise, 0);
			break;
		case 88:		/* PlatDownWaitUp */
			EV_DoPlat(line, downWaitUpStay, 0);
			break;
		case 89:		/* Platform Stop */
			EV_StopPlat(line);
			break;
		case 90:		/* Raise Door */
			EV_DoDoor(line, Normal);
			break;
		case 91:		/* Raise Floor */
			EV_DoFloor(line, raiseFloor);
			break;
		case 92:		/* Raise Floor 24 */
			EV_DoFloor(line, raiseFloor24);
			break;
		case 93:		/* Raise Floor 24 And Change */
			EV_DoFloor(line, raiseFloor24AndChange);
			break;
		case 94:		/* Raise Floor Crush */
			EV_DoFloor(line, raiseFloorCrush);
			break;
		case 95:		/* Raise floor to nearest height and change texture */
			EV_DoPlat(line, raiseToNearestAndChange, 0);
			break;
		case 96:		/* Raise floor to shortest texture height on either side of lines */
			EV_DoFloor(line, raiseToTexture);
			break;
		case 97:		/* TELEPORT! */
			EV_Teleport(line, thing);
			break;
		case 98:		/* Lower Floor (TURBO) */
			EV_DoFloor(line, turboLower);
			break;
		case 105:		/* Blazing Door Raise (faster than TURBO!) */
			EV_DoDoor(line, BlazeRaise);
			break;
		case 106:		/* Blazing Door Open (faster than TURBO!) */
			EV_DoDoor(line, BlazeOpen);
			break;
		case 107:		/* Blazing Door Close (faster than TURBO!) */
			EV_DoDoor(line, BlazeClose);
			break;
		case 120:		/* Blazing PlatDownWaitUpStay */
			EV_DoPlat(line, blazeDWUS, 0);
			break;
		case 126:		/* TELEPORT MonsterONLY */
			if (!thing->player)
				EV_Teleport(line, thing);
			break;
	}
}

/*
===============================================================================
=
= P_ShootSpecialLine - IMPACT SPECIALS
=
= Called when a thing shoots a special line
=
===============================================================================
*/

void	P_ShootSpecialLine ( mobj_t *thing, line_t *line)//L80026CD8()
{
	int		ok;

	/* */
	/*	Impacts that other things can activate */
	/* */
	if (!thing->player)
	{
		ok = 0;
		switch(line->special)
		{
			case 46:		/* OPEN DOOR IMPACT */
				ok = 1;
				break;
		}
		if (!ok)
			return;
	}

	switch(line->special)
	{
		case 24:		/* RAISE FLOOR */
			if(EV_DoFloor(line,raiseFloor))
				P_ChangeSwitchTexture(line,0);
			break;
		case 46:		/* OPEN DOOR */
			if(EV_DoDoor(line,Open))
				P_ChangeSwitchTexture(line,1);
			break;
		case 47:		/* RAISE FLOOR NEAR AND CHANGE */
			if(EV_DoPlat(line,raiseToNearestAndChange,0))
				P_ChangeSwitchTexture(line,0);
			break;
	}
}


/*
===============================================================================
=
= P_PlayerInSpecialSector
=
= Called every tic frame that the player origin is in a special sector
=
===============================================================================
*/

void P_PlayerInSpecialSector (player_t *player)//L80026DA0()
{
	sector_t	*sector;

	sector = player->mo->subsector->sector;
	if (player->mo->z != sector->floorheight)
		return;		/* not all the way down yet */

	switch (sector->special)
	{
		case 5:		/* HELLSLIME DAMAGE */
			if (!player->powers[pw_ironfeet])
			{
				stbar.specialFace = f_mowdown;

				if ((gamevbls < (int)gametic) && !(gametic & 0xf))
                  P_DamageMobj(player->mo, NULL, NULL, 10);
			}
			break;
		case 7:		/* NUKAGE DAMAGE */
			if (!player->powers[pw_ironfeet])
			{
				stbar.specialFace = f_mowdown;

				if ((gamevbls < (int)gametic) && !(gametic & 0xf))
                  P_DamageMobj(player->mo, NULL, NULL, 5);
			}
			break;
		case 16:	/* SUPER HELLSLIME DAMAGE */
		case 4:		/* STROBE HURT */
			if (!player->powers[pw_ironfeet] || (P_Random() < 5))
			{
				stbar.specialFace = f_mowdown;

                if ((gamevbls < (int)gametic) && !(gametic & 0xf))
                  P_DamageMobj(player->mo, NULL, NULL, 20);
			}
			break;

		case 9:		/* SECRET SECTOR */
			player->secretcount++;
			sector->special = 0;
			break;

		default:
			I_Error ("P_PlayerInSpecialSector: unknown special %i", sector->special);
	};
}


/*
===============================================================================
=
= P_UpdateSpecials
=
= Animate planes, scroll walls, etc
===============================================================================
*/

#define SCROLLLIMIT (FRACUNIT*-129)

int		numlinespecials;//0x80077D7C, iGp0000096c
//line_t	*linespeciallist[MAXLINEANIMS];//0x800973a4

firesky_t spreadfire;   //80077964, pcGp00000554

void P_UpdateSpecials (void)//L80026F60()
{
	anim_t		*anim;
	int			i;
	line_t		*line;
	psxobj_t	*animtex;


	/* */
	/*	ANIMATE FLATS AND TEXTURES GLOBALY */
	/* */
	for (anim = anims ; anim < lastanim ; anim++)
	{
		if (!(gametic & anim->tics))//leveltime
		{
			if (!anim->istexture)
			{
				anim->current++;
				if (anim->picnum < anim->current)
					anim->current = anim->basepic;

				flattranslation[anim->basepic] = anim->current;
				animtex = &texflats[anim->current];
			}
			else
			{
				anim->current++;
				if (anim->picnum < anim->current)
					anim->current = anim->basepic;

				texturetranslation[anim->basepic] = anim->current;
				animtex = &textures[anim->current];
			}
			animtex->index = -1;
		}
	}

	/* */
	/*	ANIMATE LINE SPECIALS */
	/* */
	for (i = 0; i < numlinespecials; i++)
	{
		line = linespeciallist[i];
		switch(line->special)
		{
			case 200:	/* EFFECT SCROLL LEFT*/
				sides[line->sidenum[0]].textureoffset += FRACUNIT;
				sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
				break;
			case 201:	/* EFFECT SCROLL RIGHT*/
				sides[line->sidenum[0]].textureoffset -= FRACUNIT;
				sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
				break;
			case 202:	/* EFFECT SCROLL UP*/
				sides[line->sidenum[0]].rowoffset += FRACUNIT;
				sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
				break;
			case 203:	/* EFFECT SCROLL DOWN*/
				sides[line->sidenum[0]].rowoffset -= FRACUNIT;
				sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
				break;
		}
	}

	/* */
	/*	DO BUTTONS */
	/* */
	for (i = 0; i < MAXBUTTONS; i++)
	{
		if (buttonlist[i].btimer > 0)
		{
			buttonlist[i].btimer -= vblsinframe[consoleplayer];

			if (buttonlist[i].btimer <= 0)
			{
				switch (buttonlist[i].where)
				{
				case top:
					sides[buttonlist[i].line->sidenum[0]].toptexture = buttonlist[i].btexture;
					break;
				case middle:
					sides[buttonlist[i].line->sidenum[0]].midtexture = buttonlist[i].btexture;
					break;
				case bottom:
					sides[buttonlist[i].line->sidenum[0]].bottomtexture = buttonlist[i].btexture;
					break;
				}
				S_StartSound((mobj_t *)buttonlist[i].soundorg, sfx_swtchn);
				D_memset(&buttonlist[i], 0, sizeof(button_t));
			}
		}
	}

	//Update spread fire
	if (rendersky && spreadfire)
        spreadfire(skytexturep);
}

/*============================================================ */
/* */
/*	Special Stuff that can't be categorized */
/* */
/*============================================================ */
int EV_DoDonut(line_t *line)//L800273F4()
{
	sector_t	*s1;
	sector_t	*s2;
	sector_t	*s3;
	int			secnum;
	int			rtn;
	int			i;
	floormove_t		*floor;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		s1 = &sectors[secnum];

		/*	ALREADY MOVING?  IF SO, KEEP GOING... */
		if (s1->specialdata)
			continue;

		rtn = 1;
		s2 = getNextSector(s1->lines[0],s1);
		for (i = 0;i < s2->linecount;i++)
		{
			if (//(!s2->lines[i]->flags & ML_TWOSIDED) ||
				(s2->lines[i]->backsector == s1))
				continue;
			s3 = s2->lines[i]->backsector;

			/* */
			/*	Spawn rising slime */
			/* */
			floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker (&floor->thinker);
			s2->specialdata = floor;
			floor->thinker.function = T_MoveFloor;
			floor->type = donutRaise;
			floor->crush = false;
			floor->direction = 1;
			floor->sector = s2;
			floor->speed = FLOORSPEED / 2;
			floor->texture = s3->floorpic;
			floor->newspecial = 0;
			floor->floordestheight = s3->floorheight;

			/* */
			/*	Spawn lowering donut-hole */
			/* */
			floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker (&floor->thinker);
			s1->specialdata = floor;
			floor->thinker.function = T_MoveFloor;
			floor->type = lowerFloor;
			floor->crush = false;
			floor->direction = -1;
			floor->sector = s1;
			floor->speed = FLOORSPEED / 2;
			floor->floordestheight = s3->floorheight;
			break;
		}
	}
	return rtn;
}

/*
==============================================================================

							   EXIT DELAY

==============================================================================
*/

/*
================================================================================
= P_SpawnDelayTimer
=
= Exclusive Psx Doom
=
===============================================================================
*/

void P_SpawnDelayTimer(int tics, void(*action)())//L80027640()
{
	delay_t *timer;

	timer = Z_Malloc(sizeof(*timer), PU_LEVSPEC, 0);
	P_AddThinker(&timer->thinker);
	timer->thinker.function = T_CountdownTimer;
	timer->tics = tics;
	timer->finishfunc = action;
}

/*
================================================================================
= T_CountdownTimer
=
= Exclusive Psx Doom
=
===============================================================================
*/

void T_CountdownTimer(delay_t *timer)//L800276B0()
{
	if (!timer->tics--)
	{
		timer->finishfunc();
		P_RemoveThinker(&timer->thinker);
		return;
	}
}

/*
================================================================================
= P_ExitLevel
=
= Exclusive Psx Doom
=
===============================================================================
*/

void P_ExitLevel(void)//L80027700()
{
	nextmap = gamemap + 1;
	P_SpawnDelayTimer(4, G_CompleteLevel);
}

/*
================================================================================
= P_SecretExitLevel
=
= Exclusive Psx Doom
=
===============================================================================
*/

void P_SecretExitLevel(int map)//L80027778()
{
	nextmap = map;
	P_SpawnDelayTimer(4, G_CompleteLevel);
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

extern int enemyspecial;   //80077DB4

void P_SpawnSpecials (void)//L800277E4()
{
	sector_t	*sector;
	int		i;

	/* */
	/*	Init special SECTORs */
	/* */
	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		if (!sector->special)
			continue;

		switch (sector->special)
		{
		case 1:		/* FLICKERING LIGHTS */
			P_SpawnLightFlash(sector);
			break;
		case 2:		/* STROBE FAST */
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			break;
		case 3:		/* STROBE SLOW */
			P_SpawnStrobeFlash(sector, SLOWDARK, 0);
			break;
		case 8:		/* GLOWING LIGHT */
			P_SpawnGlowingLight(sector, glowtolower);
			break;
		case 9:		/* SECRET SECTOR */
			totalsecret++;
			break;
		case 10:	/* DOOR CLOSE IN 30 SECONDS */
			P_SpawnDoorCloseIn30(sector);
			break;
		case 12:	/* SYNC STROBE SLOW */
			P_SpawnStrobeFlash(sector, SLOWDARK, 1);
			break;
		case 13:	/* SYNC STROBE FAST */
			P_SpawnStrobeFlash(sector, FASTDARK, 1);
			break;
		case 14:	/* DOOR RAISE IN 5 MINUTES */
			P_SpawnDoorRaiseIn5Mins(sector, i);
			break;
		case 17:
			P_SpawnFireFlicker(sector);
			break;
		case 200:
			P_SpawnGlowingLight(sector, glowto10);
			break;
		case 201:
			P_SpawnGlowingLight(sector, glowto255);
			break;
		case 202:
			P_SpawnStrobeFlashFast(sector);
			break;
		case 204:
			P_SpawnStrobeFlash(sector, TURBODARK, 0);
			break;
		}
	}

	/* */
	/*	Init line EFFECTs */
	/* */
	numlinespecials = 0;
	for (i = 0;i < numlines; i++)
    {
		switch(lines[i].special)
		{
			//case 48:	/* EFFECT FIRSTCOL SCROLL+ */
			case 200:
			case 201:
			case 202:
			case 203:
			    if(numlinespecials < MAXLINEANIMS)
                {
                    linespeciallist[numlinespecials] = &lines[i];
                    numlinespecials++;
                }
				break;
		}
    }

	/* */
	/* Init Enemy Special Death Flags */
	/* Exclusive Psx Doom */
	/* */
	enemyspecial = 0;
	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		switch (sector->tag)
		{
		case 666:
			enemyspecial |= 1;	// "Mancubus"
			break;
		case 667:
			enemyspecial |= 2;	// "Arachnotron",
			break;
		case 668:
			enemyspecial |= 4;	// "SpiderMastermind",
			break;
		case 669:
			enemyspecial |= 8;	// "Hell Knight",
			break;
		case 670:
			enemyspecial |= 16;	// "Cyberdemon",
			break;
		case 671:
			enemyspecial |= 32;	// "Baron of Hell",
			break;
		default:
			break;
		}
	}

	/* */
	/*	Init other misc stuff */
	/* */
	D_memset(activeceilings, 0, MAXCEILINGS * sizeof(ceiling_t));
	D_memset(activeplats, 0, MAXPLATS * sizeof(plat_t));
	D_memset(buttonlist, 0, MAXBUTTONS * sizeof(button_t));
}


/*
================================================================================
= P_FireSky
=
= Exclusive Psx Doom Fire Sequence
=
===============================================================================
*/

int frndindex = 0;//*(r28 + 1368) 80077968

void P_FireSky(psxobj_t *psxobj)//L80027B90()
{
    byte *src, *srcoffset;
	byte randIdx1, randIdx2;
	int width, height;
	int pixel, counter;

	src = (byte *)(lumpcache[psxobj->lump].cache) + (FIRESKY_WIDTH + 8);//8 = skip header texture data

	width = 0;
	do // width
	{
		height = 1;
		counter = width + 1;

		srcoffset = src + width;
		do // height
		{
			pixel = *(byte*)srcoffset;
			if (pixel != 0)
			{
				randIdx1 = rndtable[(frndindex & 0xff)];
				randIdx2 = rndtable[(frndindex + 1) & 0xff];
				frndindex = ((frndindex + 2) & 0xff);

				*(byte*)((counter - (randIdx1 & 3) & (FIRESKY_WIDTH - 1)) + src - FIRESKY_WIDTH) = pixel - ((randIdx2 & 1));
			}
			else
			{
				*(byte*)(srcoffset - FIRESKY_WIDTH) = 0;
			}

			srcoffset += FIRESKY_WIDTH;
			src += FIRESKY_WIDTH;
			height++;
		} while (height < FIRESKY_HEIGHT);

		src -= ((FIRESKY_WIDTH*FIRESKY_HEIGHT) - FIRESKY_WIDTH);
		width++;
	} while (width < FIRESKY_WIDTH);

	psxobj->index = -1;
}
