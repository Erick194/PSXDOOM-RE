// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 32/01/2020 [GEC]

#include "wessseq.h"

void start_record_music_mute(NoteState *notestate);
void end_record_music_mute(void);
void add_music_mute_note(unsigned short seq_num, unsigned char track,
                  unsigned char keynum, unsigned char velnum,
                  patchmaps_header *patchmap, patchinfo_header *patchinfo);
void wess_set_mute_release(int newrate);
void TriggerPSXVoice(voice_status *voice_stat, unsigned char keynum, unsigned char velnum);

void PSX_DriverInit(master_status_structure *pm_stat);
void PSX_DriverExit (track_status *ptk_stat);
void PSX_DriverEntry1 (track_status *ptk_stat);
void PSX_DriverEntry2(track_status *ptk_stat);
void PSX_DriverEntry3(track_status *ptk_stat);
void PSX_TrkOff(track_status *ptk_stat);
void PSX_TrkMute(track_status *ptk_stat);
void PSX_PatchChg(track_status *ptk_stat);
void PSX_PatchMod(track_status *ptk_stat);
void PSX_PitchMod(track_status *ptk_stat);
void PSX_ZeroMod(track_status *ptk_stat);
void PSX_ModuMod(track_status *ptk_stat);
void PSX_VolumeMod(track_status *ptk_stat);
void PSX_PanMod(track_status *ptk_stat);
void PSX_PedalMod(track_status *ptk_stat);
void PSX_ReverbMod(track_status *ptk_stat);
void PSX_ChorusMod(track_status *ptk_stat);
void PSX_voiceon(voice_status *voice_stat, track_status *ptk_stat,
	           patchmaps_header *patchmaps, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum);
void PSX_voiceparmoff(voice_status *voice_stat);
void PSX_voicerelease(voice_status *voice_stat);
void PSX_voicenote(track_status *ptk_stat,
	           patchmaps_header *patchmap, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum);
void PSX_NoteOn(track_status *ptk_stat);
void PSX_NoteOff(track_status *ptk_stat);

void(*drv_cmds[19])() =
{
	PSX_DriverInit,         //0x0
	PSX_DriverExit,         //0x1
	PSX_DriverEntry1,       //0x2
	PSX_DriverEntry2,       //0x3
	PSX_DriverEntry3,       //0x4
	PSX_TrkOff,             //0x5
	PSX_TrkMute,            //0x6
	PSX_PatchChg,           //0x7
	PSX_PatchMod,           //0x8
	PSX_PitchMod,           //0x9
	PSX_ZeroMod,            //0xA
	PSX_ModuMod,            //0xB
	PSX_VolumeMod,          //0xC
	PSX_PanMod,             //0xD
	PSX_PedalMod,           //0xE
	PSX_ReverbMod,          //0xF
	PSX_ChorusMod,          //0x10
	PSX_NoteOn,             //0x11
	PSX_NoteOff             //0x12
};

extern unsigned char                skip_table[72];         //80075930

extern unsigned char	            master_sfx_volume;	    //80075834
extern unsigned char	            master_mus_volume;	    //80075835
extern unsigned char	            pan_status;			    //80075836
extern unsigned char	            release_rate;			//80075837

extern unsigned long	            voice_flags1;			//80075838
extern unsigned long	            voice_flags2;			//8007583C
extern NoteState		            *ptr_notestate;			//80075840
extern voice_status	                *ptr_pvoicestattbl;		//80075844
extern unsigned long	            priority;			    //80075848
extern unsigned long	            max_pabstime;           //8007584C

extern master_status_structure		*drv_pmsbase;			//8007EF88
extern sequence_status				*drv_pssbase;		    //8007EF8C
extern track_status				    *drv_ptsbase;		    //8007EF90
extern unsigned long				drv_hwvl;		        //8007EF98
extern patch_group_data			    *drv_ppgd;		        //8007EF9C
extern unsigned long				*drv_pcurabstime;		//8007EFA0
extern patches_header				*drv_patchesbase;		//8007EFA4
extern patchmaps_header			    *drv_patchmapsbase;		//8007EFA8
extern patchinfo_header			    *drv_samplesbase;		//8007EFAC
extern char						    *drv_drummapsbase;		//8007EFB0
extern SpuVoiceAttr				    drv_outattr;            //8007EFB4

extern unsigned char				drv_nvst;		        //8007EED0
extern unsigned char				drv_npti;               //8007EED4
extern unsigned long				drv_cnt;				//8007EED8
extern voice_status				    *drv_pvsbase;		    //8007EEDC

extern voice_status				    *drv_pcurvs;	        //8007EF94

extern unsigned char				drv_channels[24];		//8007f00C

extern void (**CmdFuncArr[10])(track_status *);
extern void(*DrvFunctions[36])();

//-----------------------------------------------------------
// Sound System
//-----------------------------------------------------------
void start_record_music_mute(NoteState *notestate)//L80045750()
{
    ptr_notestate = notestate;
    if (notestate != 0)
    {
        notestate->numnotes = 0;
    }
}

void end_record_music_mute(void)//L8004576C()
{
	ptr_notestate = 0;
}

void add_music_mute_note(unsigned short seq_num, unsigned char track,
                  unsigned char keynum, unsigned char velnum,
                  patchmaps_header *patchmap, patchinfo_header *patchinfo)//L8004577C()
{
	if (ptr_notestate != 0)
	{
		ptr_notestate->nd[ptr_notestate->numnotes].seq_num = seq_num;
		ptr_notestate->nd[ptr_notestate->numnotes].track = track;
		ptr_notestate->nd[ptr_notestate->numnotes].keynum = keynum;
		ptr_notestate->nd[ptr_notestate->numnotes].velnum = velnum;
		ptr_notestate->nd[ptr_notestate->numnotes].patchmap = patchmap;
		ptr_notestate->nd[ptr_notestate->numnotes].patchinfo = patchinfo;
		ptr_notestate->numnotes++;
	}
}

void wess_set_mute_release(int newrate)//L80045848()
{
    int maxrate;

    maxrate = 0x10000000;
    release_rate = 0x1f;
    while ((newrate < maxrate && (release_rate != 0)))
    {
        maxrate >>= 1;
        release_rate -= 1;
    }
    return;
}

extern unsigned long				drv_pitch1;				//L8007EEB8
extern unsigned long				drv_pitch2;				//L8007EEBC
extern unsigned long				drv_pitch3;				//L8007EEC0
extern unsigned long				drv_volume_1;			//L8007EEC4
extern track_status				    *drv_ptrk_stat;			//L8007EEC8
extern short				        drv_pan_1;				//L8007EECC

void TriggerPSXVoice(voice_status *voice_stat, unsigned char keynum, unsigned char velnum)//L8004587C()
{
	unsigned long volume;
	unsigned long pitch;

	drv_outattr.voice = 1 << (voice_stat->refindx & 31);
	drv_outattr.mask = (SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_NOTE | SPU_VOICE_SAMPLE_NOTE | SPU_VOICE_WDSA | SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2);
    //printf("Voice pos %d\n",drv_outattr.voice);

	drv_ptrk_stat = (drv_ptsbase+voice_stat->track);

	if (drv_ptrk_stat->reverb != 0)
	{
		if (drv_channels[voice_stat->refindx] == 0)
		{
			SpuSetReverbVoice(SPU_ON, drv_outattr.voice);
			drv_channels[voice_stat->refindx] = 0x7f;
		}
	}
	else
	{
		if (drv_channels[voice_stat->refindx] != 0)
		{
			SpuSetReverbVoice(SPU_OFF, drv_outattr.voice);
			drv_channels[voice_stat->refindx] = 0;
		}
	}

	if (pan_status != 0)
	{
		drv_pan_1 = drv_ptrk_stat->pan_cntrl + voice_stat->patchmaps->pan - 0x40;

		if (drv_pan_1 < 0)      drv_pan_1 = 0;
        if (drv_pan_1 > 0x7F)	drv_pan_1 = 0x7F;
	}
	else
	{
		drv_pan_1 = 0x40;
	}

	//Volume
	if (drv_ptrk_stat->sndclass == SNDFX_CLASS)
	{
		volume = velnum * voice_stat->patchmaps->volume * drv_ptrk_stat->volume_cntrl * master_sfx_volume;
	}
	else
	{
		volume = velnum * voice_stat->patchmaps->volume * drv_ptrk_stat->volume_cntrl * master_mus_volume;
	}

	drv_volume_1 = volume >> 21;

	//Pan
	if (pan_status != 0)
	{
		if (pan_status != 1)//2
		{
			drv_outattr.volume.right = ((drv_volume_1 << 7) * (0x80 - drv_pan_1)) >> 7;
			drv_outattr.volume.left = ((drv_volume_1 << 7) * (drv_pan_1 + 1)) >> 7;
		}
		else//1
		{
			drv_outattr.volume.left = ((drv_volume_1 << 7) * (0x80 - drv_pan_1)) >> 7;
			drv_outattr.volume.right = ((drv_volume_1 << 7) * (drv_pan_1 + 1)) >> 7;
		}
	}
	else//0
	{
		drv_outattr.volume.left = drv_outattr.volume.right = drv_volume_1 << 6;
	}

	//Pitch
	if (drv_ptrk_stat->pitch_cntrl == 0)
    {
		drv_outattr.note = (voice_stat->keynum << 8);
	}
	else
	{
		if (drv_ptrk_stat->pitch_cntrl < 1)
		{
			pitch = drv_ptrk_stat->pitch_cntrl * voice_stat->patchmaps->pitchstep_min;

			drv_pitch1 = (32 - pitch);
			drv_pitch2 = (drv_pitch1 >> 13) + 1;
			drv_pitch3 = 0x80 - ((drv_pitch1 & 8191) >> 6);

			drv_outattr.note = (voice_stat->keynum - (drv_pitch2)) << 8 | drv_pitch3 & 0x7F;
		}
		else
		{
			pitch = drv_ptrk_stat->pitch_cntrl * voice_stat->patchmaps->pitchstep_max;

			drv_pitch1 = (32 + pitch);
			drv_pitch2 = (drv_pitch1 >> 13);
			drv_pitch3 = (drv_pitch1 & 8191) >> 6;

			drv_outattr.note = (voice_stat->keynum + (drv_pitch2)) << 8 | drv_pitch3 & 0x7F;
		}
	}

	drv_outattr.sample_note = (voice_stat->patchmaps->root_key << 8) | voice_stat->patchmaps->fine_adj;
	drv_outattr.addr = voice_stat->patchinfo->sample_pos;
	drv_outattr.adsr1 = voice_stat->patchmaps->adsr1;
	drv_outattr.adsr2 = voice_stat->patchmaps->adsr2;

	SpuSetKeyOnWithAttr(&drv_outattr);
}
//-----------------------------------------------------------
// Driver System
//-----------------------------------------------------------

void PSX_DriverInit(master_status_structure *pm_stat)//L80045CFC()
{
	char *pmem;

	//printf("PSX_DriverInit\n");

	drv_pmsbase = pm_stat;
	drv_pcurabstime = pm_stat->pabstime;
	drv_pssbase = pm_stat->pseqstattbl;
	drv_ptsbase = pm_stat->ptrkstattbl;
	drv_pvsbase = pm_stat->pvoicestattbl;
	drv_nvst = pm_stat->voices_total;

	drv_cnt = 0;

	if (drv_nvst != 0)
	{
		while (drv_cnt < drv_nvst)
		{
			if ((pm_stat->pvoicestattbl + drv_cnt)->patchtype == 1)
			{
				drv_pcurvs = (pm_stat->pvoicestattbl + drv_cnt);
				break;
			}
			drv_cnt++;
		}
	}

	drv_npti = drv_pmsbase->pmod_info->mod_hdr.patch_types_infile;

	drv_cnt = 0;
	if (drv_npti != 0)
	{
		while (drv_cnt < drv_npti)
		{
			if ((pm_stat->ppat_info + drv_cnt)->pat_grp_hdr.patch_id == 1)
			{
				drv_ppgd = (pm_stat->ppat_info + drv_cnt);
				break;
			}
			drv_cnt++;
		}
	}

	drv_hwvl = drv_ppgd->pat_grp_hdr.hw_voice_limit;

	pmem = drv_ppgd->ppat_data; /* pointer math temp */
	drv_patchesbase = (patches_header *)pmem;

	pmem += (drv_ppgd->pat_grp_hdr.patches * sizeof(patches_header));
	drv_patchmapsbase = (patchmaps_header *)pmem;

	pmem += (drv_ppgd->pat_grp_hdr.patchmaps * sizeof(patchmaps_header));
	drv_samplesbase = (patchinfo_header *)pmem;

	pmem += (drv_ppgd->pat_grp_hdr.patchinfo * sizeof(patchinfo_header));
	drv_drummapsbase = (char *)pmem;

	psxspu_init();

	drv_cnt = 0;
	do
	{
		drv_channels[drv_cnt] = 0x7F;
		drv_cnt++;
	} while (drv_cnt < 24);
}

void PSX_DriverExit (track_status *ptk_stat)//L80045F24()
{
    //printf("PSX_DriverExit\n");
	SpuQuit();
}

extern unsigned long		drv_vn;			    //8007EEE0
extern unsigned long		drv_vi;             //8007EEE4
extern unsigned long		drv_keychannels;    //8007EEE8
extern unsigned long		drv_keycnt;			//8007EEEC
extern voice_status		    *drv_pvs;			//8007EEF0
extern unsigned long		drv_voiceflag;      //8007EEF4
extern char				    drv_keysstatus[24]; //8007EFF4

void PSX_DriverEntry1 (track_status *ptk_stat)//L80045F44()
{
    //printf("PSX_DriverEntry1\n");
	drv_vn = drv_pmsbase->voices_active;

	if (drv_vn != 0)
	{
		drv_pvs = drv_pcurvs;
		drv_vi = drv_hwvl;

		while (drv_vi--)
		{
			if ((drv_pvs->flags & (VOICE_ACTIVE | VOICE_RELEASE)) == 3)
			{
				if (drv_pvs->pabstime < *drv_pcurabstime)
				{
					PSX_voiceparmoff(drv_pvs);
					if (!--drv_vn) break;
				}
			}
			drv_pvs++;
		}
	}

	drv_voiceflag = 0;
	if (voice_flags1 != 0)
	{
		drv_voiceflag = voice_flags1;
		voice_flags1 = 0;
	}

	if (voice_flags2 != 0)
	{
		drv_outattr.voice = voice_flags2;
		drv_outattr.mask = (SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_RR);
		drv_outattr.r_mode = SPU_VOICE_EXPDec;
		drv_outattr.rr = release_rate;
		SpuSetVoiceAttr(&drv_outattr);

		drv_voiceflag |= voice_flags2;
		voice_flags2 = 0;
	}

	if (drv_voiceflag != 0)
	{
		SpuSetKey(SPU_OFF, drv_voiceflag);
		drv_voiceflag = 0;
	}

	SpuGetAllKeysStatus(drv_keysstatus);

	drv_keychannels = 24;
	drv_keycnt = 0;
	drv_pvs = drv_pcurvs;
	while (drv_keychannels--)
	{
		if ((drv_pvs->flags & VOICE_ACTIVE) != 0)
		{
			if (drv_keysstatus[drv_keycnt] == 0)
			{
				PSX_voiceparmoff(drv_pvs);
			}
		}
		drv_pvs++;
		drv_keycnt++;
	}
}

void PSX_DriverEntry2(track_status *ptk_stat)//L800461F4()
{
    //printf("PSX_DriverEntry2\n");
}

void PSX_DriverEntry3(track_status *ptk_stat)//L800461FC()
{
    //printf("PSX_DriverEntry3\n");
}

extern sequence_status				*drv_pss;		//L8007EEF8

void PSX_TrkOff(track_status *ptk_stat)//L80046204()
{
    //printf("PSX_TrkOff\n");
	drv_pss = drv_pssbase + ptk_stat->seq_owner;
	PSX_TrkMute(ptk_stat);

	if (ptk_stat->voices_active != 0)
	{
		ptk_stat->flags |= (TRK_OFF | TRK_STOPPED);

		if (!drv_pss->tracks_playing--)
			drv_pss->playmode = SEQ_STATE_STOPPED;
	}
	else
	{
		Eng_TrkOff(ptk_stat);
	}
}

extern unsigned long				drv_vn_2;		    //L8007EEFC
extern unsigned long				drv_vi_2;		    //L8007EF00
extern voice_status				   *drv_pvs_2;			//L8007EF04
extern sequence_status			   *drv_pss_2;			//L8007EF08

void PSX_TrkMute(track_status *ptk_stat)//L800462B0()
{
    //printf("PSX_TrkMute\n");
	drv_vn_2 = ptk_stat->voices_active;

	if (drv_vn_2 != 0)
	{
		drv_pvs_2 = drv_pcurvs;
		drv_vi_2 = drv_hwvl;

		while (drv_vi_2--)
		{
			if (((drv_pvs_2->flags & VOICE_ACTIVE) != 0) && (drv_pvs_2->track == ptk_stat->refindx))
			{
                if (ptr_notestate != 0 && (!(drv_pvs_2->flags & VOICE_RELEASE) && (ptk_stat->sndclass == MUSIC_CLASS)))
                {
                  drv_pss_2 = (drv_pssbase + ptk_stat->seq_owner);
                  add_music_mute_note(drv_pss_2->seq_num,
                               drv_pvs_2->track,
                               drv_pvs_2->keynum,
                               drv_pvs_2->velnum,
                               drv_pvs_2->patchmaps,
                               drv_pvs_2->patchinfo);
                }
                drv_pvs_2->adsr2 = 0x10000000 >> (0x1f - (unsigned int)release_rate & 0x1f);
                PSX_voicerelease(drv_pvs_2);
                //printf("voice_stat->refindx %d\n",drv_pvs_2->refindx);

				voice_flags2 = 1 << (drv_pvs_2->refindx & 31) | voice_flags2;
				if (!--drv_vn_2) break;
			}

			drv_pvs_2++;
		}
	}
}

extern unsigned short				drv_thepatch;				//L8007EF0C

void PSX_PatchChg(track_status *ptk_stat)//L8004646C()
{
    //printf("PSX_PatchChg\n");
	drv_thepatch = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	ptk_stat->patchnum = drv_thepatch;
}

void PSX_PatchMod(track_status *ptk_stat)//L80046494()
{
    //printf("PSX_PatchMod\n");
}

extern unsigned long				drv_voices_active_3;		//L8007EF10
extern unsigned long				drv_hwvl_3;		//L8007EF14
extern unsigned long				drv_pitch_1;				//L8007EF18
extern unsigned long				drv_pitch_2;				//L8007EF1C
extern unsigned long				drv_pitch_3;				//L8007EF20
extern voice_status				    *drv_voice_stat_3;			//L8007EF24
extern short				        drv_pitch_cntrl;			//L8007EF28

void PSX_PitchMod(track_status *ptk_stat)//L8004649C()
{
	unsigned long pitch;

	//printf("PSX_PitchMod\n");

	drv_pitch_cntrl = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	if (ptk_stat->pitch_cntrl != drv_pitch_cntrl)
	{
		ptk_stat->pitch_cntrl = drv_pitch_cntrl;
		drv_voices_active_3 = ptk_stat->voices_active;

		if (drv_voices_active_3 != 0)
		{
			drv_voice_stat_3 = drv_pcurvs;
			drv_hwvl_3 = drv_hwvl;

			while (drv_hwvl_3--)
			{
                if ((drv_voice_stat_3->flags & VOICE_ACTIVE) && (drv_voice_stat_3->track == ptk_stat->refindx))
                {
                    drv_outattr.voice = 1 << (drv_voice_stat_3->refindx & 31);
                    drv_outattr.mask = (SPU_VOICE_NOTE);

                    if (ptk_stat->pitch_cntrl != 0)
                    {
                        if (ptk_stat->pitch_cntrl <= 0)
                        {
                            pitch = ptk_stat->pitch_cntrl * drv_voice_stat_3->patchmaps->pitchstep_min;

                            drv_pitch_1 = (32 - pitch);
                            drv_pitch_2 = (drv_pitch_1 >> 13) + 1;
                            drv_pitch_3 = 0x80 - ((drv_pitch_1 & 8191) >> 6);

                            drv_outattr.note = (drv_voice_stat_3->keynum - (drv_pitch_2)) << 8 | drv_pitch_3 & 0x7F;
                        }
                        else
                        {
                            pitch = ptk_stat->pitch_cntrl * drv_voice_stat_3->patchmaps->pitchstep_max;

                            drv_pitch_1 = (32 + pitch);
                            drv_pitch_2 = (drv_pitch_1 >> 13);
                            drv_pitch_3 = (drv_pitch_1 & 8191) >> 6;

                            drv_outattr.note = (drv_voice_stat_3->keynum + (drv_pitch_2)) << 8 | drv_pitch_3 & 0x7F;
                        }
                    }
                    else
                    {
                        drv_outattr.note = (drv_voice_stat_3->keynum << 8);
                    }

                    SpuSetVoiceAttr(&drv_outattr);

                    if (!--drv_voices_active_3) break;
                }

				drv_voice_stat_3++;
			}
		}
	}
}

void PSX_ZeroMod(track_status *ptk_stat)//L800466EC()
{
    //printf("PSX_ZeroMod\n");
}

void PSX_ModuMod(track_status *ptk_stat)//L800466f4()
{
    //printf("PSX_ModuMod\n");
}

extern unsigned long				drv_voices_active_4;		//L8007EF2C
extern unsigned long				drv_hwvl_4;		//L8007EF30
extern unsigned long				drv_volume_2;				//L8007EF34
extern voice_status				    *drv_voice_stat_4;			//L8007EF38
extern short				        drv_pan_2;					//L8007EF3C

void PSX_VolumeMod(track_status *ptk_stat)//L800466FC()
{
	unsigned long volume;
	//printf("PSX_VolumeMod\n");
	ptk_stat->volume_cntrl = *(ptk_stat->ppos + 1);

	drv_voices_active_4 = ptk_stat->voices_active;
	if (drv_voices_active_4 != 0)
	{
		drv_voice_stat_4 = drv_pcurvs;
		drv_hwvl_4 = drv_hwvl;

		while (drv_hwvl_4--)
		{
			if ((drv_voice_stat_4->flags & VOICE_ACTIVE) && (drv_voice_stat_4->track == ptk_stat->refindx))
			{
				if (pan_status == 0)
				{
					drv_pan_2 = 0x40;
				}
				else
				{
					drv_pan_2 = ptk_stat->pan_cntrl + drv_voice_stat_4->patchmaps->pan - 0x40;

					if (drv_pan_2 < 0)      drv_pan_2 = 0;
					if (drv_pan_2 > 0x7F)	drv_pan_2 = 0x7F;
				}

				if (ptk_stat->sndclass == SNDFX_CLASS)
				{
					volume = drv_voice_stat_4->velnum * drv_voice_stat_4->patchmaps->volume * ptk_stat->volume_cntrl * master_sfx_volume;
				}
				else
				{
					volume = drv_voice_stat_4->velnum * drv_voice_stat_4->patchmaps->volume * ptk_stat->volume_cntrl * master_mus_volume;
				}

				drv_volume_2 = volume >> 21;

				drv_outattr.voice = 1 << (drv_voice_stat_4->refindx & 31);
				drv_outattr.mask = (SPU_VOICE_VOLL | SPU_VOICE_VOLR);

				if (pan_status != 0)
				{
					if (pan_status != 1)//2
					{
						drv_outattr.volume.right = ((drv_volume_2 << 7) * (0x80 - drv_pan_2)) >> 7;
						drv_outattr.volume.left  = ((drv_volume_2 << 7) * (drv_pan_2 + 1)) >> 7;
					}
					else//1
					{
						drv_outattr.volume.left  = ((drv_volume_2 << 7) * (0x80 - drv_pan_2)) >> 7;
						drv_outattr.volume.right = ((drv_volume_2 << 7) * (drv_pan_2 + 1)) >> 7;
					}
				}
				else//0
				{
					drv_outattr.volume.left = drv_outattr.volume.right = drv_volume_2 << 6;
				}

				SpuSetVoiceAttr(&drv_outattr);

				if (!--drv_voices_active_4) break;
			}

			drv_voice_stat_4++;
		}
	}
}

extern unsigned long				drv_voices_active_5;		//L8007EF40
extern unsigned long				drv_hwvl_5;		//L8007EF44
extern unsigned long				drv_volume_3;				//L8007EF48
extern voice_status				    *drv_voice_stat_5;			//L8007EF4C
extern short				        drv_pan_3;					//L8007EF50

void PSX_PanMod(track_status *ptk_stat)//L80046A14()
{
	unsigned long volume;
	//printf("PSX_PanMod\n");

	ptk_stat->pan_cntrl = *(ptk_stat->ppos + 1);

	if (pan_status != 0)
	{
		drv_voices_active_5 = ptk_stat->voices_active;

		if (drv_voices_active_5 != 0)
		{
			drv_voice_stat_5 = drv_pcurvs;
			drv_hwvl_5 = drv_hwvl;

			while (drv_hwvl_5--)
			{
				if ((drv_voice_stat_5->flags & VOICE_ACTIVE) && (drv_voice_stat_5->track == ptk_stat->refindx))
				{
					drv_pan_3 = ptk_stat->pan_cntrl + drv_voice_stat_5->patchmaps->pan - 0x40;

					if (drv_pan_3 < 0)      drv_pan_3 = 0;
                    if (drv_pan_3 > 0x7F)	drv_pan_3 = 0x7F;

					if (ptk_stat->sndclass == SNDFX_CLASS)
					{
						volume = drv_voice_stat_5->velnum * drv_voice_stat_5->patchmaps->volume * ptk_stat->volume_cntrl * master_sfx_volume;
					}
					else
					{
						volume = drv_voice_stat_5->velnum * drv_voice_stat_5->patchmaps->volume * ptk_stat->volume_cntrl * master_mus_volume;
					}

					drv_volume_3 = volume >> 21;

					drv_outattr.voice = 1 << (drv_voice_stat_5->refindx & 31);
					drv_outattr.mask = (SPU_VOICE_VOLL | SPU_VOICE_VOLR);

					if (pan_status != 1)//2
					{
						drv_outattr.volume.right = ((drv_volume_3 << 7) * (0x80 - drv_pan_3)) >> 7;
						drv_outattr.volume.left = ((drv_volume_3 << 7) * (drv_pan_3 + 1)) >> 7;
					}
					else//1
					{
						drv_outattr.volume.left = ((drv_volume_3 << 7) * (0x80 - drv_pan_3)) >> 7;
						drv_outattr.volume.right = ((drv_volume_3 << 7) * (drv_pan_3 + 1)) >> 7;
					}

					SpuSetVoiceAttr(&drv_outattr);

					if (!--drv_voices_active_5) break;
				}

				drv_voice_stat_5++;
			}
		}
	}
}

//PSX_PedalMod
void PSX_PedalMod(track_status *ptk_stat)//L80046CF0()
{
    //printf("PSX_PedalMod\n");
}

//PSX_ReverbMod
void PSX_ReverbMod(track_status *ptk_stat)//L80046CF8()
{
    //printf("PSX_ReverbMod\n");
}

//PSX_ChorusMod
void PSX_ChorusMod(track_status *ptk_stat)//L80046D00()
{
    //printf("PSX_ChorusMod\n");
}

void PSX_voiceon(voice_status *voice_stat, track_status *ptk_stat,
	           patchmaps_header *patchmaps, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum)//L80046D08()
{
	int adsr2;
	//printf("PSX_voiceon\n");

	voice_stat->flags = (voice_stat->flags | VOICE_ACTIVE) & ~VOICE_RELEASE;
	voice_stat->track = ptk_stat->refindx;
	voice_stat->keynum = keynum;
	voice_stat->velnum = velnum;
	voice_stat->sndtype = 0;//no used??
	voice_stat->priority = ptk_stat->priority;
	voice_stat->patchmaps = patchmaps;
	voice_stat->patchinfo = patchinfo;
	voice_stat->pabstime = *drv_pcurabstime;

    if (!(patchmaps->adsr2 & 0x20))
    {
        adsr2 = 0x5dc0000;
    }
    else
    {
        adsr2 = 0x10000000;
    }
    voice_stat->adsr2 = adsr2 >> (0x1f - ((unsigned int)patchmaps->adsr2 & 0x1f) & 0x1f);

	ptk_stat->voices_active++;
	drv_pmsbase->voices_active++;

	TriggerPSXVoice(voice_stat, keynum, velnum);//TriggerPSXVoice
}


extern track_status				*drv_ptrk_stat_1;			//L8007EF54

void PSX_voiceparmoff(voice_status *voice_stat)//L80046DDC()
{
    //printf("PSX_voiceparmoff\n");
	drv_ptrk_stat_1 = (drv_ptsbase + voice_stat->track);

	drv_pmsbase->voices_active--;
	drv_ptrk_stat_1->voices_active--;

	if ((drv_ptrk_stat_1->voices_active == 0) && (drv_ptrk_stat_1->flags & TRK_OFF))
	{
        Eng_TrkOff(drv_ptrk_stat_1);
	}

	voice_stat->flags &= ~(VOICE_ACTIVE|VOICE_RELEASE);
}

void PSX_voicerelease(voice_status *voice_stat)//L80046EA4()
{
    //printf("PSX_voicerelease\n");
	voice_flags1 = 1 << (voice_stat->refindx & 31) | voice_flags1;
	voice_stat->flags |= VOICE_RELEASE;
	voice_stat->pabstime = *drv_pcurabstime + voice_stat->adsr2;
}


extern unsigned long				drv_hwvl_6;		//L8007EF58
extern unsigned long				drv_note_stat;				//L8007EF5C
extern voice_status				*drv_voice_stat_6;			//L8007EF60

void PSX_voicenote(track_status *ptk_stat,
	           patchmaps_header *patchmap, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum)//L80046EF0()
{
    //printf("PSX_voicenote\n");
	drv_note_stat = 0;
	drv_hwvl_6 = drv_hwvl;
	drv_voice_stat_6 = drv_pcurvs;

    while (drv_hwvl_6--)
    {
        if ((drv_voice_stat_6->flags & VOICE_ACTIVE) == 0)
        {
          PSX_voiceon(drv_voice_stat_6, ptk_stat, patchmap, patchinfo, keynum, velnum);
          drv_note_stat = 0;
          break;
        }
        if (drv_voice_stat_6->priority <= ptk_stat->priority)
        {
          if (priority <= drv_voice_stat_6->priority)
          {
            if ((drv_voice_stat_6->flags & VOICE_RELEASE) == 0)
            {
              if ((ptr_pvoicestattbl->flags & VOICE_RELEASE) != 0) goto next_voice;
            }
            else
            {
              if ((ptr_pvoicestattbl->flags & VOICE_RELEASE) == 0) goto set_note_stat;
            }
            if (max_pabstime <= drv_voice_stat_6->pabstime) goto next_voice;
          }

    set_note_stat:
          drv_note_stat = 1;
          priority = drv_voice_stat_6->priority;
          max_pabstime = drv_voice_stat_6->pabstime;
          ptr_pvoicestattbl = drv_voice_stat_6;
        }

    next_voice:
        drv_voice_stat_6++;
    }

    if (drv_note_stat != 0)
    {
        PSX_voiceparmoff(ptr_pvoicestattbl);
        PSX_voiceon(ptr_pvoicestattbl,ptk_stat,patchmap,patchinfo,keynum,velnum);
    }
}

extern unsigned long				drv_patch_cnt;				//L8007EF64
extern unsigned char				drv_byte01;					//L8007EF68
extern unsigned char				drv_byte02;					//L8007EF6C
extern unsigned char				drv_patchmap_cnt;			//L8007EF70
extern patches_header				*drv_patch_1;				//L8007EF74
extern patchmaps_header			    *drv_patchmap_1;			//L8007EF78
extern patchinfo_header			    *drv_samplesbase_1;			//L8007EF7C

void PSX_NoteOn(track_status *ptk_stat)//L80047104()
{
	char *drummaps;
	//printf("PSX_NoteOn\n");

	drv_byte01 = *(ptk_stat->ppos + 1);
	drv_byte02 = *(ptk_stat->ppos + 2);

	if (ptk_stat->sndclass == DRUMS_CLASS)
	{
		drummaps = (drv_drummapsbase + (drv_byte01 * sizeof(long)));
		drv_patch_1 = drv_patchesbase + *(unsigned short*)(drummaps);
		drv_byte01 = *(drummaps + sizeof(short));
	}
	else
	{
		drv_patch_1 = drv_patchesbase+ptk_stat->patchnum;
	}

	drv_patch_cnt = 0;
	drv_patchmap_cnt = (char) drv_patch_1->patchmap_cnt;

	while (drv_patchmap_cnt--)
	{
		drv_patchmap_1 = drv_patchmapsbase + (drv_patch_1->patchmap_idx + drv_patch_cnt);
		drv_samplesbase_1 = drv_samplesbase + drv_patchmap_1->sample_id;

		if (drv_samplesbase_1->sample_pos != 0)
		{
			if (drv_patchmap_1->note_min <= drv_byte01)
			{
				if (drv_byte01 <= drv_patchmap_1->note_max)
				{
					PSX_voicenote(ptk_stat,drv_patchmap_1,
                   drv_samplesbase_1,drv_byte01,drv_byte02);
				}
			}
		}

		drv_patch_cnt++;
	}
}

extern unsigned long			drv_hwvl_7;		    //L8007EF80
extern voice_status				*drv_voice_stat_7;  //L8007EF84

void PSX_NoteOff(track_status *ptk_stat)//L800472E8()
{
    //printf("PSX_NoteOff\n");
	drv_voice_stat_7 =
	drv_pcurvs;
	drv_hwvl_7 = drv_hwvl;

	while (drv_hwvl_7--)
	{
		if (((drv_voice_stat_7->flags & (VOICE_ACTIVE|VOICE_RELEASE)) == VOICE_ACTIVE) &&
			(drv_voice_stat_7->keynum == *(ptk_stat->ppos + 1)) &&
			(drv_voice_stat_7->track == ptk_stat->refindx))
		{
			PSX_voicerelease(drv_voice_stat_7);
		}

		drv_voice_stat_7++;
	}
}
