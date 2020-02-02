#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

/*================================================================== */
/*================================================================== */
/* */
/*							VERTICAL DOORS */
/* */
/*================================================================== */
/*================================================================== */

/*================================================================== */
/* */
/*	T_VerticalDoor */
/* */
/*================================================================== */
void T_VerticalDoor (vldoor_t *door)//L800152D0()
{
	result_e	res;

	switch(door->direction)
	{
		case 0:		/* WAITING */
			if (!--door->topcountdown)
            {
				switch(door->type)
				{
					case BlazeRaise:
						door->direction = -1; /* time to go back down */
						S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);
						break;
					case Normal:
						door->direction = -1; /* time to go back down */
						S_StartSound((mobj_t *)&door->sector->soundorg,sfx_dorcls);
						break;
					case Close30ThenOpen:
						door->direction = 1;
						S_StartSound((mobj_t *)&door->sector->soundorg,sfx_doropn);
						break;
					default:
						break;
				}
            }
			break;
		case 2:		/*  INITIAL WAIT */
			if (!--door->topcountdown)
            {
				switch(door->type)
				{
					case RaiseIn5Mins:
						door->direction = 1;
						door->type = Normal;
						S_StartSound((mobj_t *)&door->sector->soundorg,sfx_doropn);
						break;
				default:
					break;
				}
            }
			break;
		case -1:	/* DOWN */
			res = T_MovePlane(door->sector,door->speed,
				door->sector->floorheight,false,1,door->direction);
			if (res == pastdest)
            {
				switch(door->type)
				{
					case BlazeRaise:
					case BlazeClose:
						door->sector->specialdata = NULL;
						P_RemoveThinker(&door->thinker);  /* unlink and free */
						S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);
						break;
					case Normal:
					case Close:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						break;
					case Close30ThenOpen:
						door->direction = 0;
						door->topcountdown = TICRATE*30;
						break;
					default:
						break;
				}
            }
			else if (res == crushed)
			{
				switch (door->type)
				{
				case BlazeClose:
				case Close:		/* DO NOT GO BACK UP! */
					break;

				default:
					door->direction = 1;
					S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
					break;
				}
			}
			break;
		case 1:		/* UP */
			res = T_MovePlane(door->sector,door->speed,
				door->topheight,false,1,door->direction);
			if (res == pastdest)
            {
				switch(door->type)
				{
					case BlazeRaise:
					case Normal:
						door->direction = 0; /* wait at top */
						door->topcountdown = door->topwait;
						break;
					case Close30ThenOpen:
					case BlazeOpen:
					case Open:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
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
/*		P_CheckKeyLock */
/*		Exclusive Psx Doom */
/*		Check for locks */
/* */
/*================================================================== */

boolean P_CheckKeyLock(line_t *line, mobj_t *thing)//L80015514()
{
	player_t *player;

	player = thing->player;

	if (!player)
		return false;

	switch (line->special)
	{
	case 26:		/* Blue Card Lock */
	case 32:
	case 99:
	case 133:
		if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
		{
			player->message = "You need a blue key.";
			S_StartSound(thing, sfx_oof);
			if (player == &players[consoleplayer])
				stbar.tryopen[it_bluecard] = true;
			return false;
		}
		break;
	case 27:		/* Yellow Card Lock */
	case 34:
	case 136:
	case 137:
		if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
		{
			player->message = "You need a yellow key.";
			S_StartSound(thing, sfx_oof);
			if (player == &players[consoleplayer])
				stbar.tryopen[it_yellowcard] = true;
			return false;
		}
		break;
	case 28:		/* Red Card Lock */
	case 33:
	case 134:
	case 135:
		if (!player->cards[it_redcard] && !player->cards[it_redskull])
		{
			player->message = "You need a red key.";
			S_StartSound(thing, sfx_oof);
			if (player == &players[consoleplayer])
				stbar.tryopen[it_redcard] = true;
			return false;
		}
		break;
	}

	return true;
}


/*================================================================== */
/* */
/*		EV_DoDoor */
/*		Move a door up/down and all around! */
/* */
/*================================================================== */
int EV_DoDoor (line_t *line, vldoor_e  type)//L800156FC()
{
	int			secnum,rtn;
	sector_t		*sec;
	vldoor_t		*door;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* */
		/* new door thinker */
		/* */
		rtn = 1;
		door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker (&door->thinker);
		sec->specialdata = door;
		door->thinker.function = T_VerticalDoor;
		door->topwait = VDOORWAIT;
		door->speed = VDOORSPEED;
		door->sector = sec;
		door->type = type;

		switch(type)
		{
			case BlazeClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				door->speed = VDOORSPEED * 4;
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);
				break;
			case Close:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,sfx_dorcls);
				break;
			case Close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,sfx_dorcls);
				break;
			case BlazeRaise:
			case BlazeOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->speed = VDOORSPEED * 4;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdopn);
				break;
			case Normal:
			case Open:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg,sfx_doropn);
				break;
			default:
				break;
		}
	}
	return rtn;
}


/*================================================================== */
/* */
/*	EV_VerticalDoor : open a door manually, no tag value */
/* */
/*================================================================== */
void EV_VerticalDoor (line_t *line, mobj_t *thing)//L80015920()
{
	player_t		*player;
	int				secnum;
	sector_t		*sec;
	vldoor_t		*door;
	int				side;

	side = 0;			/* only front sides can be used */

	/* if the sector has an active thinker, use it */
	sec = sides[ line->sidenum[side^1]] .sector;
	secnum = sec-sectors;
	if (sec->specialdata)
	{
		door = sec->specialdata;
		switch(line->special)
		{
			case 1:		/* ONLY FOR "RAISE" DOORS, NOT "OPEN"s */
			case 26:
			case 27:
			case 28:
			case 117:
				if (door->direction == -1)
					door->direction = 1;	/* go back up */
				else
				{
					if (!thing->player)
						return;				/* JDC: bad guys never close doors */
					door->direction = -1;	/* start going down immediately */
				}
				return;
		}
	}

	/* for proper sound */
	switch(line->special)
	{
		case 1:		/* NORMAL DOOR SOUND */
		case 31:
			S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn);
			break;
		case 117:	/* BLAZING DOOR RAISE */
		case 118:	/* BLAZING DOOR OPEN */
			S_StartSound((mobj_t *)&sec->soundorg, sfx_bdopn);
			break;
		default:	/* LOCKED DOOR SOUND */
			S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn);
			break;
	}

	/* */
	/* new door thinker */
	/* */
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	door->thinker.function = T_VerticalDoor;
	door->speed = VDOORSPEED;
	door->sector = sec;
	door->direction = 1;
	door->topwait = VDOORWAIT;

	switch(line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = Normal;
			break;
		case 31:
		case 32:
		case 33:
		case 34:
			door->type = Open;
			line->special = 0;
			break;
		case 117:	/* blazing door raise */
			door->type = BlazeRaise;
			door->speed = VDOORSPEED * 4;
			break;
		case 118:	/* blazing door open */
			door->type = BlazeOpen;
			door->speed = VDOORSPEED * 4;
			line->special = 0;
			break;
	}

	/* */
	/* find the top and bottom of the movement range */
	/* */
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
}

/*================================================================== */
/* */
/*	Spawn a door that closes after 30 seconds */
/* */
/*================================================================== */
void P_SpawnDoorCloseIn30 (sector_t *sec)//L80015B1C()
{
	vldoor_t	*door;

	door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
	door->thinker.function = T_VerticalDoor;
	door->sector = sec;
	door->direction = 0;
	door->type = Normal;
	door->speed = VDOORSPEED;
	door->topcountdown = 30 * TICRATE;
}

/*================================================================== */
/* */
/*	Spawn a door that opens after 5 minutes */
/* */
/*================================================================== */
void P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum)//L80015B9C()
{
	vldoor_t	*door;

	door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
	door->thinker.function = T_VerticalDoor;
	door->sector = sec;
	door->direction = 2;
	door->type = RaiseIn5Mins;
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
	door->topwait = VDOORWAIT;
	door->topcountdown = 5 * 60 * TICRATE;
}

