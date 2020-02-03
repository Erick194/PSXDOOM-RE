/* This file is for trial only, it does not really belong to PSXDOOM */

#include <kernel.h>
#include <libspu.h>

extern void SeqEngine(void);
static long WessInterruptHandler(void);

//int SeqOn = 0;
extern volatile int SeqOn;
extern volatile int val;
unsigned long millicount = 0;

int WessTimerActive = 0;

int                     T2counter = 0;
static unsigned long    EV2 = 0;                /* interrupt event var */

static long WessInterruptHandler(void)
{
    T2counter++;
    //val += 2;
    //printf("T2counter %d\n",T2counter);
    //printf("SeqOn %d\n",SeqOn);
    //psxspu_fadeengine();
    if(SeqOn)
    {
        SeqEngine();
    }
    return(0);
}

void init_WessTimer(void)
{
    SeqOn = 0;
    EnterCriticalSection();

    EV2 = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, WessInterruptHandler);
    EnableEvent(EV2);
    SetRCnt(RCntCNT2, 34722, RCntMdINTR);   /* 33038 is ~120 Hz */
    StartRCnt(RCntCNT2);

    WessTimerActive = 1;

    ExitCriticalSection();
}

void exit_WessTimer(void)
{
    EnterCriticalSection();

    WessTimerActive = 0;

    DisableEvent(EV2);
    CloseEvent(EV2);

    ExitCriticalSection();
}
