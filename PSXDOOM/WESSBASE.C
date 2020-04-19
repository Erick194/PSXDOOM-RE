// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include <libspu.h>
#include "psxspu.h"
#include "wessapi.h"
#include "wessarc.h"
#include "lcdload.h"

// Variables Aquí para que funcionen correctamante con WessInterruptHandler


int psxsettings[SNDHW_TAG_MAX*2] = //800756EC
{
        SNDHW_TAG_DRIVER_ID, PSX_ID,
        SNDHW_TAG_SOUND_EFFECTS, 1,
        SNDHW_TAG_MUSIC, 1,
        SNDHW_TAG_DRUMS, 1,
        SNDHW_TAG_END, 0
};

int *settings_list[2] =
{
        psxsettings,
        0
};

SampleBlock doomsfxblk;//8007eac0
SampleBlock mapsfxblk;//8007ec54

NoteState tmp_notestate;//0x8007e93c


//WESSAPI.C-------------------------------------------
int num_sd = 0;                         //80075714
char *wmd_file_ptr = 0;                 //80075718
char *wmd_file_ptr2 = 0;                //8007571C
Wess_File_IO_Struct *fp_wmd_file = 0;   //80075720
int sysinit = 0;                        //80075724
int module_loaded = 0;                  //80075728
int early_exit = 0;                     //8007572C
int max_seq_num = 0;                    //80075730
int mem_limit = 0;                      //80075734
patch_group_header scratch_pat_grp_hdr; //8007EDE8
track_header scratch_trk_hdr;           //8007EE04
int wmd_mem_is_mine = 0;                //80075738
char *wmd_mem = 0;                      //8007573C
char *wmd_end = 0;                      //80075740
int wmd_size = 0;                       //80075744
int (*Error_func)(int, int) = 0;        //80075748
int Error_module = 0;                   //8007574C
pmasterstat *pm_stat;                   //800A8538

//WESSARC.C-------------------------------------------
int SeqOn = 0;
unsigned long millicount = 0;
unsigned long millisum = 0;

int WessTimerActive = 0;

int T2counter = 0;

#define CHUNKSIZE 2048
char chunk[CHUNKSIZE];//80096394
char chunk2[CHUNKSIZE];//80096BA4

int ReadChunk1 = 0;//*L8007EE70
int ReadChunk2 = 0;//*L8007EE1C

//LCDLOAD.C-------------------------------------------

int lcd_open = 0;	                //L800758F4
char *lcd_ppat_data_patches = 0;    //L800758F8
char *lcd_ppat_data_patchmaps = 0;  //L800758FC
char *lcd_ppat_data_patchinfo = 0;  //L80075900
char *lcd_ppat_data_track = 0;      //L80075904
patch_group_data *lcd_ppat_info = 0;//L80075908;
unsigned long lcd_sample_pos = 0;   //L8007590C
unsigned int lcd_sectorbuf_pos = 0; //L80075910
unsigned int lcd_totaltogo = 0;     //L80075914

short lcd_sectorbuf_code = 0;       //L80075918
char *lcd_sectorbuf;                //L8007591C

int lcd_cd_status = 0;              //L80075920

int  lcd_sync_intr = 0;             //L80075924
char lcd_sync_result[8] = { 0,0,0,0,0,0,0,0 };//0x80075928

Wess_Data_IO_Struct lcd_data_fileref;//0x8007f104

//SEQLOAD.C-------------------------------------------
int seq_loader_enable = 0;          //80075790
char *loaderfile = 0;		        //80075794
pmasterstat *ref_pm_stat = 0;       //80075798
int ref_max_seq_num = 0;            //8007579C
char Driver_Init = DriverInit;      //800757A0
char Reset_Gates = ResetGates;      //800757A1
int	opencount = 0;				    //800757A4
int(*Seq_Error_func)(int, int) = 0; //800757A8
int Seq_Error_module = 0;           //800757AC
Wess_File_IO_Struct *fp_seq_file = 0;//800757B0

track_header seq_track_header;      //8007EE74
track_header base_track_header;     //8007EE8C

//PSXSPU.C-------------------------------------------
#define MALLOC_MAX 1
char spu_malloc_rec[SPU_MALLOC_RECSIZ * (MALLOC_MAX + 1)];

unsigned int psxspu_initialized = 0;
unsigned int psxspu_status = 0;
unsigned long end_of_sram = 0x7F000;

int psxspu_master_vol = 0x3FFF;
int psxspu_master_fadecount = 0;
int psxspu_master_vol_fixed = 0x3FFF0000;
int psxspu_master_destvol = 0;
int psxspu_master_changeval = 0;

int psxspu_cd_vol = 0x3CFF;
int psxspu_cd_fadecount = 0;
int psxspu_cd_vol_fixed = 0x3FFF0000;
int psxspu_cd_destvol = 0;
int psxspu_cd_changeval =  0;

SpuReverbAttr rev_attr;//8007EEA4

//WESSSEQ.C-------------------------------------------
unsigned char skip_table[72] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
0x02, 0x03, 0x02, 0x04, 0x05, 0x05, 0x02, 0x02,
0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03,
0x03, 0x01, 0x01, 0x01, 0x00, 0x01, 0x02, 0x03,
0x04, 0x07, 0x06, 0x16, 0x17, 0x18, 0x15, 0x1A,
0x1E, 0x19, 0x1B, 0x1D, 0x1C, 0x1F, 0x05, 0x0F,
0x08, 0x09, 0x12, 0x11, 0x10, 0x14, 0x0D, 0x0B,
0x22, 0x23, 0x13, 0x0C, 0x0A, 0x20, 0x21, 0x0E
};

unsigned char	master_sfx_volume = 0x7f;	//*L80075834
unsigned char	master_mus_volume = 0x7f;	//*L80075835
unsigned char	pan_status = 0x1;			//*L80075836
unsigned char	release_rate = 0xD;			//*L80075837

unsigned long	voice_flags1 = 0;			//*L80075838
unsigned long	voice_flags2 = 0;			//*L8007583C
NoteState		*ptr_notestate = 0;			//*L80075840
voice_status	*ptr_pvoicestattbl = 0;		//*L80075844
unsigned long	priority = 0x100;			//*L80075848
unsigned long	max_pabstime = 0xFFFFFFFF;  //*L8007584C

//-----------------------------------------------------------
// Driver System
//-----------------------------------------------------------

master_status_structure		*drv_pmsbase;			//L8007EF88
sequence_status				*drv_pssbase;		    //L8007EF8C
track_status				*drv_ptsbase;		    //L8007EF90
unsigned long				 drv_hwvl;		        //L8007EF98
patch_group_data			*drv_ppgd;		        //L8007EF9C
unsigned long				*drv_pcurabstime;		//L8007EFA0
patches_header				*drv_patchesbase;		//L8007EFA4
patchmaps_header			*drv_patchmapsbase;		//L8007EFA8
patchinfo_header			*drv_samplesbase;		//L8007EFAC
char						*drv_drummapsbase;		//L8007EFB0
SpuVoiceAttr				drv_outattr;            //L8007EFB4

unsigned char				drv_nvst;		        //L8007EED0
unsigned char				drv_npti;	            //L8007EED4
unsigned long				drv_cnt;				//L8007EED8
voice_status				*drv_pvsbase;		    //L8007EEDC

voice_status				*drv_pcurvs;	        //L8007EF94

unsigned char				drv_channels[24];		//8007f00c

unsigned long				drv_pitch1;				//L8007EEB8
unsigned long				drv_pitch2;				//L8007EEBC
unsigned long				drv_pitch3;				//L8007EEC0
unsigned long				drv_volume_1;			//L8007EEC4
track_status				*drv_ptrk_stat;			//L8007EEC8
short				        drv_pan_1;				//L8007EECC

unsigned long		        drv_vn;			        //8007EEE0
unsigned long		        drv_vi;                 //8007EEE4
unsigned long		        drv_keychannels;        //8007EEE8
unsigned long		        drv_keycnt;			    //8007EEEC
voice_status		        *drv_pvs;			    //8007EEF0
unsigned long		        drv_voiceflag;          //8007EEF4
char				        drv_keysstatus[24];     //8007EFF4

sequence_status				*drv_pss;		        //L8007EEF8

unsigned long				drv_vn_2;		        //L8007EEFC
unsigned long				drv_vi_2;		        //L8007EF00
voice_status				*drv_pvs_2;			    //L8007EF04
sequence_status				*drv_pss_2;			    //L8007EF08

unsigned short				drv_thepatch;		    //L8007EF0C

unsigned long				drv_voices_active_3;	//L8007EF10
unsigned long				drv_hwvl_3;		        //L8007EF14
unsigned long				drv_pitch_1;			//L8007EF18
unsigned long				drv_pitch_2;			//L8007EF1C
unsigned long				drv_pitch_3;			//L8007EF20
voice_status				*drv_voice_stat_3;		//L8007EF24
short				        drv_pitch_cntrl;		//L8007EF28

unsigned long				drv_voices_active_4;	//L8007EF2C
unsigned long				drv_hwvl_4;		        //L8007EF30
unsigned long				drv_volume_2;			//L8007EF34
voice_status				*drv_voice_stat_4;		//L8007EF38
short				        drv_pan_2;				//L8007EF3C

unsigned long				drv_voices_active_5;	//L8007EF40
unsigned long				drv_hwvl_5;		        //L8007EF44
unsigned long				drv_volume_3;			//L8007EF48
voice_status				*drv_voice_stat_5;		//L8007EF4C
short				        drv_pan_3;				//L8007EF50

track_status				*drv_ptrk_stat_1;		//L8007EF54

unsigned long				drv_hwvl_6;		        //L8007EF58
unsigned long				drv_note_stat;			//L8007EF5C
voice_status				*drv_voice_stat_6;		//L8007EF60

unsigned long				drv_patch_cnt;			//L8007EF64
unsigned char				drv_byte01;				//L8007EF68
unsigned char				drv_byte02;				//L8007EF6C
unsigned char				drv_patchmap_cnt;		//L8007EF70
patches_header				*drv_patch_1;			//L8007EF74
patchmaps_header			*drv_patchmap_1;		//L8007EF78
patchinfo_header			*drv_samplesbase_1;	    //L8007EF7C

unsigned long				drv_hwvl_7;		        //L8007EF80
voice_status			    *drv_voice_stat_7;		//L8007EF84

unsigned int data01;        //L8007F024
unsigned char data02;       //L8007F028

//-----------------------------------------------------------
// Engine System
//-----------------------------------------------------------

unsigned char			 eng_ntwa;	            //L800758E4
track_status			*eng_ptsbase;	        //L800758E8
sequence_status			*eng_pssbase;	        //L800758EC
master_status_structure *eng_pmsbase;		    //L800758F0

sequence_status			*eng_seq_stat_1;		//L8007F02C
char					*eng_ptrk_indxs1;		//L8007F030
unsigned long			eng_max_trks_perseq1;	//L8007F034

unsigned char			eng_patchnum;			//L8007F038
short			        eng_pitch_cntrl;		//L8007F03C

unsigned char			eng_volume_cntrl;		//L8007F040

unsigned char			eng_pan_cntrl;			//L8007F044

unsigned char			eng_callback_areas;	    //L8007F048
unsigned char			eng_callbacks_active;   //L8007F04C
callback_status			*eng_pcalltable;	    //L8007F050

short			        eng_labelnum;	        //L8007F054
char					*eng_plabellist;        //L8007F058
char					*eng_pgates;	        //L8007F05C

char					*eng_piters;	        //L8007F068
short			        eng_labelnum2;	        //L8007F060
char					*eng_plabellist2;       //L8007F064

unsigned char			eng_gates_per_seq;	    //L8007F06C
char					*eng_pgates2;		    //L8007F070

unsigned char			eng_iters_per_seq;	    //L8007F074
char					*eng_piters2;		    //L8007F078

char					*eng_piters3;		    //L8007F07C

unsigned short			eng_tracks;				//L8007F080
unsigned char			eng_tracks_active2;		//L8007F084
unsigned char			*eng_ptrk_indxs2;		//L8007F088
track_status			*eng_ptrk_stat_1;		//L8007F08C
sequence_status			*eng_seq_stat_2;		//L8007F090

short			        eng_labelnum3;			//L8007F094
unsigned short			eng_tracks2;			//L8007F098
char					*eng_plabellist3;		//L8007F09C
unsigned char			eng_tracks_active3;		//L8007F0A0
unsigned char			*eng_ptrk_indxs3;		//L8007F0A4
track_status			*eng_ptrk_stat_2;		//L8007F0A8
sequence_status			*eng_seq_stat_3;		//L8007F0AC

short			        eng_labelnum4;			//L8007F0B0
unsigned short			eng_tracks3;			//L8007F0B4
char					*eng_plabellist4;		//L8007F0B8
unsigned char			eng_tracks_active4;		//L8007F0BC
unsigned char			*eng_ptrk_indxs4;		//L8007F0C0
track_status			*eng_ptrk_stat_3;		//L8007F0C4
sequence_status			*eng_seq_stat_4;		//L8007F0C8

unsigned short			eng_tracks4;			//L8007F0CC
unsigned char			eng_tracks_active5;		//L8007F0D0
unsigned char			*eng_ptrk_indxs5;		//L8007F0D4
track_status			*eng_ptrk_stat_4;		//L8007F0D8
sequence_status			*eng_seq_stat_5;		//L8007F0DC

sequence_status			*eng_seq_stat_6;		//L8007F0E0
track_status			*eng_ptrk_stat_5;		//L8007F0E4
unsigned char			*eng_ptrk_indxs6;		//L8007F0E8
unsigned char			eng_tracks_active6;		//L8007F0EC
unsigned long			eng_max_trks_perseq;	//L8007F0F0

track_status *eng_tmp_ptrkstattbl;              //L8007F0F4
unsigned int eng_tmp_trks_active;               //L8007F0F8
unsigned int eng_tmp_trk_work_areas;            //L8007F0FC
char *eng_tmp_ppos;                             //L8007F100
