    /*------------------------------------------------------------------*/
    /*
                     Sony CD Architecture Specific Routines
                            by Scott Patterson

                     development and testing period on this code:
                            4/18/95 - 9/28/95
    */
    /*------------------------------------------------------------------*/

#ifndef _PSXCD_H
#define _PSXCD_H

    /*
        The following functions will pause for a short CD volume fade if
        the CD volume is not zero (and _CD_SPU_LINK_ == 1):

        psxcd_async_read()
        psxcd_read()
        psxcd_stop()
        psxcd_pause()

        To asyncronously fade the CD volume beforehand so these functions
        will not have any delays see comments in "psxspu.h"
    */

#include <sys\types.h>
#include <libcd.h>

#define _CD_VERSION_  1
#define _CD_SPU_LINK_ 1//para cd tracks
#define _CD_ABS_OPEN_ 1

#define PSXCD_SEEK_SET    0
#define PSXCD_SEEK_CUR    1
#define PSXCD_SEEK_END    2

#if _CD_VERSION_ == 0
 #include <libsn.h>
#endif

typedef struct PsxCd_File {
    CdlFILE       file;
    CdlLOC        new_io_loc;
    unsigned long io_block_offset;
    unsigned char io_result[8];
}PsxCd_File;

#if _CD_ABS_OPEN_ == 1

typedef struct PsxCd_MapTbl {
    int           abs;
    u_long        size;
}PsxCd_MapTbl;

extern PsxCd_MapTbl cdmaptbl[];

#endif

// CD-data read stuff
#define CD_ROM_SECTOR_SIZE 2048
#define SECTORS_PER_SECOND 75
#define SECONDS_PER_MINUTE 60

/*
    Asyncronous read function: psxcd_async_on()

    this function returns non-zero during asyncronous reading

    to code an asyncronous read:

    psxcd_async_read(destptr,readbytes,fileptr);

    // do your other shit here
    // (don't mess with the RAM you are reading to!)

    // to check for completion
    if(!psxcd_async_on())
    {
        // psxcd_async_read is complete
    }

    // to wait for completion:
    while(psxcd_async_on()); // wait for completion of psxcd_async_read
*/

extern int           psxcd_async_on(void);

/*
    CD-DA play seek variable: seeking_for_play

    after psxcd_seek_for_play(track) call is made

    this function returns non-zero until seeking is done
*/

extern int           psxcd_seeking_for_play(void);

/*
    CD-DA play stop/pause variable: waiting_for_pause

    after psxcd_stop/psxcd_pause call is made

    this function returns non-zero until pause operation is done
*/

extern int           psxcd_waiting_for_pause(void);

extern int           cdl_insetmode;
extern int           cdl_intr;
extern unsigned char cdl_stat;
extern unsigned char cdl_com;

extern int           cdl_errcount;
extern int           cdl_errintr;
extern unsigned char cdl_errstat;
extern unsigned char cdl_errcom;
extern int readcount;
extern int playcount;

// PSX CD-MODE1 DATA commands

extern void          psxcd_init(void);

extern void          psxcd_exit(void);

extern void          psxcd_disable_callbacks(void);

extern void          psxcd_enable_callbacks(void);

extern void          psxcd_set_data_mode(void);

extern PsxCd_File   *psxcd_open(char *filename);

extern void          psxcd_async_read_cancel(void);

extern int           psxcd_async_read(void *destptr,int readbytes,PsxCd_File *fileptr);

extern int           psxcd_read(void *destptr,int readbytes,PsxCd_File *fileptr);

extern int           psxcd_seek(PsxCd_File *fileptr,int seekpos,int seekmode);

extern unsigned long psxcd_tell(PsxCd_File *fileptr);

extern void          psxcd_close(PsxCd_File *fileptr);

// PSX CD-AUDIO commands

extern void psxcd_set_stereo(int stereo_true);

extern void psxcd_set_audio_mode(void);

extern void psxcd_seek_for_play(int track);

extern void psxcd_seek_for_play_at(int track,
                                   int sectoroffset);

extern void psxcd_play(int track, int vol);

extern void psxcd_play_at(int track,
                          int vol,
                          int sectoroffset);

extern void psxcd_set_loop_volume(int volforloop);

extern void psxcd_play_at_andloop(int track,
                                  int vol,
                                  int sectoroffset,
                                  int fadeuptime,
                                  int tracktoloop,
                                  int volforloop,
                                  int loopstartsectoroffset,
                                  int loopstartfadeuptime);

extern void psxcd_stop(void);

extern void psxcd_pause(void);

extern void psxcd_restart(int vol);

int psxcd_play_status(void);

int psxcd_elapsed_sectors(void);

// PSX low level CD extras

extern char sectorbuf[CD_ROM_SECTOR_SIZE];

extern void psxcd_init_pos(void);

#endif

