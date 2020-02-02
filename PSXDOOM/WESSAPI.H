    /*------------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                    Application Programming Interface Routines
                            by Scott Patterson
    */
    /*------------------------------------------------------------------*/

#ifndef _WESSAPI_H
#define _WESSAPI_H

#include "wessarc.h"
    /*------------------------------------------------------------------*/
    /*
        Enumerated types.
    */
    /*------------------------------------------------------------------*/

enum Wess_Error {

    wess_NO_ERROR,
    wess_FOPEN,
    wess_FREAD,
    wess_FSEEK
};

/* used by wess_exit */
enum RestoreFlag {NoRestore,YesRestore};

/* used by wess pause functions */
enum MuteFlag {NoMute,YesMute};

typedef struct NoteData {
    short  seq_num;					//*4
	short  track;					//*6
	char  keynum;					//*8
	char  velnum;					//*9
	short   pad1;					//*10
	patchmaps_header *patchmap;		//*12
	patchinfo_header *patchinfo;	//*16
}NoteData;

typedef struct NoteState {
    int      numnotes;
    NoteData nd[24];
}NoteState;

/* used by wess_sequence_status function */
enum SequenceStatus {SEQUENCE_INVALID,SEQUENCE_INACTIVE,SEQUENCE_STOPPED,SEQUENCE_PLAYING};

/* used by wess trigger functions */
enum LoopedFlag {NoLooped,YesLooped};

/* used by wess trigger functions */
enum HandleFlag {NoHandle,YesHandle};

extern pmasterstat *pm_stat;


    /*------------------------------------------------------------------*/
    /*
        Externs.
    */
    /*------------------------------------------------------------------*/

    /*
        extern: millicount

        - a millisecond counter used by the sound system and available
          to programmers for timing measurements
    */

extern volatile unsigned long millicount;
extern volatile int T1counter;
extern volatile int SeqOn;

    /*------------------------------------------------------------------*/
    /*
        System Setup and Shutdown Functions.
    */
    /*------------------------------------------------------------------*/

    /*
        routine: wess_install_error_handler()

        - for installing an error callback to notify file access errors
        - module is your own ID returned as err_module parameter
        - err_enum is the returned Wess_Error enum parameter
    */

extern void wess_install_error_handler (int (*error_func)(int err_module, int err_enum),
                                        int   module);

    /*
        routine: wess_install_handler()

        - the sound system timing handler can be installed independent
          of the sound system for convenience.
        - this can be useful for having the millicount running before
          sound system initialization by wess_init()
    	- does not install if already installed.
    */

extern void wess_install_handler (void);

    /*
        routine: wess_restore_handler()

        - restores timing handler to original system control
        - only useful when wess_exit(NoRestore) has been called
        - does not restore if already restored.
    */

extern void wess_restore_handler (void);

    /*
        routine: wess_init()

        - initialize the Williams Entertainment Sound System.
        - wess_install_handler() will be called if it hasn't been
          called previously.
    */

extern int wess_init (void);

    /*
        routine: wess_exit()

        - shuts down the Williams Entertainment Sound System.
        - if a module is loaded, calls wess_unload_module().
        - if wess_exit(YesRestore) is called, wess_restore_handler()
          will be called after the sequencer engine is shutdown
        - to have millicount running after sound system shutdown then
          call wess_exit(NoRestore)
    */

extern void wess_exit (enum RestoreFlag rflag);

    /*
        routine: wess_load_module()

        - loads master table of sounds and sequences
        - returns 0 for failure, 1 for success
        - NULL for memory_pointer results in internal memory allocation
          of memory_allowance bytes
        - if memory_pointer!=NULL then it is assumed to be a pointer
          to memory_allocation bytes of memory for use by the sound system
        - enables sequencer engine
    */

extern int wess_load_module (void *wmd_ptr,
                             char *memory_pointer,
                             int   memory_allowance,
                             int **settings_tag_lists);

    /*
        routine: wess_unload_module()

        - disables sequencer engine
        - frees any allocated memory
    */

extern void wess_unload_module (void);

    /*
        routine: wess_get_wmd_start()

        - after call to wess_load_module is successful, this
          gets the pointer to beginning of module block
    */

extern char *wess_get_wmd_start (void);

    /*
        routine: wess_get_wmd_end()

        - after call to wess_load_module is successful, this
          gets the pointer past end of module block
    */

extern char *wess_get_wmd_end (void);

    /*------------------------------------------------------------------*/
    /*
        Sequencer calls.
    */
    /*------------------------------------------------------------------*/

    /*
        routine: wess_seq_trigger()

    	- a sequence is everything from a single sound to a music sequence.
        - multiple sequences can be called simultaneously
        - dynamic sequence, track, and voice allocation is managed
          by the sequencer
        - tracks have priorty assignments and voices triggered by a
          track inheirit its priority
        - you can trigger multiple instances of a sequence with multiple
          trigger calls
    */

#define TRIGGER_VOLUME   (0x1L<< 0)
#define TRIGGER_PAN      (0x1L<< 1)
#define TRIGGER_PATCH    (0x1L<< 2)
#define TRIGGER_PITCH    (0x1L<< 3)
#define TRIGGER_MUTEMODE (0x1L<< 4)
#define TRIGGER_TEMPO    (0x1L<< 5)
#define TRIGGER_TIMED    (0x1L<< 6)
#define TRIGGER_LOOPED   (0x1L<< 7)
#define TRIGGER_REVERB   (0x1L<< 8)

typedef struct  {
                    unsigned long   mask;
                    unsigned char   volume;   /* 0-127 */
                    unsigned char   pan;      /* 0-127, 64 center */
                    short           patch;    /* 0-32767 */
                    short           pitch;    /* -8192 to 8191 */
                    unsigned char   mutemode; /* 0-7 */
                    unsigned char   reverb;
                    unsigned short  tempo;
                    unsigned long   timeppq;
                } TriggerPlayAttr;

    /* the basic sequence trigger call */

extern void wess_seq_trigger         (int seq_num);

    /* override masked sequence parameters */

extern void wess_seq_trigger_special (int              seq_num,
                                      TriggerPlayAttr *attr);

    /* set your own type number to the sequence */

extern void wess_seq_trigger_type         (int           seq_num,
                                           unsigned long seq_type);

    /* set your own type number to the sequence and
       override masked sequence parameters          */

extern void wess_seq_trigger_type_special (int              seq_num,
                                           unsigned long    seq_type,
                                           TriggerPlayAttr *attr);

extern void wess_seq_update_type_special  (unsigned long    seq_type,
                                           TriggerPlayAttr *attr);

    /*
        routine: wess_seq_status()

        - find out if any instances of sequences_number are running
        - returns 0 SEQUENCE_INVALID for no such sequence
                  1 SEQUENCE_INACTIVE for is not being processed
                  2 SEQUENCE_STOPPED for sequence is stopped
                  3 SEQUENCE_PLAYING for sequence is playing
    */

extern int wess_seq_status (int sequence_number);

    /*
        routine: wess_seq_stop()

        - stops all instances of a specified sequence
    */

extern void wess_seq_stop (int sequence_number);

    /*
        routine: wess_seq_stoploop()

        - turns off track loops of all instances of a specified sequence
          which were started with wess_seq_trigger_looped
    */

extern void wess_seq_stoploop (int sequence_number);//no used

    /*
        routine: wess_seq_stoptype()

    	- immediate stop of all sequences that were registered as a certain
          type by wess_seq_trigger_type
    */

extern void wess_seq_stoptype (unsigned long sequence_type);

    /*
        routine: wess_seq_stopall()

        - immediate stop of all sequences
    */

extern void wess_seq_stopall (void);

    /*
        routine: wess_seq_pause()

        - immediately pause a sequence
        - if MuteFlag is YesMute, all playing voices are muted
        - if MuteFlag is NoMute, all playing voices do note decay
    */

extern void wess_seq_pause (int           sequence_number,
                            enum MuteFlag mflag);

    /*
        routine: wess_seq_restart()

        - restarts a paused sequence
    */

extern void wess_seq_restart (int sequence_number);

    /*
        routine: wess_seq_stopall()

        - immediate pause of all sequences
        - if mflag is YesMute, all playing voices are muted
        - if mflag is NoMute, all playing voices remain on
        - if pns is not NULL, any music muting is recorded
    */

extern void wess_seq_pauseall (enum MuteFlag mflag, NoteState *pns);

    /* functions used internally for mute status */
extern void start_record_music_mute(NoteState *pns);
extern void end_record_music_mute(void);

    /*
        routine: wess_seq_stopall()

        - restart all paused sequences
        - if pns is not NULL, music unmuting is played
    */

extern void wess_seq_restartall (NoteState *pns);

    /*
        routine: wess_register_callback()

        - callbacks are useful for video and program flow synchronization
          with audio.
        - callbacks are called when any track of a playing sequence
          encounters a StatusMark with a registered marker_ID
        - the first parameter of the callback is the marker_ID encountered
        - the second parameter is the StatusMark's data field
    */

extern void wess_register_callback (char   marker_ID,
                                    void (*function_pointer)(char,short));

    /*
        routine: wess_delete_callback()

        - stop any callbacks from a particular marker_ID
    */

extern void wess_delete_callback (char marker_ID);

    /*------------------------------------------------------------------*/
    /*
        Master functions get and set global sound parameters.
    */
    /*------------------------------------------------------------------*/

    /*
        routine: wess_master_sfx_volume_get()
        routine: wess_master_mus_volume_get()

        - gets the master volume
    */

extern char wess_master_sfx_volume_get (void);
extern char wess_master_mus_volume_get (void);

    /*
        routine: wess_master_sfx_vol_set()
        routine: wess_master_mus_vol_set()

        - sets the master volume
    */

extern void wess_master_sfx_vol_set (char volume);
extern void wess_master_mus_vol_set (char volume);

    /*
        routine: wess_pan_mode_get()

        - gets the pan mode where 0=off, 1=normal, 2=switchLR
    */

extern char wess_pan_mode_get (void);

    /*
        routine: wess_pan_mode_set()

        - sets the pan mode where 0=off, 1=normal, 2=switchLR
    */

extern void wess_pan_mode_set (char mode);

    /*
        routine: psx_set_mute_release(millisec)

        - sets the stop/mute release time given milliseconds
    */

extern void psx_set_mute_release(int millisec);

    /*
        routine: wess_master_status()

        - system status includes drivers loaded, patches loaded, sequences
          loaded, and memory locations.
    */

extern void * wess_get_master_status (void);


extern void wess_low_level_init(void);

#endif




