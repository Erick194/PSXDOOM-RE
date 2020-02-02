/* the status bar consists of two 8 bit color objects: */
/* the static background pic and a transparent foreground object that */
/* all the stats are drawn onto when they change */
/* coordinates are status bar relative (0 is first line of sbar) */

#define	FLASHDELAY	4		/* # of tics delay (1/30 sec) */
#define FLASHTIMES	6		/* # of times to flash new frag amount (EVEN!) */

#define AMMOX		52
#define AMMOY		14

#define HEALTHX		104
#define HEALTHY		AMMOY

#define KEYX		124
#define REDKEYY		3
#define BLUKEYY		15
#define YELKEYY		27
#define KEYW		16
#define KEYH		11

#define FACEX		144
#define FACEY		5

#define ARMORX		226
#define ARMORY		HEALTHY

#define MAPX		316
#define MAPY		HEALTHY

#define HISFRAGX	MAPX
#define HISFRAGY	HEALTHY

#define YOURFRAGX	278
#define YOURFRAGY	HEALTHY

typedef enum
{
	f_none,
	f_unk1,
	f_unk2,
	f_faceleft,		/* turn face left */
	f_faceright,	/* turn face right */
	f_hurtbad,		/* surprised look when slammed hard */
	f_gotgat,		/* picked up a weapon smile */
	f_mowdown,		/* grimace while continuous firing */
	NUMSPCLFACES
} spclface_e;

#define	NUMFACES	47  //[PSX] Change 48 to 47
#define	EVILFACE    6
#define	GODFACE		40
#define DEADFACE 	41
#define FIRSTSPLAT	42

#define GIBTIME		2   //[PSX] Change 4 to 2
#define NUMSPLATS	6
#define	NUMMICROS	8	/* amount of micro-sized #'s (weapon armed) */

typedef struct
{
	byte xpos;
	byte ypos;
	byte x;
	byte y;
	byte w;
	byte h;
} facedata_t;

extern facedata_t facedata[NUMFACES];//0x80073c98

typedef enum
{
	sb_minus,
	sb_0,
	sb_percent = 11,
	sb_card_b,
	sb_card_y,
	sb_card_r,
	sb_skul_b,
	sb_skul_y,
	sb_skul_r,
	NUMSBOBJ
} sbobj_e;

typedef struct
{
	short	active; //*
	short	doDraw; //2
	short   delay;  //4
	short	times;  //6
} sbflash_t;

typedef struct
{
	int		    face;               //800984F4
	/* Messaging */
	spclface_e	specialFace;	    //800984F8  /* Which type of special face to make */
	boolean     tryopen[NUMCARDS];	//800984FC  /* Tried to open a card or skull door */
	boolean	    gotgibbed;          //80098514  /* Got gibbed */
	int         gibframe;           //80098518
	int         gibdelay;           //8009851C
	char        *message;           //80098520
    int         messagedelay;       //80098524
} stbar_t;

extern	stbar_t	stbar; //800984F4
extern  psxobj_t statuspic;
extern	int     weaponowned[NUMWEAPONS];  //80073ebc
extern	short	micronums_x[NUMMICROS];//80073ee0
extern	short	micronums_y[NUMMICROS];//80073ef0
extern	short	card_y[NUMCARDS];//80073f00

typedef struct {
	byte x;
	byte y;
	byte w;
	byte h;
} symboldata_t;

extern symboldata_t symboldata[];//80073e16

void ST_Init (void);
void ST_InitEveryLevel(void);
void ST_Ticker (void);
void ST_Drawer (void);
void ST_DrawValue(int x, int y, int value);
void ST_DrawMessage(int x, int y, char *text);
void ST_CheatDraw(void);
void ST_doPaletteStuff(void);
int ST_GetTextCenterX(char *text);
void ST_DrawText(int x, int y, char *text);


