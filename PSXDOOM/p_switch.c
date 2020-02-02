#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/* */
/*	CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE */
/* */
/*================================================================== */

#define     numswitches 49
switchlist_t alphSwitchList[numswitches] = //80067234
{
	{ "SW1BMET",	"SW2BMET" },
	{ "SW1BRICK",	"SW2BRICK" },
	{ "SW1BRNZ",	"SW2BRNZ" },
	{ "SW1BROWN",	"SW2BROWN" },
	{ "SW1MARB",	"SW2MARB" },
	{ "SW1MET",		"SW2MET" },
	{ "SW1METAL",	"SW2METAL" },
	{ "SW1NEW02",	"SW2NEW02" },
	{ "SW1NEW03",	"SW2NEW03" },
	{ "SW1NEW04",	"SW2NEW04" },
	{ "SW1NEW05",	"SW2NEW05" },
	{ "SW1NEW06",	"SW2NEW06" },
	{ "SW1NEW10",	"SW2NEW10" },
	{ "SW1NEW11",	"SW2NEW11" },
	{ "SW1NEW13",	"SW2NEW13" },
	{ "SW1NEW14",	"SW2NEW14" },
	{ "SW1NEW20",	"SW2NEW20" },
	{ "SW1NEW25",	"SW2NEW25" },
	{ "SW1NEW26",	"SW2NEW26" },
	{ "SW1NEW28",	"SW2NEW28" },
	{ "SW1NEW29",	"SW2NEW29" },
	{ "SW1NEW30",	"SW2NEW30" },
	{ "SW1NEW31",	"SW2NEW31" },
	{ "SW1NEW32",	"SW2NEW32" },
	{ "SW1NEW33",	"SW2NEW33" },
	{ "SW1NEW34",	"SW2NEW34" },
	{ "SW1NEW35",	"SW2NEW35" },
	{ "SW1NEW36",	"SW2NEW36" },
	{ "SW1NEW39",	"SW2NEW39" },
	{ "SW1NEW40",	"SW2NEW40" },
	{ "SW1NEW41",	"SW2NEW41" },
	{ "SW1NEW42",	"SW2NEW42" },
	{ "SW1NEW45",	"SW2NEW45" },
	{ "SW1NEW46",	"SW2NEW46" },
	{ "SW1NEW47",	"SW2NEW47" },
	{ "SW1NEW51",	"SW2NEW51" },
	{ "SW1NEW57",	"SW2NEW57" },
	{ "SW1NEW60",	"SW2NEW60" },
	{ "SW1NEW63",	"SW2NEW63" },
	{ "SW1NEW65",	"SW2NEW65" },
	{ "SW1NEW66",	"SW2NEW66" },
	{ "SW1NEW68",	"SW2NEW68" },
	{ "SW1NEW69",	"SW2NEW69" },
	{ "SW1NEW70",	"SW2NEW70" },
	{ "SW1RED",		"SW2RED" },
	{ "SW1RUST",	"SW2RUST" },
	{ "SW1SKULL",	"SW2SKULL" },
	{ "SW1STAR",	"SW2STAR" },
	{ "SW1STEEL",	"SW2STEEL" },
};

//int			switchlist[MAXSWITCHES * 2];//80097424
//int			numswitches;
//button_t	    buttonlist[MAXBUTTONS];//800975B0


/*
===============
=
= P_InitSwitchList
=
= Only called at game initialization
=
===============
*/

void P_InitSwitchList(void)//L80027C64()
{
	int		i;
	int		index;
	int		tex1, tex2;

	//numswitches = sizeof(alphSwitchList)/sizeof(alphSwitchList[0]);

	for (index = 0,i = 0;i < numswitches;i++)
	{
		//if (!alphSwitchList[i].name1[0])
			//break;

		tex1 = R_TextureNumForName(alphSwitchList[i].name1);
		tex2 = R_TextureNumForName(alphSwitchList[i].name2);

		if (textures[tex1].vtpage != 0 && textures[tex2].vtpage == 0)
			TextureCache(&textures[tex2]);

		if (textures[tex2].vtpage != 0 && textures[tex1].vtpage == 0)
			TextureCache(&textures[tex1]);

		switchlist[index] = tex1;
		index++;
		switchlist[index] = tex2;
		index++;
	}

	//switchlist[index] = -1;
}

/*================================================================== */
/* */
/*	Start a button counting down till it turns off. */
/* */
/*================================================================== */
void P_StartButton(line_t *line,bwhere_e w,int texture,int time)//80027D88
{
	int		i;

	for (i = 0;i < MAXBUTTONS;i++)
    {
		if (buttonlist[i].btimer == 0)
		{
			buttonlist[i].line = line;
			buttonlist[i].where = w;
			buttonlist[i].btexture = texture;
			buttonlist[i].btimer = time;
			buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
			return;
		}
    }

	//I_Error("P_StartButton: no button slots left!");
}

/*================================================================== */
/* */
/*	Function that changes wall texture. */
/*	Tell it if switch is ok to use again (1=yes, it's a button). */
/* */
/*================================================================== */
void P_ChangeSwitchTexture(line_t *line,int useAgain)//L80027E1C()
{
	int	texTop;
	int	texMid;
	int	texBot;
	int	i;
	int	sound;

	if (!useAgain)
		line->special = 0;

	texTop = sides[line->sidenum[0]].toptexture;
	texMid = sides[line->sidenum[0]].midtexture;
	texBot = sides[line->sidenum[0]].bottomtexture;

	sound = sfx_swtchn;
	if (line->special == 11)		/* EXIT SWITCH? */
		sound = sfx_swtchx;

	for (i = 0;i < numswitches*2;i++)
    {
		if (switchlist[i] == texTop)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].toptexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line,top,switchlist[i],BUTTONTIME);
			return;
		}
		else if (switchlist[i] == texMid)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].midtexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line, middle,switchlist[i],BUTTONTIME);
			return;
		}
		else if (switchlist[i] == texBot)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].bottomtexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line, bottom,switchlist[i],BUTTONTIME);
			return;
        }
    }
}

/*
==============================================================================
=
= P_UseSpecialLine
=
= Called when a thing uses a special line
= Only the front sides of lines are usable
===============================================================================
*/

boolean P_UseSpecialLine ( mobj_t *thing, line_t *line)//L8002810C()
{
	/* */
	/*	Switches that other things can activate */
	/* */
	if (!thing->player)
	{
		if (line->flags & ML_SECRET)
			return false;		/* never open secret doors */
		switch(line->special)
		{
			case 1:		/* MANUAL DOOR RAISE */
/*			case 32:	// MANUAL BLUE */
/*			case 33:	// MANUAL RED */
/*			case 34:	// MANUAL YELLOW */
				break;
			default:
				return false;
		}
	}

	/* */
	/* do something */
	/*	*/
	switch (line->special)
	{
		/*=============================================== */
		/*	MANUALS */
		/*=============================================== */
		case 26:		/* Blue Door Raise */
		case 27:		/* Yellow Door Raise */
		case 28:		/* Red Door Raise */
		case 32:		/* Blue Door Open */
		case 33:		/* Red Door Open */
		case 34:		/* Yellow Door open */
			if (P_CheckKeyLock(line,thing))
				EV_VerticalDoor(line, thing);
			break;
		case 1:			/* Vertical Door */
		case 31:		/* Manual Door Open */
		case 117:		/* Blazing Door Raise */
		case 118:		/* Blazing Door Open */
				EV_VerticalDoor(line, thing);
			break;
		case 99:		/* Blue Blazing Door Open */
		case 134:		/* Red Blazing Door Open */
		case 136:		/* Yellow Blazing Door open */
			if (P_CheckKeyLock(line,thing))
			{
				if (EV_DoDoor(line, BlazeOpen))
					P_ChangeSwitchTexture(line, 1);
			}
			break;
		case 133:		/* Blue Blazing Door Open */
		case 135:		/* Red Blazing Door Open */
		case 137:		/* Yellow Blazing Door open */
			if (P_CheckKeyLock(line,thing))
			{
				if (EV_DoDoor(line, BlazeOpen))
					P_ChangeSwitchTexture(line, 1);//[GEC] Bug?? DoomPc is 0
			}
			break;
		/*=============================================== */
		/*	BUTTONS */
		/*=============================================== */
		case 42:		/* Close Door */
			if (EV_DoDoor(line, Close))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 43:		/* Lower Ceiling to Floor */
			if (EV_DoCeiling(line, lowerToFloor))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 45:		/* Lower Floor to Surrounding floor height */
			if (EV_DoFloor(line, lowerFloor))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 60:		/* Lower Floor to Lowest */
			if (EV_DoFloor(line, lowerFloorToLowest))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 61:		/* Open Door */
			if (EV_DoDoor(line, Open))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 62:		/* PlatDownWaitUpStay */
			if (EV_DoPlat(line, downWaitUpStay, 1))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 63:		/* Raise Door */
			if (EV_DoDoor(line, Normal))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 64:		/* Raise Floor to ceiling */
			if (EV_DoFloor(line, raiseFloor))
				P_ChangeSwitchTexture(line, 1);
			break;
        case 65:		/* Raise Floor Crush */
			if (EV_DoFloor(line, raiseFloorCrush))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 66:		/* Raise Floor 24 and change texture */
			if (EV_DoPlat(line, raiseAndChange, 24))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 67:		/* Raise Floor 32 and change texture */
			if (EV_DoPlat(line, raiseAndChange, 32))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 68:		/* Raise Plat to next highest floor and change texture */
			if (EV_DoPlat(line, raiseToNearestAndChange, 0))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 69:		/* Raise Floor to next highest floor */
			if (EV_DoFloor(line, raiseFloorToNearest))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 70:		/* Turbo Lower Floor */
			if (EV_DoFloor(line, turboLower))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 114:		/* Blazing Door Raise (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeRaise))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 115:		/* Blazing Door Open (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeOpen))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 116:		/* Blazing Door Close (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeClose))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 123:		/* Blazing PlatDownWaitUpStay */
			if (EV_DoPlat(line, blazeDWUS, 0))
				P_ChangeSwitchTexture(line, 1);
			break;
		case 138:		/* Light Turn On */
			EV_LightTurnOn(line, 255);
			P_ChangeSwitchTexture(line, 1);
			break;
		case 139:		/* Light Turn Off */
			EV_LightTurnOn(line, 35);
			P_ChangeSwitchTexture(line, 1);
			break;

		/*=============================================== */
		/*	SWITCHES */
		/*=============================================== */
		case 7:			/* Build Stairs */
			if (EV_BuildStairs(line, build8))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 9:			/* Change Donut */
			if (EV_DoDonut(line))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 11:		/* Exit level */
            P_ExitLevel();//G_ExitLevel();
            P_ChangeSwitchTexture(line, 0);
			break;
		case 14:		/* Raise Floor 32 and change texture */
			if (EV_DoPlat(line, raiseAndChange, 32))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 15:		/* Raise Floor 24 and change texture */
			if (EV_DoPlat(line, raiseAndChange, 24))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 18:		/* Raise Floor to next highest floor */
			if (EV_DoFloor(line, raiseFloorToNearest))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 20:		/* Raise Plat next highest floor and change texture */
			if (EV_DoPlat(line, raiseToNearestAndChange, 0))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 21:		/* PlatDownWaitUpStay */
			if (EV_DoPlat(line, downWaitUpStay, 0))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 23:		/* Lower Floor to Lowest */
			if (EV_DoFloor(line, lowerFloorToLowest))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 29:		/* Raise Door */
			if (EV_DoDoor(line, Normal))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 41:		/* Lower Ceiling to Floor */
			if (EV_DoCeiling(line, lowerToFloor))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 49:		/* Ceiling Crush And Raise */
			if (EV_DoCeiling(line, crushAndRaise))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 50:		/* Close Door */
			if (EV_DoDoor(line, Close))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 51:		/* Secret EXIT */
			P_SecretExitLevel(line->tag);//G_SecretExitLevel();
			P_ChangeSwitchTexture(line, 0);
			break;
		case 55:		/* Raise Floor Crush */
			if (EV_DoFloor(line, raiseFloorCrush))
				P_ChangeSwitchTexture(line, 0);
			break;
        case 71:		/* Turbo Lower Floor */
			if (EV_DoFloor(line, turboLower))
				P_ChangeSwitchTexture(line, 0);
            break;
		case 101:		/* Raise Floor */
			if (EV_DoFloor(line, raiseFloor))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 102:		/* Lower Floor to Surrounding floor height */
			if (EV_DoFloor(line, lowerFloor))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 103:		/* Open Door */
			if (EV_DoDoor(line, Open))
				P_ChangeSwitchTexture(line, 0);
			break;
        case 111:	/* Blazing Door Raise (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeRaise))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 112:		/* Blazing Door Open (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeOpen))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 113:		/* Blazing Door Close (faster than TURBO!) */
			if (EV_DoDoor(line, BlazeClose))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 122:		/* Blazing PlatDownWaitUpStay */
			if (EV_DoPlat(line, blazeDWUS, 0))
				P_ChangeSwitchTexture(line, 0);
			break;
		case 127:		/* Build Stairs Turbo 16 */
			if (EV_BuildStairs(line, turbo16))
				P_ChangeSwitchTexture(line, 0);
			break;
	}

	return true;
}
