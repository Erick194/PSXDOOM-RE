
    /*------------------------------------------------------------------*/
    /*
                     Williams Entertainment Sound System
                            by Scott Patterson

                     PSX SPU Volume and Reverb controls.
    */
    /*------------------------------------------------------------------*/

#ifndef _PSXSPU_H
#define _PSXSPU_H

#define FADE_TIME 250

    /*
        Notes on fade calls:

        To do a syncronous fade to zero volume:

            psxspu_start_cd_fade(FADE_TIME,0);
            while(psxspu_get_cd_fade_status());

        To do an asyncronous fade to zero volume:

            psxspu_start_cd_fade(FADE_TIME,0);
            .
            .
            .
            if(!psxspu_get_cd_fade_status())
            {
                //fade is now done
            }
    */

#define MAX_MASTER_VOL 0x3fff
#define MAX_CD_VOL    0x3cff
#define MED_CD_VOL    0x2fff
#define MIN_CD_VOL    0x1fff

#define SRAM_ADDRESS_LIMIT 520192
#define SRAM_RESERVED_SIZE 4112
#define SRAM_TO_ALLOC_SPU (SRAM_ADDRESS_LIMIT-SRAM_RESERVED_SIZE)

#define PSXSPU_REV_MODE_OFF        0
#define PSXSPU_REV_MODE_ROOM       1
#define PSXSPU_REV_MODE_STUDIO_A   2
#define PSXSPU_REV_MODE_STUDIO_B   3
#define PSXSPU_REV_MODE_STUDIO_C   4
#define PSXSPU_REV_MODE_HALL       5
#define PSXSPU_REV_MODE_SPACE      6
#define PSXSPU_REV_MODE_ECHO       7
#define PSXSPU_REV_MODE_DELAY      8
#define PSXSPU_REV_MODE_PIPE       9

extern unsigned long end_of_sram;

extern void psxspu_init(void);

extern void psxspu_init_reverb(long  rev_mode,
                               short depthleft,
                               short depthright,
                               long  delay,
                               long  feedback);

extern void psxspu_set_reverb_depth(short depthleft,
                                    short depthright);

extern void psxspu_fadeengine(void);

extern void psxspu_set_master_vol(int vol);
extern int  psxspu_get_master_vol(void);
extern void psxspu_start_master_fade(int msec,int destvol);
extern void psxspu_stop_master_fade(void);

//returns zero when fade is done
extern int  psxspu_get_master_fade_status(void);

extern void psxspu_set_cd_vol(int vol);
extern int  psxspu_get_cd_vol(void);
extern void psxspu_start_cd_fade(int msec,int destvol);
extern void psxspu_stop_cd_fade(void);

//returns zero when fade is done
extern int  psxspu_get_cd_fade_status(void);

extern void psxspu_setcdmixon(void);
extern void psxspu_setcdmixoff(void);

#endif

