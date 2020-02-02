/* P_user.c */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"


fixed_t 		forwardmove[2] = {0x40000, 0x60000};//80067668
fixed_t 		sidemove[2] = {0x38000, 0x58000}; //80067670

#define SLOWTURNTICS    10
fixed_t			angleturn[] = //80067678
	{300,300,500,500,600,700,800,900,900,1000};
fixed_t			fastangleturn[] = //800676A0
	{800,800,900,1000,1000,1200,1200,1300,1300,1400};


/*============================================================================= */

mobj_t          *slidething;    //80077D04, pmGp000008f4
extern	fixed_t	slidex, slidey; //80077dbc || 80077dc0
extern	line_t	*specialline;   //80077dc8

void P_SlideMove ();

void P_PlayerMove (mobj_t *mo)//L80029670()
{
	fixed_t		momx, momy;
	line_t		*latchedline;
	fixed_t		latchedx, latchedy;

	momx = vblsinframe[playernum] * (mo->momx>>2);
	momy = vblsinframe[playernum] * (mo->momy>>2);

	slidething = mo;

	P_SlideMove ();

	latchedline = (line_t *)specialline;
	latchedx = slidex;
	latchedy = slidey;

	if ((latchedx == mo->x) && (latchedy == mo->y))
		goto stairstep;

	if (P_TryMove (mo, latchedx, latchedy))
		goto dospecial;

stairstep:

	if (momx > MAXMOVE)
		momx = MAXMOVE;
	else if (momx < -MAXMOVE)
		momx = -MAXMOVE;

	if (momy > MAXMOVE)
		momy = MAXMOVE;
	else if (momy < -MAXMOVE)
		momy = -MAXMOVE;

	/* something fucked up in slidemove, so stairstep */

	if (P_TryMove (mo, mo->x, mo->y + momy))
	{
		mo->momx = 0;
		mo->momy = momy;
		goto dospecial;
	}

	if (P_TryMove (mo, mo->x + momx, mo->y))
	{
		mo->momx = momx;
		mo->momy = 0;
		goto dospecial;
	}

	mo->momx = mo->momy = 0;

dospecial:
	if (latchedline)
		P_CrossSpecialLine (latchedline, mo);
}


/*
===================
=
= P_PlayerXYMovement
=
===================
*/

#define	STOPSPEED		0x1000
#define	FRICTION		0xd200  //Jag 0xd240
//inline
void P_PlayerXYMovement (mobj_t *mo)//L800297E8()
{
	P_PlayerMove (mo);

	/* */
	/* slow down */
	/* */
	if (mo->z > mo->floorz)
		return;		/* no friction when airborne */

	if (mo->flags & MF_CORPSE)
		if (mo->floorz != mo->subsector->sector->floorheight)
			return;			/* don't stop halfway off a step */

	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
	&& mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
	{
		mo->momx = 0;
		mo->momy = 0;
	}
	else
	{
		mo->momx = (mo->momx>>8)*(FRICTION>>8);
		mo->momy = (mo->momy>>8)*(FRICTION>>8);
	}
}


/*
===============
=
= P_PlayerZMovement
=
===============
*/
//inline
void P_PlayerZMovement (mobj_t *mo)//L800298D8()
{
	/* */
	/* check for smooth step up */
	/* */
	if (mo->z < mo->floorz)
	{
		mo->player->viewheight -= (mo->floorz - mo->z);
		mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight) >> 2;
	}

	/* */
	/* adjust height */
	/* */
	mo->z += mo->momz;

	/* */
	/* clip movement */
	/* */
	if (mo->z <= mo->floorz)
	{	/* hit the floor */
		if (mo->momz < 0)
		{
			if (mo->momz < -(GRAVITY*2))	/* squat down */
			{
				mo->player->deltaviewheight = mo->momz>>3;
				S_StartSound (mo, sfx_oof);
			}
			mo->momz = 0;
		}
		mo->z = mo->floorz;
	}
	else
	{
		if (mo->momz == 0)
			mo->momz = -GRAVITY;
		else
			mo->momz -= (GRAVITY/2);
	}

	if (mo->z + mo->height > mo->ceilingz)
	{	/* hit the ceiling */
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
	}
}


/*
================
=
= P_PlayerMobjThink
=
================
*/

void P_PlayerMobjThink (mobj_t *mobj)//L80029A08()
{
	state_t	*st;
	int		state;

	/* */
	/* momentum movement */
	/* */
	if (mobj->momx || mobj->momy)
		P_PlayerXYMovement (mobj);

	if ( (mobj->z != mobj->floorz) || mobj->momz)
		P_PlayerZMovement (mobj);

	/* */
	/* cycle through states, calling action functions at transitions */
	/* */
	if (mobj->tics == -1)
		return;				/* never cycle */

	mobj->tics--;

	if (mobj->tics > 0)
		return;				/* not time to cycle yet */

	state = mobj->state->nextstate;
	st = &states[state];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
}

/*============================================================================= */


/*
====================
=
= P_BuildMove
=
====================
*/

void P_BuildMove (player_t *player) //L80029CA4()
{
	int         speed;
	int			buttons, oldbuttons;
	mobj_t		*mo;
	buttons_t	*cbutton;
	fixed_t		ForwardMove, SideMove;
	angle_t		AngleTurn;

	cbutton = BT_DATA[playernum];
	buttons = ticbuttons[playernum];
	oldbuttons = oldticbuttons[playernum];

	speed = (buttons & cbutton->BT_SPEED) > 0;

	/*  */
	/* use two stage accelerative turning on the joypad  */
	/*  */
	if (((buttons & PAD_LEFT) && (oldbuttons & PAD_LEFT)) || ((buttons & PAD_RIGHT) && (oldbuttons & PAD_RIGHT)))
		player->turnheld++;
	else
		player->turnheld = 0;

	if (player->turnheld >= SLOWTURNTICS)
		player->turnheld = SLOWTURNTICS-1;

	player->forwardmove = player->sidemove = player->angleturn = 0;

	if (buttons & cbutton->BT_STRAFELEFT)
	{
		SideMove = -vblsinframe[playernum] * sidemove[speed];
		if (SideMove < 0) { SideMove += 3; }

		player->sidemove = (SideMove >> 2);
	}
	else if (buttons & cbutton->BT_STRAFERIGHT)
	{
		SideMove = vblsinframe[playernum] * sidemove[speed];
		if (SideMove < 0) { SideMove += 3; }

		player->sidemove = (SideMove >> 2);
	}

    if (buttons & cbutton->BT_STRAFE)
	{
		if (buttons & PAD_LEFT)
		{
			SideMove = -vblsinframe[playernum] * sidemove[speed];
			if (SideMove < 0) { SideMove += 3; }

			player->sidemove = (SideMove >> 2);
		}
		else if (buttons & PAD_RIGHT)
		{
			SideMove = vblsinframe[playernum] * sidemove[speed];
			if (SideMove < 0) { SideMove += 3; }

			player->sidemove = (SideMove >> 2);
		}
	}
	else
	{
		if (speed && !(buttons & (PAD_UP| PAD_DOWN)))
		{
			if (buttons & PAD_LEFT)
			{
				AngleTurn =  vblsinframe[playernum] * fastangleturn[player->turnheld];
				if (AngleTurn < 0) { AngleTurn += 3; }
				AngleTurn >>= 2;

				player->angleturn = AngleTurn << 17;
			}
			else if (buttons & PAD_RIGHT)
			{
				AngleTurn = vblsinframe[playernum] * fastangleturn[player->turnheld];
				if (AngleTurn < 0) { AngleTurn += 3; }
				AngleTurn >>= 2;

				player->angleturn = -AngleTurn << 17;
			}
		}
		else
		{
			if (buttons & PAD_LEFT)
			{
				AngleTurn = vblsinframe[playernum] * angleturn[player->turnheld];
				if (AngleTurn < 0) { AngleTurn += 3; }
				AngleTurn >>= 2;

				player->angleturn = AngleTurn << 17;
			}
			else if (buttons & PAD_RIGHT)
			{
				AngleTurn = vblsinframe[playernum] * angleturn[player->turnheld];
				if (AngleTurn < 0) { AngleTurn += 3; }
				AngleTurn >>= 2;

				player->angleturn = -AngleTurn << 17;
			}
		}
	}

	if (buttons & PAD_UP)
	{
		ForwardMove = vblsinframe[playernum] * forwardmove[speed];
		if (ForwardMove < 0) { ForwardMove += 3; }

		player->forwardmove = (ForwardMove >> 2);
	}
	else if (buttons & PAD_DOWN)
	{
		ForwardMove = -vblsinframe[playernum] * forwardmove[speed];
		if (ForwardMove < 0) { ForwardMove += 3; }

		player->forwardmove = (ForwardMove >> 2);
	}

	/* */
	/* if slowed down to a stop, change to a standing frame */
	/* */
	mo = player->mo;

	if (!mo->momx && !mo->momy && player->forwardmove == 0 && player->sidemove == 0 )
	{	/* if in a walking frame, stop moving */
		if (mo->state == &states[S_PLAY_RUN1]
		|| mo->state == &states[S_PLAY_RUN2]
		|| mo->state == &states[S_PLAY_RUN3]
		|| mo->state == &states[S_PLAY_RUN4])
			P_SetMobjState (mo, S_PLAY);
	}
}


/*
===============================================================================

						movement

===============================================================================
*/

#define MAXBOB			0x100000		/* 16 pixels of bob */

boolean		onground;//80077FF4, iGp00000be4

/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/
//inline
void P_Thrust (player_t *player, angle_t angle, fixed_t move) //L8002A188()
{
	angle >>= ANGLETOFINESHIFT;
	player->mo->momx += (move >> 8)*(finecosine[angle] >> 8);
	player->mo->momy += (move >> 8)*(finesine[angle] >> 8);
}



/*
==================
=
= P_CalcHeight
=
= Calculate the walking / running height adjustment
=
==================
*/

void P_CalcHeight (player_t *player) //L8002A1FC()
{
	int			angle;
	fixed_t		bob;
	fixed_t		val;

	/* */
	/* regular movement bobbing (needs to be calculated for gun swing even */
	/* if not on ground) */
	/* OPTIMIZE: tablify angle  */
	/* */
	val = player->mo->momx>>8;
	player->bob = val*val;
	val = player->mo->momy>>8;
	player->bob += val*val;

	player->bob >>= 4;
	if (player->bob > MAXBOB)
	{
		player->bob = MAXBOB;
	}

	if (!onground)
	{
		player->viewz = player->mo->z + VIEWHEIGHT;
		if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;
		return;
	}

	//angle = (FINEANGLES/40*gamevbls)&(FINEANGLES-1);
	angle = (FINEANGLES/40*ticon)&(FINEANGLES-1);
	bob = (player->bob >> 17) * finesine[angle];

	/* */
	/* move viewheight */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;
		if (player->viewheight > VIEWHEIGHT)
		{
			player->viewheight = VIEWHEIGHT;
			player->deltaviewheight = 0;
		}
		if (player->viewheight < VIEWHEIGHT/2)
		{
			player->viewheight = VIEWHEIGHT/2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if (player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT/2;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	player->viewz = player->mo->z + player->viewheight + bob;
	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		player->viewz = player->mo->ceilingz-4*FRACUNIT;
}

/*
=================
=
= P_MovePlayer
=
=================
*/

void P_MovePlayer (player_t *player)//L8002A3B8()
{
	player->mo->angle += player->angleturn;

	/* don't let the player control movement if not onground */
	onground = (player->mo->z <= player->mo->floorz);

	if (player->forwardmove && onground)
		P_Thrust (player, player->mo->angle, player->forwardmove);
	if (player->sidemove && onground)
        P_Thrust (player, player->mo->angle-ANG90, player->sidemove);

	if ((player->forwardmove || player->sidemove) && player->mo->state == &states[S_PLAY])
		P_SetMobjState (player->mo, S_PLAY_RUN1);
}


/*
=================
=
= P_DeathThink
=
=================
*/

#define		ANG5	(ANG90/18)

void P_DeathThink (player_t *player)//L8002A570()
{
	angle_t		angle, delta;

	P_MovePsprites (player);

	/* fall to the ground */
	if (player->viewheight > 8*FRACUNIT)
		player->viewheight -= FRACUNIT;
	onground = (player->mo->z <= player->mo->floorz);
	P_CalcHeight (player);

	if (player->attacker && player->attacker != player->mo)
	{
		angle = R_PointToAngle2 (player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
		delta = angle - player->mo->angle;
		if (delta < ANG5 || delta > (unsigned)-ANG5)
		{	/* looking at killer, so fade damage flash down */
			player->mo->angle = angle;
			if (player->damagecount)
				player->damagecount--;
		}
		else if (delta < ANG180)
			player->mo->angle += ANG5;
		else
			player->mo->angle -= ANG5;
	}
	else if (player->damagecount)
		player->damagecount--;


	if (((byte)ticbuttons[playernum] != 0) && (player->viewheight <= 8*FRACUNIT))
    {
		player->playerstate = PST_REBORN;
    }
}



/*
=================
=
= P_PlayerThink
=
=================
*/

//extern int ticphase;

void P_PlayerThink (player_t *player)//L8002A6C8()
{
	int		     buttons, oldbuttons;
	buttons_t    *cbutton;
	weapontype_t weapon, lastweapon;

	buttons = ticbuttons[playernum];
	oldbuttons = oldticbuttons[playernum];
	cbutton = BT_DATA[playernum];

	/* */
	/* check for weapon change */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		weapon = player->pendingweapon;
		if (weapon == wp_nochange)
			weapon = player->readyweapon;

		lastweapon = weaponowned[weapon];
		weapon = lastweapon;
		if ((buttons & cbutton->BT_WEAPONBACKWARD) && !(oldbuttons & cbutton->BT_WEAPONBACKWARD))
		{
			if (!lastweapon && player->weaponowned[wp_chainsaw])
			{
				lastweapon = wp_pistol;
			}
			else if(lastweapon > wp_fist)
			{
			    weapon -= 1;
				if (!player->weaponowned[weapon] && (weapon > wp_fist))
				{
                    weapon -= 1;
                    while (!player->weaponowned[weapon] && (weapon > wp_fist))
                    {
                        weapon -= 1;
                    }
				}
			}
		}
		else if ((buttons & cbutton->BT_WEAPONFORWARD) && !(oldbuttons & cbutton->BT_WEAPONFORWARD))
		{
			if (lastweapon < wp_chainsaw)
			{
				weapon += 1;
				if (!player->weaponowned[weapon] && (weapon < wp_chainsaw))
				{
					weapon += 1;
					while (!player->weaponowned[weapon] && (weapon < wp_chainsaw))
                    {
                        weapon += 1;
                    }
				}
			}

			if (weapon == wp_chainsaw)
			{
				weapon = wp_bfg;
				if (!player->weaponowned[wp_bfg])
				{
					do
					{
						weapon -= 1;
					} while (!player->weaponowned[weapon]);
				}
			}
		}

		if (weapon != lastweapon)
		{
			if ((weapon == wp_fist) && (player->readyweapon != wp_chainsaw))
			{
				weapon = (player->weaponowned[wp_chainsaw] > 0) << 3;
			}
			player->pendingweapon = weapon;
		}
	}

	if (!gamepaused)
	{
		P_PlayerMobjThink(player->mo);
		P_BuildMove(player);

		if (player->playerstate == PST_DEAD)
		{
			P_DeathThink(player);
			return;
		}

		/* */
		/* chain saw run forward */
		/* */
		if (player->mo->flags & MF_JUSTATTACKED)
		{
			player->angleturn = 0;
			player->forwardmove = 0xc800;
			player->sidemove = 0;
			player->mo->flags &= ~MF_JUSTATTACKED;
		}

		/* */
		/* move around */
		/* reactiontime is used to prevent movement for a bit after a teleport */
		/* */

		if (player->mo->reactiontime)
			player->mo->reactiontime--;
		else
			P_MovePlayer(player);

		P_CalcHeight(player);
		if (player->mo->subsector->sector->special)
		{
			P_PlayerInSpecialSector(player);

			if (player->playerstate == PST_DEAD)
				return;
		}

		/* */
		/* check for use */
		/* */

		if (buttons & cbutton->BT_USE)
		{
			if (player->usedown == false)
			{
				P_UseLines(player);
				player->usedown = true;
			}
		}
		else
			player->usedown = false;

		if (buttons & cbutton->BT_ATTACK)
		{
			P_SetMobjState(player->mo, S_PLAY_ATK1);

			player->attackdown++;
			if (player->attackdown > 30 && playernum == consoleplayer &&
               (player->readyweapon == wp_chaingun || player->readyweapon == wp_plasma))
            {
                stbar.specialFace = f_mowdown;
            }
		}
		else
			player->attackdown = 0;

		/* */
		/* cycle psprites */
		/* */

		P_MovePsprites(player);

		/* */
		/* counters */
		/* */

		if (gamevbls < gametic)
		{
			if (player->powers[pw_strength])
				player->powers[pw_strength]++;	/* strength counts up to diminish fade */

			if (player->powers[pw_invulnerability])
				player->powers[pw_invulnerability]--;

			if (player->powers[pw_invisibility])
			{
				player->powers[pw_invisibility]--;
				if (!player->powers[pw_invisibility])
				{
					player->mo->flags &= ~(MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3);
				}
				else if ((player->powers[pw_invisibility] < 61) && !(player->powers[pw_invisibility] & 7))
                {
                    player->mo->flags ^= (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3);
                }
			}

            if (player->powers[pw_infrared])
                player->powers[pw_infrared]--;

            if (player->powers[pw_ironfeet])
                player->powers[pw_ironfeet]--;

            if (player->damagecount)
                player->damagecount--;

            if (player->bonuscount)
                player->bonuscount--;
		}
	}
}


