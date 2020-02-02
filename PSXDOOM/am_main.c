/* am_main.c -- automap */

#include "doomdef.h"
#include "p_local.h"

#define STEPVALUE   0x800000

#define COLOR_RED     0xA40000
#define COLOR_GREEN   0x00C000
#define COLOR_BROWN   0x8A5C30
#define COLOR_YELLOW  0xCCCC00
#define COLOR_GREY    0x808080
#define COLOR_AQUA    0x0080FF

#define MAXSCALE	64
#define MINSCALE	8

//automap bounding box of level
fixed_t am_maxleft;     //iGp00000c94
fixed_t am_maxright;    //iGp00000ca4
fixed_t am_maxtop;      //iGp00000cac
fixed_t am_maxbottom;   //iGp00000ca0

void AM_DrawLine(int color, int x1, int y1, int x2, int y2);


/*================================================================= */
/* */
/* Start up Automap */
/* */
/*================================================================= */
void AM_Start(void)//L8003B83C()
{
    am_maxleft = bmaporgx;
    am_maxbottom = bmaporgy;
    am_maxright = (bmapwidth << 23) + bmaporgx;
    am_maxtop = (bmapheight << 23) + bmaporgy;
}

/*
==================
=
= AM_Control
=
= Called by P_PlayerThink before any other player processing
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

void AM_Control (player_t *player)//L8003B884()
{
	int buttons, oldbuttons, step;

	if (gamepaused)
        return;

    buttons = ticbuttons[playernum];
    oldbuttons = oldticbuttons[playernum];

    if ((buttons & PAD_SELECT) && !(oldbuttons & PAD_SELECT))
    {
        player->automapflags ^= AF_ACTIVE;
        player->automapx = player->mo->x;
        player->automapy = player->mo->y;
    }

    if(!(player->automapflags & AF_ACTIVE))
        return;

    if (player->playerstate != PST_LIVE)
        return;

    if (!(buttons & PAD_CROSS))
    {
        player->automapflags &= ~AF_FOLLOW;
        return;
    }

    if (!(player->automapflags & AF_FOLLOW))
    {
        player->automapflags |= AF_FOLLOW;
        player->automapx = player->mo->x;
        player->automapy = player->mo->y;
    }

    step = STEPVALUE;
    if (buttons & PAD_SQUARE)
        step *= 2;

    if (!(player->automapflags & AF_FOLLOW))
        return;


    //
    // check bounding box collision
    //

    if (buttons & PAD_RIGHT)
    {
        player->automapx += step;

        if (am_maxright < player->automapx)
            player->automapx = am_maxright;
    }
    else if (buttons & PAD_LEFT)
    {
        player->automapx -= step;

        if (player->automapx < am_maxleft)
            player->automapx = am_maxleft;
    }

    if (buttons & PAD_UP)
    {
        player->automapy += step;

        if (am_maxtop < player->automapy)
            player->automapy = am_maxtop;
    }
    else if (buttons & PAD_DOWN)
    {
        player->automapy -= step;

        if (player->automapy < am_maxbottom)
            player->automapy = am_maxbottom;
    }

    if (buttons & PAD_R1)
    {
        player->automapscale -= 2;

        if (player->automapscale < MINSCALE)
            player->automapscale = MINSCALE;
    }
    else if (buttons & PAD_L1)
    {
        player->automapscale += 2;

        if (MAXSCALE < player->automapscale)
            player->automapscale = MAXSCALE;
    }

    ticbuttons[playernum] &= ~(PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_R1 | PAD_L1);
}

/*
==================
=
= AM_Drawer
=
= Draws the current frame to workingscreen
==================
*/

void AM_Drawer (void)//L8003BAB0()
{
    int			i;
	player_t	*p;
	line_t		*line;
	mobj_t		*mo;
	mobj_t		*next;
	fixed_t		x1, y1;
	fixed_t		x2, y2;
	fixed_t		ox, oy;
	fixed_t     c;
    fixed_t     s;
	fixed_t		nx1, ny1;
	fixed_t		nx2, ny2;
	fixed_t		nx3, ny3;
	angle_t     angle;
	int			color;
	int			scale;

	DrawRender();

	p = &players[consoleplayer];

	scale = p->automapscale;
	if (p->automapflags & AF_FOLLOW)
	{
		ox = p->automapx;
		oy = p->automapy;
	}
	else
	{
		ox = p->mo->x;
		oy = p->mo->y;
	}

	line = lines;
	for (i = 0; i < numlines; i++, line++)
	{
		if ((!(line->flags & ML_MAPPED) || // IF NOT MAPPED OR DON'T DRAW
			line->flags & ML_DONTDRAW) &&
			(!(p->powers[pw_allmap] + (p->cheats & CF_ALLLINES))))
			continue;

		x1 = (scale * (line->v1->x - ox >> 8)) >> 16;
		y1 = (scale * (line->v1->y - oy >> 8)) >> 16;
		x2 = (scale * (line->v2->x - ox >> 8)) >> 16;
		y2 = (scale * (line->v2->y - oy >> 8)) >> 16;

		//
		// Figure out color
		//
		color = COLOR_BROWN;

		if ((p->powers[pw_allmap] + (p->cheats & CF_ALLLINES)) && // IF COMPMAP && !MAPPED YET
			!(line->flags & ML_MAPPED))
			color = COLOR_GREY;
		else if (line->flags & ML_SECRET)
			color = COLOR_RED;
		else if (line->special)
			color = COLOR_YELLOW;
		else if (!(line->flags & ML_TWOSIDED)) // ONE-SIDED LINE
			color = COLOR_RED;

		AM_DrawLine(color, x1, y1, x2, y2);
	}

	// SHOW ALL MAP THINGS (CHEAT)
	if (p->cheats & CF_ALLTHINGS)
	{
		for (mo = mobjhead.next; mo != &mobjhead; mo = next)
		{
			next = mo->next;

			if (mo == p->mo)//Ignore player
                continue;

            angle = mo->angle;
            x1 = mo->x - ox;
            y1 = mo->y - oy;

            c = finecosine[angle >> ANGLETOFINESHIFT];
            s = finesine[angle >> ANGLETOFINESHIFT];
            nx1 = scale * ((x1 + (c * 24)) >> 8) >> 16;
            ny1 = scale * ((y1 + (s * 24)) >> 8) >> 16;

            c = finecosine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
            s = finesine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
            nx2 = scale * ((x1 + (c * 24)) >> 8) >> 16;
            ny2 = scale * ((y1 + (s * 24)) >> 8) >> 16;

            c = finecosine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
            s = finesine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
            nx3 = scale * ((x1 + (c * 24)) >> 8) >> 16;
            ny3 = scale * ((y1 + (s * 24)) >> 8) >> 16;

            AM_DrawLine(COLOR_AQUA, nx1, ny1, nx2, ny2);
            AM_DrawLine(COLOR_AQUA, nx2, ny2, nx3, ny3);
            AM_DrawLine(COLOR_AQUA, nx1, ny1, nx3, ny3);
		}
	}

	// SHOW PLAYERS
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if ((i != consoleplayer) && (netgame != gt_coop))
			continue;

		p = &players[i];

		if (p->playerstate == PST_LIVE && !(gametic & 2))
            continue;

		color = COLOR_GREEN;
		if ((i == consoleplayer) && (netgame == gt_coop))
			color = COLOR_YELLOW;

		angle = p->mo->angle;
		x1 = p->mo->x - ox;
		y1 = p->mo->y - oy;

		c = finecosine[angle >> ANGLETOFINESHIFT];
		s = finesine[angle >> ANGLETOFINESHIFT];
		nx1 = scale * ((x1 + (c * 24)) >> 8) >> 16;
        ny1 = scale * ((y1 + (s * 24)) >> 8) >> 16;

		c = finecosine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
		s = finesine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
		nx2 = scale * ((x1 + (c * 24)) >> 8) >> 16;
        ny2 = scale * ((y1 + (s * 24)) >> 8) >> 16;

		c = finecosine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
		s = finesine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
		nx3 = scale * ((x1 + (c * 24)) >> 8) >> 16;
        ny3 = scale * ((y1 + (s * 24)) >> 8) >> 16;

		AM_DrawLine(color, nx1, ny1, nx2, ny2);
		AM_DrawLine(color, nx2, ny2, nx3, ny3);
		AM_DrawLine(color, nx1, ny1, nx3, ny3);
	}
}

/*
==================
=
= AM_DrawLine
=
==================
*/

//From StationDoom By BodbDearg
typedef enum
{
	INSIDE  = 0,
    LEFT    = 1,
    RIGHT	= 2,
    BOTTOM	= 4,
    TOP	    = 8
} outflags_t;

//Update OutCode Flags From StationDoom By BodbDearg
void AM_DrawLine(int color, int x1, int y1, int x2, int y2)//L8003C16C()
{
    int outcode1, outcode2;
    LINE_F2 *line = (LINE_F2*) getScratchAddr(128);//1F800200

    outcode1 = (x1 < -128) ? LEFT : INSIDE;
    if (128 < x1 ) {outcode1 |= RIGHT;}
    if (y1 < -100) {outcode1 |= BOTTOM;}
    if (100 < y1 ) {outcode1 |= TOP;}

    outcode2 = (x2 < -128) ? LEFT : INSIDE;
    if (128 < x2 ) {outcode2 |= RIGHT;}
    if (y2 < -100) {outcode2 |= BOTTOM;}
    if (100 < y2 ) {outcode2 |= TOP;}

    if (outcode1 & outcode2)
        return;

    setLineF2(line);
    setRGB0(line, (color >> 16), (color >> 8), color);
    setXY2(line, (x1 + 128), (100 - y1), (x2 + 128) , (100 - y2));
    W_AddPrim(line);
}
