// Reconstruido Y Verificado por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 32/01/2020 [GEC]

    /*------------------------------------------------------------------*/
    /*
                     Sony CD Architecture Specific Routines
                            by Scott Patterson

                     development and testing period on this code:
                            4/18/95 - 9/28/95
    */
    /*------------------------------------------------------------------*/

#include <kernel.h>
#include "wessapi.h"
#include "psxcd.h"
#include "psxspu.h"

// undeclared SONY prototypes
//extern void CdInit(void);

#if _CD_VERSION_ == 1

// CD Stuff
// CD Stuff
// CD Stuff
// CD Stuff

#if _CD_ABS_OPEN_ == 1
  //lets just put this table in this file
  #include "psxcdabs.c"
#endif

// private variables
static volatile int seeking_for_play = 0;
static volatile int waiting_for_pause = 0;
static volatile int async_on = 0;
static CdlLOC sectorbuf_contents = {0,0,0,0};

static void psxcd_memcpy(void *pdest, void *psrc, unsigned long bytestocopy);

//static void (*cd_pause_callback)(void) = 0;

static u_long cbsyncsave;
static u_long cbreadysave;
static void cbcomplete(int intr, u_char *result);
static void cbready(int intr, u_char *result);

static int psxcd_mode = -1; //initially an undefined mode
static int IsCdInit = 0;
static int init_pos = 0;

static PsxCd_File cdfile;
static CdlLOC cur_io_loc = {0,0,0,0};

int cb_enable_flag = 0;
int critical_error = 0;
int cdl_intr = 0;
int cdl_errintr = 0;
int cdl_errcount = 0;
unsigned char cdl_stat = 0;
unsigned char cdl_errstat = 0;
unsigned char cdl_com = CdlPause;
unsigned char cdl_errcom = 0;
int readcount = 0;
int playcount = 0;

char sectorbuf[CD_ROM_SECTOR_SIZE];

static char cd_param[8] = {0,0,0,0,0,0,0,0};

// CD ASYNC CHECKING
static int  sync_intr = 0;//iGp000007c4 80077BD4
static char sync_result[8] = {0,0,0,0,0,0,0,0};//80077bd8
static int  check_intr = 0;
static char check_result[8] = {0,0,0,0,0,0,0,0};

// CD ASYNC STUFF
#define PSXCD_COMMAND_END       0
#define PSXCD_COMMAND_COPY      1
#define PSXCD_COMMAND_SEEK      2
#define PSXCD_COMMAND_READ      3
#define PSXCD_COMMAND_READCOPY  4

typedef struct PsxCd_Command {
    int     command;
    int     amount; //sectors or bytes
    char   *pdest;  //buffer or direct
    char   *psrc;  //buffer pos
    CdlLOC  io_loc;
}PsxCd_Command;

static int cur_cmd = 0;
static PsxCd_Command psxcd_cmd[5];

static void *lastdestptr = 0;
static int lastreadbytes = 0;
static PsxCd_File newfilestruct;
static PsxCd_File lastfilestruct;

// CD-DA play stuff
static int playflag = 0;
static int playvol = 0;
static int playfadeuptime = 0;
static int looptrack = 0;
static int loopflag = 0;
static int loopvol = 0;
static int loopsectoroffset = 0;
static int loopfadeuptime = 0;
static CdlLOC  cdloc = {0,0,0,0};
static CdlATV  cdatv = {0,0,0,0};
static CdlLOC  loc[100];
static CdlLOC  newloc = {0,0,0,0};
static CdlLOC  lastloc = {0,0,0,0};
static CdlLOC  beginloc = {0,0,0,0};

static void psxcd_memcpy(void *pdest, void *psrc, unsigned long bytestocopy)//L8003EF70()
{
    while(bytestocopy--)
    {
        *(char *)pdest++ = *(char *)psrc++;
    }
}

extern volatile unsigned long millicount;

static void psxcd_sync(void)//8003EFA4
{
    unsigned long block;

    block = millicount;
    block += 8000;

    while(millicount<block)
    {
        sync_intr = CdSync(1,(u_char *)sync_result);
        if(sync_intr==CdlDiskError)
        {
            CdFlush();
            cdl_errintr = sync_intr + 80; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = sync_result[0];
            cdl_errcount++;
        }
        if(sync_intr==CdlComplete) return;
    }
}

static int psxcd_critical_sync(void)//8003F060
{
    unsigned long block;

    block = millicount;
    block += 8000;

    while(millicount<block)
    {
        sync_intr = CdSync(1,(u_char *)sync_result);
        if(sync_intr==CdlDiskError)
        {
            CdFlush();
            cdl_errintr = sync_intr + 70; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = sync_result[0];
            cdl_errcount++;
            return(0);
        }
        if(sync_intr==CdlComplete) return(1);
    }
    return(0);
}

static void cbcomplete(int intr, u_char *result)//8003F11C
{
    if(!cb_enable_flag) return;

    //cdl_intr = intr + 10; //to give us unique error numbers
    //cdl_stat = result[0];
    if(intr == CdlComplete)
    {
        if(cdl_com==CdlSeekP)
        {
            seeking_for_play = 0;
            if(playflag)
            {
                #if _CD_SPU_LINK_ == 1
                 psxspu_setcdmixon();
                 if(playfadeuptime)
                 {
                    psxspu_set_cd_vol(0);
                    psxspu_start_cd_fade(playfadeuptime,playvol);
                    playfadeuptime = 0;
                 } else {
                    psxspu_set_cd_vol(playvol);
                 }
                #endif

                CdControlF(cdl_com=CdlPlay,0);
            }
        } else if(cdl_com==CdlPause) {
            waiting_for_pause = 0;
        }
    } else {
        // error = intr
        cdl_errintr = intr + 10; //to give us unique error numbers
        cdl_errcom = cdl_com;
        cdl_errstat = result[0];
        cdl_errcount++;
        //if((cdl_com==CdlSeekP)&&(result[0]&2))
        //{
        //    CdControlF(cdl_com=CdlSeekP,(u_char *)&cdloc);
        //}
    }
}

static void cbready(int intr, u_char *result)//8003F200
{
    if(!cb_enable_flag) return;

    cdl_intr = intr;
    cdl_stat = result[0];
    if((result[0]&CdlStatRead)&&async_on&&(cdl_com==CdlReadN))
    {
        //if shell open, hang till closed then reread
        readcount++;
        if (intr == CdlDataReady)
        {
            switch(psxcd_cmd[cur_cmd].command)
            {
                case PSXCD_COMMAND_READ:
                    //if((!CdGetSector((u_long *)psxcd_cmd[cur_cmd].pdest, 2048/4))||(result[0]!=0x22))
                    //{
                    //    cdl_errintr = 50;
                    //    cdl_errcom = cdl_com;
                    //    cdl_errstat = result[0];
                    //    cdl_errcount++;
                    //    ////reissue the last read command
                    //    //newfilestruct = lastfilestruct;
                    //    //psxcd_async_read(lastdestptr,lastreadbytes,&newfilestruct);
                    //    return;
                    //}
                    // must check for quad align!!!!
                    if((unsigned long)psxcd_cmd[cur_cmd].pdest&3)
                    {
                        CdGetSector((u_long *)sectorbuf, 2048/4);
                        psxcd_memcpy(psxcd_cmd[cur_cmd].pdest,
                                     sectorbuf,
                                     2048);
                    } else {
                        CdGetSector((u_long *)psxcd_cmd[cur_cmd].pdest, 2048/4);
                    }
                    psxcd_cmd[cur_cmd].pdest += 2048;
                    if(!--psxcd_cmd[cur_cmd].amount)
                    {
                        cur_cmd++;
                    }
                    break;

                case PSXCD_COMMAND_READCOPY:
                    //if((!CdGetSector((u_long *)sectorbuf, 2048/4))||(result[0]!=0x22))
                    //{
                    //    cdl_errintr = 50;
                    //    cdl_errcom = cdl_com;
                    //    cdl_errstat = result[0];
                    //    cdl_errcount++;
                    //    //reissue the last read command
                    //    //newfilestruct = lastfilestruct;
                    //    //psxcd_async_read(lastdestptr,lastreadbytes,&newfilestruct);
                    //    return;
                    //}
                    CdGetSector((u_long *)sectorbuf, 2048/4);
                    psxcd_memcpy(psxcd_cmd[cur_cmd].pdest,
                                 psxcd_cmd[cur_cmd].psrc,
                                 psxcd_cmd[cur_cmd].amount);
                    cur_cmd++;
                    break;

                default:
                    psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_END;
                    break;

            }

            if(psxcd_cmd[cur_cmd].command == PSXCD_COMMAND_END)
            {
                async_on = 0;
                waiting_for_pause = 1;
                CdControlF(cdl_com=CdlPause, 0);
            }

            return;
        } else {
            // error = intr
            if(intr==CdlDiskError)
            {
                CdFlush();
                critical_error = 1;
            }
            cdl_errintr = intr;
            cdl_errcom = cdl_com;
            cdl_errstat = result[0];
            cdl_errcount++;

            //reissue the last read command
            //newfilestruct = lastfilestruct;
            //psxcd_async_read(lastdestptr,lastreadbytes,&newfilestruct);
            return;
        }
    }


    if(result[0]&CdlStatPlay/*cdl_com==CdlPlay*/)
    {
        //if shell open, hang till closed then restart play
        if (intr == CdlDataReady)
        {
            if ((result[4]&0x80) == 0)
            {
                playcount++;
                newloc.track = btoi(result[1]);
                newloc.minute = result[3];
                newloc.second = result[4];
                newloc.sector = result[5];
                //vol   = (result[6]<<8)|result[7];
            }

            return;
        } else {
            // error = intr
            // intr==CdlDataEnd means end of track
            if(intr==CdlDataEnd)
            {
                *(int *)&lastloc = 0;
                cdl_com = CdlNop;
                //if playflag and loopflag, play again
                if(playflag||loopflag) //if(playflag&&loopflag)
                {
                    psxcd_play_at_andloop(looptrack, loopvol, loopsectoroffset, loopfadeuptime,
                                           looptrack, loopvol, loopsectoroffset, loopfadeuptime);
                }
            } else {
                if(intr==CdlDiskError)
                {
                    CdFlush();
                }
                cdl_errintr = intr + 20; //to give us unique error numbers
                cdl_errcom = cdl_com;
                cdl_errstat = result[0];
                cdl_errcount++;
            }

            return;
        }
    }

    //if(intr==CdlDiskError)
    //{
    //    CdFlush();
    //}
    //we only get here if there is an error
    cdl_errintr = intr + 30; //to give us unique error numbers
    cdl_errcom = cdl_com;
    cdl_errstat = result[0];
    cdl_errcount++;
}

#endif //_CD_VERSION_

void psxcd_disable_callbacks(void)//8003F604
{
#if _CD_VERSION_ == 1
    cb_enable_flag = 0;
#endif
}

void psxcd_enable_callbacks(void)//8003F610
{
#if _CD_VERSION_ == 1
    cb_enable_flag = 1;
#endif
}

void psxcd_init(void)//8003F620
{
#if _CD_VERSION_ == 1

    int i,ntoc;

    if(IsCdInit)
    {
        return;
    }

    CdInit();

    IsCdInit = 1;

    for(i=0;i<100;i++)
    {
        loc[i].minute = 0;
        loc[i].second = 0;
        loc[i].sector = 0;
        loc[i].track = 0;
    }

    #if _CD_SPU_LINK_ == 1
     psxspu_init();
    #endif

    if((ntoc=CdGetToc(loc)) == 0)
    {
        //printf("No TOC found: please use CD-DA disc...\n");
        return;
    }

    init_pos = 0;
    async_on = 0;
    cbsyncsave = (u_long)CdSyncCallback((CdlCB)cbcomplete);
    cbreadysave = (u_long)CdReadyCallback((CdlCB)cbready);

    psxcd_enable_callbacks();

#else
    return;
#endif
}

void psxcd_exit(void)//8003F6FC
{
#if _CD_VERSION_ == 1
    CdSyncCallback((void *)cbsyncsave);
    CdReadyCallback((void *)cbreadysave);
#else
    return;
#endif
}

void psxcd_set_data_mode(void)//8003F72C
{
#if _CD_VERSION_ == 1
    //int millitemp;

    if(psxcd_mode!=1)
    {
        playflag = 0; // ensure no SeekP callback activity
        loopflag = 0;
        seeking_for_play = 0;

        #if _CD_SPU_LINK_ == 1
         if(psxspu_get_cd_vol())
         {
             psxspu_start_cd_fade(FADE_TIME,0);
             while(psxspu_get_cd_fade_status());
         }
         psxspu_setcdmixoff();
        #endif

        psxcd_sync(); // to ensure no pending pauses...

        cd_param[0] = CdlModeSpeed;  //|CdlModeRept not necessary

        CdControl(cdl_com=CdlSetmode, cd_param, 0);

        psxcd_mode = 1;

        //millitemp = millicount;
        //millitemp += 1500;
        //while(millicount<millitemp);

        psxcd_sync(); // to ensure no pending pauses...

        CdControl(cdl_com=CdlPause, 0, 0);

        psxcd_sync(); // to ensure no pending pauses...

        CdFlush();

    } else {
        if(async_on)
        {
            psxcd_async_read_cancel();
        }

        if(cdl_com!=CdlPause)
        {
            psxcd_sync(); // to ensure no pending pauses...
            CdControl(cdl_com=CdlPause, 0, 0);
        }

        psxcd_sync(); // to ensure no pending pauses...

        CdFlush();

    }
#endif
}

#if _CD_VERSION_ == 1

 #if _CD_ABS_OPEN_ == 1

  PsxCd_File *psxcd_open(char *filename)//8003F83C
  {
      //printf("filename %d\n", (int)filename);
      //printf("abs %d\n", cdmaptbl[(int)filename].abs);
      //printf("size %d\n", cdmaptbl[(int)filename].size);

      CdIntToPos(cdmaptbl[(int)filename].abs,&cdfile.file.pos);
      cdfile.file.size = cdmaptbl[(int)filename].size;
      cdfile.new_io_loc = cdfile.file.pos;

      cdfile.io_block_offset = 0;
      cdfile.io_result[0] = 0;
      cdfile.io_result[1] = 0;
      cdfile.io_result[2] = 0;
      cdfile.io_result[3] = 0;
      cdfile.io_result[4] = 0;
      cdfile.io_result[5] = 0;
      cdfile.io_result[6] = 0;
      cdfile.io_result[7] = 0;

      return(&cdfile);

  }

 #else /* _CD_ABS_OPEN_ */

  PsxCd_File *psxcd_open(char *filename)
  {
      //int dum;

      psxcd_set_data_mode();

      cdfile.file.pos.minute = 0;
      cdfile.file.pos.second = 0;
      cdfile.file.pos.sector = 0;
      cdfile.file.pos.track  = 0;

      if(!CdSearchFile(&cdfile.file,filename))
      {
          return(0);
      }

      cdfile.new_io_loc = cdfile.file.pos;

      cdfile.io_block_offset = 0;
      cdfile.io_result[0] = 0;
      cdfile.io_result[1] = 0;
      cdfile.io_result[2] = 0;
      cdfile.io_result[3] = 0;
      cdfile.io_result[4] = 0;
      cdfile.io_result[5] = 0;
      cdfile.io_result[6] = 0;
      cdfile.io_result[7] = 0;

      return(&cdfile);

  }
 #endif /* _CD_ABS_OPEN_ */

#endif /* _CD_VERSION_ */


void psxcd_init_pos(void)//8003F90C
{
#if _CD_VERSION_ == 1
    init_pos = 0;
    playflag = 0;
    loopflag = 0;
    seeking_for_play = 0;
    waiting_for_pause = 0;
    critical_error = 0;
#endif
}

int psxcd_async_on(void)//8003F92C
{
#if _CD_VERSION_ == 1
    if(async_on)
    {
        check_intr = CdSync(1,(u_char *)check_result);
        if(critical_error ||
           (check_intr==CdlDiskError) ||
           //(check_result[0] & !(CdlStatSeek|CdlStatRead|CdlStatStandby)) || // This line of code are blocked in Psx Doom
           !(check_result[0] & CdlStatStandby))
        {
            CdFlush();
            critical_error = 0;
            cdl_errintr = check_intr + 100; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = check_result[0];
            cdl_errcount++;

            //reissue the last read command
            newfilestruct = lastfilestruct;
            psxcd_async_read(lastdestptr,lastreadbytes,&newfilestruct);
        }
        return(1);
    } else {
        return(0);
    }
#else
    return(0);
#endif
}

int psxcd_seeking_for_play(void)//8003FA34
{
#if _CD_VERSION_ == 1
    if(seeking_for_play)
    {
        check_intr = CdSync(1,(u_char *)check_result);
        if((check_intr==CdlDiskError) ||
           //(check_result[0] & !(CdlStatSeek|CdlStatPlay|CdlStatStandby)) || // This line of code are blocked in Psx Doom
           !(check_result[0] & CdlStatStandby))
        {
            CdFlush();
            cdl_errintr = check_intr + 110; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = check_result[0];
            cdl_errcount++;

            //reissue the last play command
            psxcd_sync();

            CdControlF(cdl_com=CdlSeekP, (u_char *)&lastloc);
        }
        return(1);
    } else {
        return(0);
    }
#else
    return(0);
#endif
}

int psxcd_waiting_for_pause(void)//8003FAE4
{
#if _CD_VERSION_ == 1
    if(waiting_for_pause)
    {
        check_intr = CdSync(1,(u_char *)check_result);
        if((check_intr==CdlDiskError) ||
           //(check_result[0] & !(CdlStatSeek|CdlStatRead|CdlStatPlay|CdlStatStandby)) || // This line of code are blocked in Psx Doom
           !(check_result[0] & CdlStatStandby))
        {
            CdFlush();
            cdl_errintr = check_intr + 120; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = check_result[0];
            cdl_errcount++;

            //reissue the pause command
            psxcd_sync();

            CdControlF(cdl_com=CdlPause, 0);
        }
        return(1);
    } else {
        return(0);
    }
#else
    return(0);
#endif
}

int psxcd_read(void *destptr,int readbytes,PsxCd_File *fileptr)//8003FB90
{
    int retbytes;

    retbytes = psxcd_async_read(destptr, readbytes, fileptr);

    // wait for Async
    while(psxcd_async_on());

    return(retbytes);
}

void psxcd_async_read_cancel(void)//8003FBC8
{
#if _CD_VERSION_ == 1
    if(async_on)
    {
        async_on = 0;
        init_pos = 0;

        psxcd_sync(); // to ensure no pending pauses...

        waiting_for_pause = 1;
        CdControlF(cdl_com=CdlPause, 0);
    }
#endif
}

int psxcd_async_read(void *destptr,int readbytes,PsxCd_File *fileptr)//8003FC14
{
#if _CD_VERSION_ == 1

    int read_setup_failure;
    int sectors;
    char *pdest;
    unsigned long bytestoread,byteoffset,bytestocopy,bytesofblocks;

    if(!readbytes)
    {
        return(0);
    }

    if(*(int *)&fileptr->file.pos==0)
    {
        return(0);
    }

    do
    {
        sectors = 0;
        read_setup_failure = 0;

        psxcd_set_data_mode();

        lastdestptr = destptr;
        lastreadbytes = readbytes;
        lastfilestruct = *fileptr;

        // pause check is later in this function

        cur_cmd = 0;

        pdest = (char *)destptr;
        bytestoread = readbytes;
        byteoffset = fileptr->io_block_offset;

        // if read begins inside block:

        if(byteoffset && bytestoread)
        {
            if(!init_pos||(*(long *)&cur_io_loc!=*(long *)&fileptr->new_io_loc))
            {
                psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_SEEK;
                psxcd_cmd[cur_cmd].io_loc = fileptr->new_io_loc;
                cur_cmd++;

                cur_io_loc = fileptr->new_io_loc;

                init_pos = 1;
            }

            bytestocopy = CD_ROM_SECTOR_SIZE-byteoffset;
            if(bytestocopy>bytestoread)
            {
                bytestocopy = bytestoread;
            }

            if(cur_cmd||(*(long *)&sectorbuf_contents!=*(long *)&cur_io_loc))
            {
                psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_READCOPY;
                psxcd_cmd[cur_cmd].amount = bytestocopy;
                psxcd_cmd[cur_cmd].pdest = pdest;
                psxcd_cmd[cur_cmd].psrc = sectorbuf + byteoffset;
                psxcd_cmd[cur_cmd].io_loc = cur_io_loc;
                cur_cmd++;

                sectorbuf_contents = cur_io_loc;
            } else {
                psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_COPY;
                psxcd_cmd[cur_cmd].amount = bytestocopy;
                psxcd_cmd[cur_cmd].pdest = pdest;
                psxcd_cmd[cur_cmd].psrc = sectorbuf + byteoffset;
                cur_cmd++;

            }

            bytestoread -= bytestocopy;
            pdest += bytestocopy;
            fileptr->io_block_offset = byteoffset + bytestocopy;
            if(fileptr->io_block_offset==CD_ROM_SECTOR_SIZE)
            {
                CdIntToPos(CdPosToInt(&cur_io_loc) + 1,&cur_io_loc);
                fileptr->new_io_loc = cur_io_loc;
                fileptr->io_block_offset = 0;
            }
        }

        // for block aligned reading:

        if(bytestoread>=CD_ROM_SECTOR_SIZE)
        {
            sectors = bytestoread/CD_ROM_SECTOR_SIZE;
            bytesofblocks = sectors*CD_ROM_SECTOR_SIZE;

            if(!init_pos||(*(long *)&cur_io_loc!=*(long *)&fileptr->new_io_loc))
            {
                psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_SEEK;
                psxcd_cmd[cur_cmd].io_loc = fileptr->new_io_loc;
                cur_cmd++;

                cur_io_loc = fileptr->new_io_loc;

                init_pos = 1;
            }

            psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_READ;
            psxcd_cmd[cur_cmd].amount = sectors;
            psxcd_cmd[cur_cmd].pdest = pdest;
            if((unsigned long)pdest&3)
            {
                CdIntToPos(CdPosToInt(&cur_io_loc) + sectors - 1,&sectorbuf_contents);
            }
            psxcd_cmd[cur_cmd].io_loc = cur_io_loc;
            cur_cmd++;

            pdest += bytesofblocks;
            bytestoread -= bytesofblocks;

            CdIntToPos(CdPosToInt(&cur_io_loc) + sectors,&cur_io_loc);

            fileptr->io_block_offset = 0;
            fileptr->new_io_loc = cur_io_loc;
        }

        // if read ends inside block:

        if(bytestoread)
        {
            if(!init_pos||(*(long *)&cur_io_loc!=*(long *)&fileptr->new_io_loc))
            {
                psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_SEEK;
                psxcd_cmd[cur_cmd].io_loc = fileptr->new_io_loc;
                cur_cmd++;

                cur_io_loc = fileptr->new_io_loc;

                init_pos = 1;
            }

            psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_READCOPY;
            psxcd_cmd[cur_cmd].amount = bytestoread;
            psxcd_cmd[cur_cmd].pdest = pdest;
            psxcd_cmd[cur_cmd].psrc = sectorbuf;
            psxcd_cmd[cur_cmd].io_loc = cur_io_loc;
            cur_cmd++;

            sectorbuf_contents = cur_io_loc;

            fileptr->io_block_offset = bytestoread;
        }

        psxcd_cmd[cur_cmd].command = PSXCD_COMMAND_END;
        cur_cmd = 0;

        // if we are just a copy , then skip all the async stuff
        if(psxcd_cmd[cur_cmd].command == PSXCD_COMMAND_COPY)
        {
            // do copy
            psxcd_memcpy(psxcd_cmd[cur_cmd].pdest,
                         psxcd_cmd[cur_cmd].psrc,
                         psxcd_cmd[cur_cmd].amount);
            // increment cur_cmd
            cur_cmd++;
            // set to end
            if(psxcd_cmd[cur_cmd].command == PSXCD_COMMAND_END)
            {
                return(readbytes);
            }
        }

        // if we seek, start now...
        if(psxcd_cmd[cur_cmd].command == PSXCD_COMMAND_SEEK)
        {
            // do seek
            psxcd_sync();
            CdControl(cdl_com=CdlSetloc,
                      (u_char *)&psxcd_cmd[cur_cmd].io_loc, 0);
            // increment cur_cmd
            cur_cmd++;
            if(psxcd_cmd[cur_cmd].command == PSXCD_COMMAND_END)
            {
                return(readbytes);
            }
        }

        switch(psxcd_cmd[cur_cmd].command)
        {
            case PSXCD_COMMAND_READ:
            case PSXCD_COMMAND_READCOPY:
                //do read
                if(!psxcd_critical_sync())
                {
                    read_setup_failure = 1;
                    break;
                }
                CdControl(cdl_com=CdlReadN,
                          (u_char *)&psxcd_cmd[cur_cmd].io_loc, 0);
                if(!psxcd_critical_sync())
                {
                    read_setup_failure = 1;
                    break;
                }
                async_on = 1;
                break;

            case PSXCD_COMMAND_END:
                    return(readbytes);
                break;

            default:
                    return(0);
                break;

        }

        if(read_setup_failure)
        {
            *fileptr = lastfilestruct;
        }
    }while(read_setup_failure);

    return(readbytes);

#else
    return(0);
#endif
}

int psxcd_seek(PsxCd_File *fileptr,int seekpos,int seekmode)//80040444
{
#if _CD_VERSION_ == 1

    int sector_index;

    if(*(int *)&fileptr->file.pos==0)
    {
        return(0);
    }

    if(seekmode==PSXCD_SEEK_SET)
    {
        sector_index = (seekpos / CD_ROM_SECTOR_SIZE) + CdPosToInt(&fileptr->file.pos);

        CdIntToPos(sector_index,&fileptr->new_io_loc);
        fileptr->io_block_offset = seekpos % CD_ROM_SECTOR_SIZE;

    } else if(seekmode==PSXCD_SEEK_CUR) {
        sector_index = ((seekpos+fileptr->io_block_offset) / CD_ROM_SECTOR_SIZE) + CdPosToInt(&cur_io_loc);

        CdIntToPos(sector_index,&fileptr->new_io_loc);
        fileptr->io_block_offset = (seekpos+fileptr->io_block_offset) % CD_ROM_SECTOR_SIZE;

    } else { //PSXCD_SEEK_END
        sector_index = ((fileptr->file.size-seekpos) / CD_ROM_SECTOR_SIZE) + CdPosToInt(&fileptr->file.pos);

        CdIntToPos(sector_index,&fileptr->new_io_loc);
        fileptr->io_block_offset = (fileptr->file.size-seekpos) % CD_ROM_SECTOR_SIZE;
    }
    return(0);

#else
    return(0);
#endif
}

unsigned long psxcd_tell(PsxCd_File *fileptr)//80040538
{
#if _CD_VERSION_ == 1

    int sector_index;

    if(*(int *)&fileptr->file.pos==0)
    {
        return(0);
    }

    sector_index = CdPosToInt(&fileptr->new_io_loc) - CdPosToInt(&fileptr->file.pos);
    return((sector_index*CD_ROM_SECTOR_SIZE) + fileptr->io_block_offset);

#else
    return(0);
#endif
}

void psxcd_close(PsxCd_File *fileptr)//800405A0
{
#if _CD_VERSION_ == 1

#endif
}

void psxcd_set_audio_mode(void)//800405A8
{
#if _CD_VERSION_ == 1
    if(psxcd_mode!=0)
    {
        if(async_on)
        {
            psxcd_async_read_cancel();
        }

        init_pos = 0; //since we have lost the data position of the CD
        cd_param[0] = CdlModeRept|CdlModeAP|CdlModeDA;

        CdControl(cdl_com=CdlSetmode, cd_param, 0);

        psxcd_mode = 0;

        psxcd_sync();

        CdControl(cdl_com=CdlPause, 0, 0);

        psxcd_sync(); // to ensure no pending pauses...

        CdFlush();
    } else {
        psxcd_sync();
    }
#endif
}

void psxcd_set_loop_volume(int volforloop)//8004064C
{
#if _CD_VERSION_ == 1
    loopvol = volforloop;
#endif
}

void psxcd_play_at_andloop(int track,
                           int vol,
                           int sectoroffset,
                           int fadeuptime,
                           int tracktoloop,
                           int volforloop,
                           int loopstartsectoroffset,
                           int loopstartfadeuptime)//80040658
{
#if _CD_VERSION_ == 1

    if(*(int *)&loc[track]==0)
    {
        return;
    }

    psxcd_set_audio_mode();

    playvol = vol;

    CdIntToPos((CdPosToInt(&loc[track]) + sectoroffset),&cdloc);

    playcount = 0;
    playflag = 1;
    loopflag = 1;
    looptrack = tracktoloop;
    loopvol = volforloop;
    loopsectoroffset = loopstartsectoroffset;
    loopfadeuptime = loopstartfadeuptime;
    seeking_for_play = 1;
    playfadeuptime = fadeuptime;
    CdControlF(cdl_com=CdlSeekP, (u_char *)&cdloc);

    lastloc = loc[track];
    beginloc = lastloc;
    newloc = lastloc;

#endif
}

void psxcd_play_at(int track, int vol, int sectoroffset)//800407B8
{
#if _CD_VERSION_ == 1

    if(*(int *)&loc[track]==0)
    {
        return;
    }

    playflag = 0;
    loopflag = 0;
    seeking_for_play = 0;

    psxcd_set_audio_mode();

    playvol = vol;

    CdIntToPos((CdPosToInt(&loc[track]) + sectoroffset),&cdloc);

    playcount = 0;
    playflag = 1;
    loopflag = 0;
    seeking_for_play = 1;
    playfadeuptime = 0;

    CdControlF(cdl_com=CdlSeekP, (u_char *)&cdloc);

    lastloc = loc[track];
    beginloc = lastloc;
    newloc = lastloc;

#endif
}

void psxcd_play(int track, int vol)//800408DC
{
    psxcd_play_at(track,vol,0);
}

void psxcd_seek_for_play_at(int track, int sectoroffset)//800408FC
{
#if _CD_VERSION_ == 1

    if(*(int *)&loc[track]==0)
    {
        return;
    }

    playflag = 0;
    loopflag = 0;
    seeking_for_play = 0;

    psxcd_set_audio_mode();

    CdIntToPos((CdPosToInt(&loc[track]) + sectoroffset),&cdloc);

    playcount = 0;
    playflag = 0;
    loopflag = 0;
    seeking_for_play = 1;
    CdControlF(cdl_com=CdlSeekP, (u_char *)&cdloc);

    lastloc = loc[track];
    beginloc = lastloc;
    newloc = lastloc;

#endif
}

void psxcd_seek_for_play(int track)//80040A10
{
    psxcd_seek_for_play_at(track, 0);
}

int psxcd_play_status(void)//80040A30
{
#if _CD_VERSION_ == 1
    if((cdl_com==CdlPlay)||(cdl_com==CdlSeekP))
    {
        check_intr = CdSync(1,(u_char *)check_result);
        if((check_intr==CdlDiskError) ||
           //(check_result[0] & !(CdlStatSeek|CdlStatPlay|CdlStatStandby)) || // This line of code are blocked in Psx Doom
           !(check_result[0] & CdlStatStandby))
        {
            CdFlush();
            cdl_errintr = check_intr + 90; //to give us unique error numbers
            cdl_errcom = cdl_com;
            cdl_errstat = check_result[0];
            cdl_errcount++;

            //report not playing
            return(0);
        }
        return(1);
    } else {
        return(0);
    }
#else
    return(0);
#endif
}

void psxcd_stop(void)//80040AC8
{
#if _CD_VERSION_ == 1

    playflag = 0; // ensure no SeekP callback activity
    loopflag = 0;
    seeking_for_play = 0;

    *(int *)&lastloc = 0;

    #if _CD_SPU_LINK_ == 1
     if(psxspu_get_cd_vol())
     {
         psxspu_start_cd_fade(FADE_TIME,0);
         while(psxspu_get_cd_fade_status());
     }
    #endif

    psxcd_sync();

    // pause seems to be better than stop here because
    // play is more stable after a pause
    waiting_for_pause = 1;
    CdControlF(cdl_com=CdlPause, 0);

#endif
}

void psxcd_pause(void)//80040B40
{
#if _CD_VERSION_ == 1

    playflag = 0; // ensure no SeekP callback activity
    seeking_for_play = 0;

    // this check is
    if(*(int *)&lastloc==0)
    {
        return;
    }

    lastloc = newloc;

    #if _CD_SPU_LINK_ == 1
    if(psxspu_get_cd_vol())
    {
         psxspu_start_cd_fade(FADE_TIME,0);
         while(psxspu_get_cd_fade_status());
    }
    #endif

    psxcd_sync();

    waiting_for_pause = 1;
    CdControlF(cdl_com=CdlPause, 0);

#endif
}

void psxcd_restart(int vol)//80040BE4
{
#if _CD_VERSION_ == 1

    if(*(int *)&lastloc==0)
    {
        return;
    }

    psxcd_set_audio_mode();

    playvol = vol;

    cdloc = lastloc;

    playcount = 0;
    playflag = 1;
    seeking_for_play = 1;
    CdControlF(cdl_com=CdlSeekP, (u_char *)&cdloc);

#endif
}

int psxcd_elapsed_sectors(void)//80040C6C
{
#if _CD_VERSION_ == 1
    if(*(int *)&beginloc==0)
    {
        return(0);
    }
    return(CdPosToInt(&newloc)-CdPosToInt(&beginloc));
#else
    return(0);
#endif
}

void psxcd_set_stereo(int stereo_true)//80040CC0
{
#if _CD_VERSION_ == 1
    if(stereo_true)
    {
        cdatv.val0 = 127;
        cdatv.val1 = 0;
        cdatv.val2 = 127;
        cdatv.val3 = 0;
    } else {
        cdatv.val0 = 63;
        cdatv.val1 = 63;
        cdatv.val2 = 63;
        cdatv.val3 = 63;
    }

    CdMix(&cdatv);
#endif
}


