#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/*================================================================== */
/* */
/*							CEILINGS */
/* */
/*================================================================== */
/*================================================================== */

ceiling_t	*activeceilings[MAXCEILINGS];//800A9AFC

/*================================================================== */
/* */
/*	T_MoveCeiling */
/* */
/*================================================================== */
void T_MoveCeiling (ceiling_t *ceiling)//L80014A04()
{
	result_e	res;

	switch(ceiling->direction)
	{
		case 0:		/* IN STASIS */
			break;
		case 1:		/* UP */
			res = T_MovePlane(ceiling->sector,ceiling->speed,
					ceiling->topheight,false,1,ceiling->direction);
			if (!(gametic & 7))
			{
				switch (ceiling->type)
				{
				case silentCrushAndRaise:
					break;
				default:
					S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_stnmov);
					break;
				}
			}

			if (res == pastdest)
            {
				switch(ceiling->type)
				{
					case raiseToHighest:
						P_RemoveActiveCeiling(ceiling);
						break;
					case silentCrushAndRaise:
						S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_pstop);
					case fastCrushAndRaise:
					case crushAndRaise:
						ceiling->direction = -1;
						break;
					default:
						break;
				}
            }
			break;
		case -1:	/* DOWN */
			res = T_MovePlane(ceiling->sector,ceiling->speed,
				ceiling->bottomheight,ceiling->crush,1,ceiling->direction);
			if (!(gametic & 7))
			{
				switch (ceiling->type)
				{
				case silentCrushAndRaise:
					break;
				default:
					S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_stnmov);
					break;
				}
			}
			if (res == pastdest)
            {
				switch(ceiling->type)
				{
					case silentCrushAndRaise:
						S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_pstop);
					case crushAndRaise:
						ceiling->speed = CEILSPEED;
					case fastCrushAndRaise:
						ceiling->direction = 1;
						break;
					case lowerAndCrush:
					case lowerToFloor:
						P_RemoveActiveCeiling(ceiling);
						break;
					default:
						break;
				}
            }
			else if (res == crushed)
            {
				switch(ceiling->type)
				{
					case silentCrushAndRaise:
					case crushAndRaise:
					case lowerAndCrush:
						ceiling->speed = CEILSPEED / 8;
						break;
					default:
						break;
				}
            }
			break;
	}
}

/*================================================================== */
/* */
/*		EV_DoCeiling */
/*		Move a ceiling up/down and all around! */
/* */
/*================================================================== */
int EV_DoCeiling (line_t *line, ceiling_e  type)//L80014C18()
{
	int			secnum,rtn;
	sector_t		*sec;
	ceiling_t		*ceiling;

	secnum = -1;
	rtn = 0;

	/* */
	/*	Reactivate in-stasis ceilings...for certain types. */
	/* */
	switch(type)
	{
		case fastCrushAndRaise:
		case silentCrushAndRaise:
		case crushAndRaise:
			P_ActivateInStasisCeiling(line);
		default:
			break;
	}

	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* */
		/* new door thinker */
		/* */
		rtn = 1;
		ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
		P_AddThinker (&ceiling->thinker);
		sec->specialdata = ceiling;
		ceiling->thinker.function = T_MoveCeiling;
		ceiling->sector = sec;
		ceiling->crush = false;
		switch(type)
		{
			case fastCrushAndRaise:
				ceiling->crush = true;
				ceiling->topheight = sec->ceilingheight;
				ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED * 2;
				break;
			case silentCrushAndRaise:
			case crushAndRaise:
				ceiling->crush = true;
				ceiling->topheight = sec->ceilingheight;
			case lowerAndCrush:
			case lowerToFloor:
				ceiling->bottomheight = sec->floorheight;
				if (type != lowerToFloor)
                    ceiling->bottomheight += 8*FRACUNIT;
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED;
				break;
			case raiseToHighest:
				ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
				ceiling->direction = 1;
				ceiling->speed = CEILSPEED;
				break;
		}

		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
	}
	return rtn;
}

/*================================================================== */
/* */
/*		Add an active ceiling */
/* */
/*================================================================== */
void P_AddActiveCeiling(ceiling_t *c)//L80014DEC()
{
	int		i;
	for (i = 0; i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] == NULL)
		{
			activeceilings[i] = c;
			return;
		}
    }
}

/*================================================================== */
/* */
/*		Remove a ceiling's thinker */
/* */
/*================================================================== */
void P_RemoveActiveCeiling(ceiling_t *c)//L80014E28()
{
	int		i;

	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] == c)
		{
			activeceilings[i]->sector->specialdata = NULL;
			P_RemoveThinker (&activeceilings[i]->thinker);
			activeceilings[i] = NULL;
			break;
		}
    }
}

/*================================================================== */
/* */
/*		Restart a ceiling that's in-stasis */
/* */
/*================================================================== */
void P_ActivateInStasisCeiling(line_t *line)//L80014E90()
{
	int	i;

	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
			(activeceilings[i]->direction == 0))
		{
			activeceilings[i]->direction = activeceilings[i]->olddirection;
			activeceilings[i]->thinker.function = T_MoveCeiling;
		}
    }
}

/*================================================================== */
/* */
/*		EV_CeilingCrushStop */
/*		Stop a ceiling from crushing! */
/* */
/*================================================================== */
int	EV_CeilingCrushStop(line_t	*line)//L80014F04()
{
	int		i;
	int		rtn;

	rtn = 0;
	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
			(activeceilings[i]->direction != 0))
		{
			activeceilings[i]->olddirection = activeceilings[i]->direction;
			activeceilings[i]->thinker.function = NULL;
			activeceilings[i]->direction = 0;		/* in-stasis */
			rtn = 1;
		}
    }

	return rtn;
}
