#ifndef _WESSARC_H
#define _WESSARC_H

//#include "psxspu.h"
#include "psxcd.h"

enum DriverIds {NoSound_ID,PSX_ID,GENERIC_ID=50};

enum SoundHardwareTags {
    SNDHW_TAG_END,//0
    SNDHW_TAG_DRIVER_ID,//1
    SNDHW_TAG_SOUND_EFFECTS,//2
    SNDHW_TAG_MUSIC,//3
    SNDHW_TAG_DRUMS,//4
    SNDHW_TAG_MAX//5
};

// Driver/sequencer command ids
enum DriverCmd {
    DriverInit          = 0,
    DriverExit          = 1,
    DriverEntry1        = 2,
    DriverEntry2        = 3,
    DriverEntry3        = 4,
    TrkOff              = 5,
    TrkMute             = 6,
    PatchChg            = 7,
    PatchMod            = 8,
    PitchMod            = 9,
    ZeroMod             = 10,
    ModuMod             = 11,
    VolumeMod           = 12,
    PanMod              = 13,
    PedalMod            = 14,
    ReverbMod           = 15,
    ChorusMod           = 16,
    NoteOn              = 17,
    NoteOff             = 18,
    StatusMark          = 19,
    GateJump            = 20,
    IterJump            = 21,
    ResetGates          = 22,
    ResetIters          = 23,
    WriteIterBox        = 24,
    SeqTempo            = 25,
    SeqGosub            = 26,
    SeqJump             = 27,
    SeqRet              = 28,
    SeqEnd              = 29,
    TrkTempo            = 30,
    TrkGosub            = 31,
    TrkJump             = 32,
    TrkRet              = 33,
    TrkEnd              = 34,
    NullEvent           = 35
};

//LCD FILE Structs
typedef struct
{
	unsigned short	patchmap_cnt;	//r*
	unsigned short	patchmap_idx;	//r*2
}patches_header;
//int i = sizeof(patches_header);

typedef struct
{
	unsigned char priority;			//*
	unsigned char reverb;			//*1
	unsigned char volume;			//*2
	unsigned char pan;				//*3
	unsigned char root_key;			//*4
	unsigned char fine_adj;			//*5
	unsigned char note_min;			//*6
	unsigned char note_max;			//*7
	unsigned char pitchstep_min;	//*8
	unsigned char pitchstep_max;	//*9
	unsigned short sample_id;		//*10
	unsigned short adsr1;			//*12
	unsigned short adsr2;			//*14
}patchmaps_header;
//int i = sizeof(patchmaps_header);

typedef struct
{
	unsigned int	sample_offset;	//r*
	unsigned int	sample_size;	//r*4
	unsigned long	sample_pos;		//r*8
}patchinfo_header;
//int i = sizeof(patchinfo_header);

//*******************************

typedef struct
{
	int				module_id_text;		//*
	unsigned int	module_version;		//*4
	unsigned short	sequences;			//*8
	unsigned char	patch_types_infile;	//*10
	unsigned char	seq_work_areas;		//*11
	unsigned char	trk_work_areas;		//*12
	unsigned char	gates_per_seq;		//*13
	unsigned char	iters_per_seq;		//*14
	unsigned char	callback_areas;		//*15
}module_header;

typedef struct
{
	unsigned char	voices_type;		//*
	unsigned char	voices_max;			//*1
	unsigned char	priority;			//*2
	unsigned char	lockchannel;		//*3
	unsigned char	voices_class;		//*4
	unsigned char	reverb;				//*5
	unsigned short	initpatchnum;		//*6
	unsigned short	initpitch_cntrl;	//*8
	unsigned char	initvolume_cntrl;	//*10
	unsigned char	initpan_cntrl;		//*11
	unsigned char	substack_count;		//*12
	unsigned char	mutebits;			//*13
	unsigned short	initppq;			//*14
	unsigned short	initqpm;			//*16
	unsigned short	labellist_count;	//*18
	unsigned int	data_size;			//*20
}track_header;//size 24 bytes

typedef struct
{
	track_header	trk_hdr;
	unsigned long   *plabellist;//*24
	char			*ptrk_data;//*28 ??
}track_data;//32

typedef struct
{
	unsigned short tracks;	//*
	unsigned short unk1;	//*2
}seq_header;

typedef struct
{
	seq_header		seq_hdr;		//*
	track_data		*ptrk_info;		//*4
	unsigned int	fileposition;	//*8
	unsigned int	trkinfolength;	//*12
	unsigned int	trkstoload;		//*16
}sequence_data;//debe medir 20?

typedef struct
{
	module_header	mod_hdr;			//*
	sequence_data	*pseq_info;			//*16
}module_data;

typedef void (*callfunc_t)();
typedef struct
{
	unsigned char	active;				//*
	unsigned char	type;				//*1
	unsigned short	curval;				//*2
	callfunc_t      callfunc;			//*4
}callback_status;

typedef struct
{
	unsigned int	load_flags;			//*
	unsigned char	patch_id;			//*4
	unsigned char	hw_voice_limit;		//*5
	unsigned short	pad1;				//*6
	unsigned short	patches;			//*8
	unsigned short	patch_size;			//*10
	unsigned short	patchmaps;			//*12
	unsigned short	patchmap_size;		//*14
	unsigned short	patchinfo;			//*16
	unsigned short	patchinfo_size;		//*18
	unsigned short	drummaps;			//*20
	unsigned short	drummap_size;		//*22
	unsigned int	extra_data_size;	//*24
}patch_group_header;

typedef struct
{
	int hardware_ID;//76
	int flags_load;//80
}hardware_table_list;

typedef struct
{
	patch_group_header pat_grp_hdr;		//*
	char				*ppat_data;		//*28
	int			 data_fileposition;		//*32
	int				sndhw_tags[10];		//*36
	hardware_table_list	hw_tl_list;		//*76
}patch_group_data;//size 84 bytes


#define SEQ_STATE_STOPPED 0
#define SEQ_STATE_PLAYING 1

//track
#define TRK_ACTIVE	1
#define TRK_MUTE	2
#define TRK_HANDLED	4
#define TRK_STOPPED	8
#define TRK_TIMED	16
#define TRK_LOOPED	32
#define TRK_SKIP	64
#define TRK_OFF	    128

//seq
#define SEQ_ACTIVE	1
#define SEQ_HANDLE	2

//voice
#define VOICE_ACTIVE	1
#define VOICE_RELEASE	2

typedef struct
{
	unsigned char	flags;			//*
	unsigned char	playmode;		//*1
	unsigned short	seq_num;		//*2
	unsigned char	tracks_active;	//*4
	unsigned char	tracks_playing;	//*5
	unsigned char	volume;			//*6
	unsigned char	pan;			//*7
	unsigned int	seq_type;		//*8
	char			*ptrk_indxs;	//*12
	char			*pgates;		//*16
	char			*piters;		//*20
}sequence_status;

typedef struct
{
	unsigned char	flags;	        //*     //0
	unsigned char	refindx;		//*1
	unsigned char	seq_owner;		//*2
	unsigned char	patchtype;		//*3
	unsigned int	deltatime;		//*4    //1
	unsigned char	priority;		//*8    //2
	unsigned char	reverb;			//*9
	unsigned short	patchnum;		//*10
	unsigned char	volume_cntrl;	//*12   //3
	unsigned char	pan_cntrl;		//*13
	short	        pitch_cntrl;	//*14
	unsigned char	voices_active;	//*16   //4
	unsigned char	voices_max;		//*17
	unsigned char	mutemask;		//*18
	unsigned char	sndclass;		//*19
	unsigned short	ppq;			//*20   //5
	unsigned short	qpm;			//*22
	unsigned short	labellist_count;//*24   //6
	unsigned short	labellist_max;	//*26
	unsigned long	ppi;			//*28   //7
	unsigned long	starppi;		//*32   //8
	unsigned int	accppi;			//*36   //9
	unsigned int	totppi;			//*40   //10
	unsigned int	endppi;			//*44   //11
	unsigned char	*pstart;		//*48   //12
	unsigned char	*ppos;			//*52   //13
	unsigned long   *plabellist;	//*56   //14
	unsigned char   **psubstack;	//*60   //15
	unsigned char   **psp;			//*64   //16
	unsigned char   **pstackend;	//*68   //17
	unsigned int	data_size;		//*72   //18
	unsigned int	data_space;		//*76   //19
}track_status;

typedef struct
{
	unsigned char	    flags;			//*     //0
	unsigned char	    patchtype;		//*1
	unsigned char	    refindx;		//*2
	unsigned char	    track;			//*3
	unsigned char	    priority;		//*4    //1
	unsigned char	    keynum;			//*5
	unsigned char	    velnum;			//*6
	unsigned char	    sndtype;			//*7
	patchmaps_header   *patchmaps;	//*8    //2
	patchinfo_header   *patchinfo;	//*12   //3
	unsigned long	    pabstime;		//*16   //4
	unsigned long	    adsr2;			//*20   //5
}voice_status;//size 24 bytes

typedef struct
{
	unsigned long		*pabstime;				//*
	unsigned char		seqs_active;			//*4
	unsigned char		trks_active;			//*5
	unsigned char		voices_active;			//*6
	unsigned char		voices_total;			//*7
	unsigned char		patch_types_loaded;		//*8
	unsigned char		unk1;					//*9
	unsigned char		callbacks_active;		//*10
	unsigned char		unk3;					//*11
	module_data			*pmod_info;				//*12
	callback_status		*pcalltable;			//*16
	char				*pmaster_volume;		//*20
	patch_group_data	*ppat_info;				//*24
	unsigned long		max_trks_perseq;		//*28
	sequence_status		*pseqstattbl;			//*32
	unsigned long		max_substack_pertrk;	//*36
	track_status		*ptrkstattbl;			//*40
	unsigned long		max_voices_pertrk;		//*44
	voice_status		*pvoicestattbl;			//*48
	//char				*ptr_module;			//*52
	unsigned long		fp_module;			    //*52
}master_status_structure;

typedef master_status_structure pmasterstat;

#define WESS_SSSP_TEXT		0x58535053//'SPSX'//0x58535053 //SPSX
#define WESS_CORE_VERSION	0x01

#define SNDFX_CLASS		0
#define MUSIC_CLASS		1
#define DRUMS_CLASS		2
#define SFXDRUMS_CLASS	3

#define TAG_SOUND_EFFECTS 1
#define TAG_MUSIC         2
#define TAG_DRUMS         4

#define LOAD_PATCHES	1
#define LOAD_PATCHMAPS	2
#define LOAD_PATCHINFO	4
#define LOAD_DRUMMAPS	8
#define LOAD_EXTRADATA	16

///----------------------------------
extern void (**CmdFuncArr[10])(track_status *);

extern volatile int SeqOn;
extern volatile unsigned long millicount;
extern volatile int WessTimerActive;

typedef PsxCd_File Wess_File_IO_Struct;
typedef PsxCd_File Wess_Data_IO_Struct;

extern short GetIntsPerSec(void);
extern unsigned long CalcPartsPerInt(short ips,short ppq,short qpm);

extern void init_WessTimer(void);
extern void exit_WessTimer(void);

extern void wess_low_level_exit(void);
extern void *wess_malloc(int size);
extern void wess_free(char *mem);

#endif // _WESSARC_H
