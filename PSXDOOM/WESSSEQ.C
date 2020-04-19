// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include "wessseq.h"

extern void (**CmdFuncArr[10])(track_status *);
extern void(*DrvFunctions[36])();

char *Read_Vlq(char *pstart, void *deltatime);
char *Write_Vlq(char *dest, unsigned int deltatime);
int Len_Vlq(unsigned int deltatime);

void Eng_DriverInit (master_status_structure *pm_stat);
void Eng_DriverExit (track_status *ptk_stat);
void Eng_DriverEntry1 (track_status *ptk_stat);
void Eng_DriverEntry2 (track_status *ptk_stat);
void Eng_DriverEntry3 (track_status *ptk_stat);
void Eng_TrkOff (track_status *ptk_stat);
void Eng_TrkMute (track_status *ptk_stat);
void Eng_PatchChg (track_status *ptk_stat);
void Eng_PatchMod (track_status *ptk_stat);
void Eng_PitchMod (track_status *ptk_stat);
void Eng_ZeroMod (track_status *ptk_stat);
void Eng_ModuMod (track_status *ptk_stat);
void Eng_VolumeMod (track_status *ptk_stat);
void Eng_PanMod (track_status *ptk_stat);
void Eng_PedalMod (track_status *ptk_stat);
void Eng_ReverbMod (track_status *ptk_stat);
void Eng_ChorusMod (track_status *ptk_stat);
void Eng_NoteOn (track_status *ptk_stat);
void Eng_NoteOff (track_status *ptk_stat);
void Eng_StatusMark (track_status *ptk_stat);
void Eng_GateJump (track_status *ptk_stat);
void Eng_IterJump (track_status *ptk_stat);
void Eng_ResetGates (track_status *ptk_stat);
void Eng_ResetIters (track_status *ptk_stat);
void Eng_WriteIterBox (track_status *ptk_stat);
void Eng_SeqTempo (track_status *ptk_stat);
void Eng_SeqGosub (track_status *ptk_stat);
void Eng_SeqJump (track_status *ptk_stat);
void Eng_SeqRet (track_status *ptk_stat);
void Eng_SeqEnd (track_status *ptk_stat);
void Eng_TrkTempo (track_status *ptk_stat);
void Eng_TrkGosub (track_status *ptk_stat);
void Eng_TrkJump (track_status *ptk_stat);
void Eng_TrkRet (track_status *ptk_stat);
void Eng_TrkEnd (track_status *ptk_stat);
void Eng_NullEvent (track_status *ptk_stat);

void(*DrvFunctions[36])() =
{
	Eng_DriverInit,
	Eng_DriverExit,
	Eng_DriverEntry1,
	Eng_DriverEntry2,
	Eng_DriverEntry3,
	Eng_TrkOff,
	Eng_TrkMute,
	Eng_PatchChg,
	Eng_PatchMod,
	Eng_PitchMod,
	Eng_ZeroMod,
	Eng_ModuMod,
	Eng_VolumeMod,
	Eng_PanMod,
	Eng_PedalMod,
	Eng_ReverbMod,
	Eng_ChorusMod,
	Eng_NoteOn,
	Eng_NoteOff,
	Eng_StatusMark,     //0x13
	Eng_GateJump,       //0x14
	Eng_IterJump,       //0x15
	Eng_ResetGates,     //0x16
	Eng_ResetIters,     //0x17
	Eng_WriteIterBox,   //0x18
	Eng_SeqTempo,       //0x19
	Eng_SeqGosub,       //0x1A
	Eng_SeqJump,        //0x1B
	Eng_SeqRet,         //0x1C
	//SeqFunctions ??
	Eng_SeqEnd,         //0x1D
	Eng_TrkTempo,       //0x1E
	Eng_TrkGosub,       //0x1F
	Eng_TrkJump,        //0x20
	Eng_TrkRet,         //0x21
	Eng_TrkEnd,         //0x22
	Eng_NullEvent       //0x23
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
extern unsigned char				drv_npti;	            //8007EED4
extern unsigned long				drv_cnt;				//8007EED8
extern voice_status				    *drv_pvsbase;		    //8007EEDC

extern voice_status				    *drv_pcurvs;	        //8007EF94

extern unsigned char				drv_channels[24];		//8007f00c

//-----------------------------------------------------------
// Vlq System
//-----------------------------------------------------------

extern unsigned int data01;    //L8007F024
extern unsigned char data02;    //L8007F028

char *Read_Vlq(char *pstart, void *deltatime)//L80035F80()
{
	unsigned int value;
	char data2;

	value = *pstart++;
	if (value & 0x80)
	{
		value &= 0x7f;
		do
		{
			data2 = *pstart++;
			value = (data2 & 0x7f) + (value << 7);
			data02 = data2;
			data01 = value;
		} while (data2 & 0x80);
	}
	*(int*)deltatime = value;

	return pstart;
}

char *Write_Vlq(char *dest, unsigned int deltatime)//L8004744C()
{
	char data[8];
	char *ptr;

    data[0] = (deltatime & 0x7f);
	ptr = (char*)&data+1;

	deltatime >>= 7;
	if (deltatime)
	{
		do
		{
			*ptr++ = ((deltatime & 0x7f) | 0x80);
			deltatime >>= 7;
		} while (deltatime);
	}

	do
	{
		ptr--;
		*dest++ = *ptr;
	} while (*ptr & 0x80);

	return dest;
}

int Len_Vlq(unsigned int deltatime)//L800474AC()
{
    char start[8];
    char* end = Write_Vlq(start, deltatime);
    return (int)(end - start);
}

//-----------------------------------------------------------
// Engine System
//-----------------------------------------------------------

extern unsigned char			 eng_ntwa;	        //L800758E4
extern track_status			    *eng_ptsbase;	//L800758E8
extern sequence_status			*eng_pssbase;	//L800758EC
extern master_status_structure  *eng_pmsbase;		//L800758F0

void Eng_DriverInit (master_status_structure *pm_stat)//L80047518
{
    //printf("Eng_DriverInit\n");
	eng_pmsbase = pm_stat;
	eng_ptsbase = pm_stat->ptrkstattbl;
	eng_pssbase = pm_stat->pseqstattbl;
	eng_ntwa = pm_stat->pmod_info->mod_hdr.trk_work_areas;
}

void Eng_DriverExit (track_status *ptk_stat)//L80047554()
{
    //printf("Eng_DriverExit\n");
}

void Eng_DriverEntry1 (track_status *ptk_stat)//L8004755C()
{
    //printf("Eng_DriverEntry1\n");
}

void Eng_DriverEntry2 (track_status *ptk_stat)//L80047564()
{
    //printf("Eng_DriverEntry2\n");
}

void Eng_DriverEntry3 (track_status *ptk_stat)//L8004756C()
{
    //printf("Eng_DriverEntry3\n");
}

extern sequence_status			*eng_seq_stat_1;		//L8007F02C
extern char					    *eng_ptrk_indxs1;		//L8007F030
extern unsigned long			eng_max_trks_perseq1;	//L8007F034

void Eng_TrkOff (track_status *ptk_stat)//L80047574()
{
	sequence_status			*seq_stat;
	//printf("Eng_TrkOff\n");

	seq_stat = (eng_pmsbase->pseqstattbl + ptk_stat->seq_owner);
	eng_seq_stat_1 = seq_stat;

	if ((ptk_stat->flags & TRK_STOPPED) == 0)
	{
		ptk_stat->flags |= TRK_STOPPED;

		seq_stat->tracks_playing--;
		if (seq_stat->tracks_playing == 0)
		{
			eng_seq_stat_1->playmode = 0;
		}
	}

	if ((ptk_stat->flags & TRK_HANDLED) == 0)
	{
		eng_max_trks_perseq1 = eng_pmsbase->max_trks_perseq;
		eng_ptrk_indxs1 = eng_seq_stat_1->ptrk_indxs;

		while (eng_max_trks_perseq1--)
		{
			if (*eng_ptrk_indxs1 == ptk_stat->refindx)
			{
				*eng_ptrk_indxs1 = 0xFF;
				break;
			}
			eng_ptrk_indxs1++;
		}

		ptk_stat->flags &= ~TRK_ACTIVE;

		eng_pmsbase->trks_active--;
		eng_seq_stat_1->tracks_active--;

		if (eng_seq_stat_1->tracks_active == 0)
		{
			eng_seq_stat_1->flags &= ~SEQ_ACTIVE;
			eng_pmsbase->seqs_active--;
		}
	}

	ptk_stat->flags &= ~TRK_TIMED;
}

void Eng_TrkMute (track_status *ptk_stat)//L80047720()
{
    //printf("Eng_TrkMute\n");
}

extern unsigned char			eng_patchnum;			//L8007F038

void Eng_PatchChg (track_status *ptk_stat)//L80047728()
{
    //printf("Eng_PatchChg\n");
	eng_patchnum = *(ptk_stat->ppos + 1);
	ptk_stat->patchnum = eng_patchnum;
}


void Eng_PatchMod (track_status *ptk_stat)//L80047750()
{
    //printf("Eng_PatchMod\n");
}

extern short			        eng_pitch_cntrl;		//L8007F03C

void Eng_PitchMod (track_status *ptk_stat)//L80047758()
{
    //printf("Eng_PitchMod\n");
	eng_pitch_cntrl = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	ptk_stat->pitch_cntrl = eng_pitch_cntrl;
}

void Eng_ZeroMod (track_status *ptk_stat)//L80047780()
{
    //printf("Eng_ZeroMod\n");
}

void Eng_ModuMod (track_status *ptk_stat)//L80047788()
{
    //printf("Eng_ModuMod\n");
}

extern unsigned char			eng_volume_cntrl;		//L8007F040

void Eng_VolumeMod (track_status *ptk_stat)//L80047790()
{
    //printf("Eng_VolumeMod\n");
	eng_volume_cntrl = *(ptk_stat->ppos + 1);
	ptk_stat->volume_cntrl = eng_volume_cntrl;
}

extern unsigned char			eng_pan_cntrl;			//L8007F044

void Eng_PanMod (track_status *ptk_stat)//L800477B0()
{
    //printf("Eng_PanMod\n");
	eng_pan_cntrl = *(ptk_stat->ppos + 1);
	ptk_stat->pan_cntrl = eng_pan_cntrl;
}

void Eng_PedalMod (track_status *ptk_stat)//L800477D0()
{
    //printf("Eng_PedalMod\n");
}

void Eng_ReverbMod (track_status *ptk_stat)//L800477D8()
{
    //printf("Eng_ReverbMod\n");
}

void Eng_ChorusMod (track_status *ptk_stat)//L800477E0()
{
    //printf("Eng_ChorusMod\n");
}

void Eng_NoteOn (track_status *ptk_stat)//L800477E8()
{
    //printf("Eng_NoteOn\n");
}

void Eng_NoteOff (track_status *ptk_stat)//L800477F0()
{
    //printf("Eng_NoteOff\n");
}

extern unsigned char			eng_callback_areas;	//L8007F048
extern unsigned char			eng_callbacks_active;//L8007F04C
extern callback_status			*eng_pcalltable;	//L8007F050

void Eng_StatusMark (track_status *ptk_stat)//L800477F8()
{
	callback_status *cbs;
	//printf("Eng_StatusMark\n");

	eng_callbacks_active = eng_pmsbase->callbacks_active;

	if (eng_callbacks_active)
	{
		eng_pcalltable = eng_pmsbase->pcalltable;

		eng_callback_areas = eng_pmsbase->pmod_info->mod_hdr.callback_areas;

		while (eng_callback_areas--)
		{
			cbs = eng_pcalltable;
			if (cbs->active)
			{
				if (cbs->type == *(ptk_stat->ppos + 1))
				{
					cbs->curval = (unsigned short)(*(ptk_stat->ppos + 2) | (*(ptk_stat->ppos + 3) << 8));
					cbs->callfunc(cbs->type, cbs->curval);
					break;
				}

				if(!--eng_callbacks_active) break;
			}

			eng_pcalltable++;
		}
	}
}

extern short			    eng_labelnum;	//L8007F054
extern char					*eng_plabellist;//L8007F058
extern char					*eng_pgates;	//L8007F05C

void Eng_GateJump (track_status *ptk_stat)//L80047914()
{
    //printf("Eng_GateJump\n");
	eng_pgates = (eng_pssbase + ptk_stat->seq_owner)->pgates + *(ptk_stat->ppos + 1);

	if (*eng_pgates != 0)
	{
		if (*eng_pgates == 0xFF)
		{
			*eng_pgates = *(ptk_stat->ppos + 2);
		}

		eng_labelnum = (*(ptk_stat->ppos + 3) | (*(ptk_stat->ppos + 4) << 8));

		if ((eng_labelnum >= 0) && (eng_labelnum < ptk_stat->labellist_count))
		{
            eng_plabellist = ptk_stat->pstart + *(ptk_stat->plabellist + eng_labelnum);
            ptk_stat->ppos = eng_plabellist;
		}
	}
	ptk_stat->flags |= TRK_SKIP;
}

extern char					    *eng_piters;	//L8007F068
extern short			        eng_labelnum2;	//L8007F060
extern char					    *eng_plabellist2;//L8007F064

void Eng_IterJump (track_status *ptk_stat)//L80047A00()
{
    //printf("Eng_IterJump\n");
	eng_piters = (eng_pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);

	if (*eng_piters != 0)
	{
		if (*eng_piters == 0xFF)
		{
			*eng_piters = *(ptk_stat->ppos + 2);
		}
		else
		{
			*eng_piters = *eng_piters + 0xFF;
		}

		eng_labelnum2 = (*(ptk_stat->ppos + 3) | (*(ptk_stat->ppos + 4) << 8));

		if (eng_labelnum2 >= 0)
		{
			if (eng_labelnum2 < ptk_stat->labellist_count)
			{
				eng_plabellist2 = ptk_stat->pstart + *(ptk_stat->plabellist + eng_labelnum2);
				ptk_stat->ppos = eng_plabellist2;
			}
		}
	}
	ptk_stat->flags |= TRK_SKIP;
}

extern unsigned char			eng_gates_per_seq;	//L8007F06C
extern char					    *eng_pgates2;		//L8007F070

void Eng_ResetGates (track_status *ptk_stat)//L80047AFC()
{
    //printf("Eng_ResetGates\n");
	if (*(ptk_stat->ppos + 1) == 0xFF)
	{
		eng_gates_per_seq = eng_pmsbase->pmod_info->mod_hdr.gates_per_seq;
		eng_pgates2 = (eng_pssbase + ptk_stat->seq_owner)->pgates;

        while (eng_gates_per_seq--)
        {
            *eng_pgates2++ = 0xFF;
        }
	}
    else
    {
        eng_pgates2 = (eng_pssbase + ptk_stat->seq_owner)->pgates + *(ptk_stat->ppos + 1);
        *eng_pgates2 = 0xFF;
    }
}

extern unsigned char			eng_iters_per_seq;	//L8007F074
extern char					    *eng_piters2;		//L8007F078

void Eng_ResetIters (track_status *ptk_stat)//L80047C00()
{
    //printf("Eng_ResetIters\n");
	if (*(ptk_stat->ppos + 1) == 0xff)
	{
		eng_iters_per_seq = eng_pmsbase->pmod_info->mod_hdr.iters_per_seq;
		eng_piters2 = (eng_pssbase + ptk_stat->seq_owner)->piters;

        while (eng_iters_per_seq--)
        {
            *eng_piters2++ = 0xff;
        }
	}
    else
    {
        eng_piters2 = (eng_pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);
        *eng_piters2 = 0xff;
    }
}

extern char					*eng_piters3;		//L8007F07C

void Eng_WriteIterBox (track_status *ptk_stat)//L80047D04()
{
    //printf("Eng_WriteIterBox\n");
	eng_piters3 = (eng_pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);
	*eng_piters3 = *(ptk_stat->ppos + 2);
}

extern unsigned short			eng_tracks;				//L8007F080
extern unsigned char			eng_tracks_active2;		//L8007F084
extern unsigned char			*eng_ptrk_indxs2;		//L8007F088
extern track_status			    *eng_ptrk_stat_1;		//L8007F08C
extern sequence_status			*eng_seq_stat_2;		//L8007F090

void Eng_SeqTempo (track_status *ptk_stat)//L80047D48()
{
	//printf("Eng_SeqTempo\n");

	eng_seq_stat_2 = (eng_pssbase + ptk_stat->seq_owner);
	eng_ptrk_indxs2 = eng_seq_stat_2->ptrk_indxs;
	eng_tracks_active2 = eng_seq_stat_2->tracks_active;
	eng_tracks = (eng_pmsbase->pmod_info->pseq_info+eng_seq_stat_2->seq_num)->seq_hdr.tracks;

    while (eng_tracks--)
	{
		if (*eng_ptrk_indxs2 != 0xFF)
		{
			eng_ptrk_stat_1 = (eng_ptsbase+(*eng_ptrk_indxs2));

			eng_ptrk_stat_1->qpm = (unsigned short)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
			eng_ptrk_stat_1->ppi = CalcPartsPerInt(GetIntsPerSec(), eng_ptrk_stat_1->ppq, eng_ptrk_stat_1->qpm);

			if (!--eng_tracks_active2) break;
		}
		eng_ptrk_indxs2++;
	}
}


extern short			        eng_labelnum3;			//L8007F094
extern unsigned short			eng_tracks2;			//L8007F098
extern char					    *eng_plabellist3;		//L8007F09C
extern unsigned char			eng_tracks_active3;		//L8007F0A0
extern unsigned char			*eng_ptrk_indxs3;		//L8007F0A4
extern track_status			    *eng_ptrk_stat_2;		//L8007F0A8
extern sequence_status			*eng_seq_stat_3;		//L8007F0AC

void Eng_SeqGosub (track_status *ptk_stat)//L80047EC8()
{
	//printf("Eng_SeqGosub\n");

	eng_labelnum3 = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));

	if ((eng_labelnum3 >= 0) && (eng_labelnum3 < ptk_stat->labellist_count))
	{
		eng_seq_stat_3 = (eng_pssbase + ptk_stat->seq_owner);
		eng_ptrk_indxs3 = eng_seq_stat_3->ptrk_indxs;
		eng_tracks_active3 = eng_seq_stat_3->tracks_active;
		eng_tracks2 = (eng_pmsbase->pmod_info->pseq_info + eng_seq_stat_3->seq_num)->seq_hdr.tracks;

		while (eng_tracks2--)
		{
			if (*eng_ptrk_indxs3 != 0xFF)
			{
				eng_ptrk_stat_2 = (eng_ptsbase + (*eng_ptrk_indxs3));

				*eng_ptrk_stat_2->psp = (eng_ptrk_stat_2->ppos + skip_table[26]);
				eng_ptrk_stat_2->psp++;
				eng_plabellist3 = eng_ptrk_stat_2->pstart + *(eng_ptrk_stat_2->plabellist + eng_labelnum3);
				eng_ptrk_stat_2->ppos = eng_plabellist3;

				if (!--eng_tracks_active3) break;
			}
			eng_ptrk_indxs3++;
		}
	}
	ptk_stat->flags |= TRK_SKIP;
}

extern short			        eng_labelnum4;			//L8007F0B0
extern unsigned short			eng_tracks3;			//L8007F0B4
extern char					    *eng_plabellist4;		//L8007F0B8
extern unsigned char			eng_tracks_active4;		//L8007F0BC
extern unsigned char			*eng_ptrk_indxs4;		//L8007F0C0
extern track_status			    *eng_ptrk_stat_3;		//L8007F0C4
extern sequence_status			*eng_seq_stat_4;		//L8007F0C8

void Eng_SeqJump (track_status *ptk_stat)//L800480A4()
{
	//printf("Eng_SeqJump\n");

	eng_labelnum4 = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));

	if ((eng_labelnum4 >= 0) && (eng_labelnum4 < ptk_stat->labellist_count))
	{
		eng_seq_stat_4 = (eng_pssbase + ptk_stat->seq_owner);
		eng_ptrk_indxs4 = eng_seq_stat_4->ptrk_indxs;
		eng_tracks_active4 = eng_seq_stat_4->tracks_active;
		eng_tracks3 = (eng_pmsbase->pmod_info->pseq_info + eng_seq_stat_4->seq_num)->seq_hdr.tracks;

		while (eng_tracks3--)
		{
			if (*eng_ptrk_indxs4 != 0xFF)
			{
				eng_ptrk_stat_3 = (eng_ptsbase + (*eng_ptrk_indxs4));
				eng_plabellist4 = eng_ptrk_stat_3->pstart + *(eng_ptrk_stat_3->plabellist + eng_labelnum4);
				eng_ptrk_stat_3->ppos = eng_plabellist4;

				if (!--eng_tracks_active4) break;
			}
			eng_ptrk_indxs4++;
		}
	}
	ptk_stat->flags |= TRK_SKIP;
}

extern unsigned short			eng_tracks4;			//L8007F0CC
extern unsigned char			eng_tracks_active5;		//L8007F0D0
extern unsigned char			*eng_ptrk_indxs5;		//L8007F0D4
extern track_status			    *eng_ptrk_stat_4;		//L8007F0D8
extern sequence_status			*eng_seq_stat_5;		//L8007F0DC

void Eng_SeqRet (track_status *ptk_stat)//L8004824C()
{
	//printf("Eng_SeqRet\n");

	eng_seq_stat_5 = (eng_pssbase + ptk_stat->seq_owner);
	eng_ptrk_indxs5 = eng_seq_stat_5->ptrk_indxs;
	eng_tracks_active5 = eng_seq_stat_5->tracks_active;
	eng_tracks4 = (eng_pmsbase->pmod_info->pseq_info + eng_seq_stat_5->seq_num)->seq_hdr.tracks;

	while (eng_tracks4--)
	{
		if (*eng_ptrk_indxs5 != 0xFF)
		{
			eng_ptrk_stat_4 = (eng_ptsbase + (*eng_ptrk_indxs5));
			eng_ptrk_stat_4->psp--;
			eng_ptrk_stat_4->ppos = *eng_ptrk_stat_4->psp;

			if (!--eng_tracks_active5) break;
		}
		eng_ptrk_indxs5++;
	}

	ptk_stat->flags |= TRK_SKIP;
}

extern sequence_status			*eng_seq_stat_6;		//L8007F0E0
extern track_status			    *eng_ptrk_stat_5;		//L8007F0E4
extern unsigned char			*eng_ptrk_indxs6;		//L8007F0E8
extern unsigned char			eng_tracks_active6;		//L8007F0EC
extern unsigned long			eng_max_trks_perseq;	//L8007F0F0

void Eng_SeqEnd (track_status *ptk_stat)//L8004839C()
{
	//printf("Eng_SeqEnd\n");

	if (!(ptk_stat->flags & TRK_HANDLED))
	{
		eng_seq_stat_6 = (eng_pmsbase->pseqstattbl + ptk_stat->seq_owner);
		eng_ptrk_indxs6 = eng_seq_stat_6->ptrk_indxs;
		eng_tracks_active6 = eng_seq_stat_6->tracks_active;
		eng_max_trks_perseq = *(char *)&eng_pmsbase->max_trks_perseq;//char??

		while (eng_max_trks_perseq--)
		{
			if (*eng_ptrk_indxs6 != 0xFF)
			{
				eng_ptrk_stat_5 = (eng_ptsbase + *eng_ptrk_indxs6);
				CmdFuncArr[eng_ptrk_stat_5->patchtype][TrkOff]((track_status *)eng_ptrk_stat_5);

				if (!--eng_tracks_active6) break;
			}

			eng_ptrk_indxs6++;
		}
	}
	else
	{
		eng_seq_stat_6 = (eng_pmsbase->pseqstattbl + ptk_stat->seq_owner);
		eng_ptrk_indxs6 = eng_seq_stat_6->ptrk_indxs;
		eng_tracks_active6 = eng_seq_stat_6->tracks_active;
		eng_max_trks_perseq = *(char *)&eng_pmsbase->max_trks_perseq;//char??

		while (eng_max_trks_perseq--)
		{
			if (*eng_ptrk_indxs6 != 0xFF)
			{
				eng_ptrk_stat_5 = (eng_ptsbase + (*eng_ptrk_indxs6));
				CmdFuncArr[eng_ptrk_stat_5->patchtype][TrkOff]((track_status *)eng_ptrk_stat_5);

				if (!--eng_tracks_active6) break;
			}

			eng_ptrk_indxs6++;
		}

		ptk_stat->flags |= TRK_SKIP;
	}
}

void Eng_TrkTempo (track_status *ptk_stat)//L80048644()
{
    //printf("Eng_TrkTempo\n");
	ptk_stat->qpm = *(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8);
	ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(), ptk_stat->ppq, ptk_stat->qpm);
}

void Eng_TrkGosub (track_status *ptk_stat)//L800486A0()
{
	unsigned int position;
	//printf("Eng_TrkGosub\n");

	position = (unsigned int)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	if ((position >= 0) && (position < ptk_stat->labellist_count))
	{
		*ptk_stat->psp = (ptk_stat->ppos + skip_table[26]);
		ptk_stat->psp++;
		ptk_stat->ppos = ptk_stat->pstart + *(ptk_stat->plabellist + position);
		ptk_stat->flags |= TRK_SKIP;
	}
}

void Eng_TrkJump (track_status *ptk_stat)//L80048734()
{
	unsigned int position;

	//printf("Eng_Jump\n");
	position = (unsigned int)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));

	if ((position >= 0) && (position < ptk_stat->labellist_count))
	{
		ptk_stat->ppos = ptk_stat->pstart + *(ptk_stat->plabellist + position);
		ptk_stat->deltatime = 0;
		ptk_stat->flags |= TRK_SKIP;
	}
}

void Eng_TrkRet (track_status *ptk_stat)//L800487A4()
{
    //printf("Eng_TrkRet\n");
	ptk_stat->psp--;
	ptk_stat->ppos = *ptk_stat->psp;
	ptk_stat->ppos = Read_Vlq(ptk_stat->ppos, &ptk_stat->deltatime);
	ptk_stat->flags |= TRK_SKIP;
}

void Eng_TrkEnd (track_status *ptk_stat)//L800487F8()
{
	//printf("Eng_TrkEnd\n");
	if (!(ptk_stat->flags & TRK_HANDLED))
    {
        if (!(ptk_stat->flags & TRK_LOOPED) || (ptk_stat->totppi < 16))
        {
            CmdFuncArr[ptk_stat->patchtype][TrkOff](ptk_stat);
            return;
        }
    }
    else
    {
        if (!(ptk_stat->flags & TRK_LOOPED) || (ptk_stat->totppi < 16))
        {
            CmdFuncArr[ptk_stat->patchtype][TrkOff](ptk_stat);
            ptk_stat->flags |= TRK_SKIP;
            return;
        }
    }

    ptk_stat->flags |= TRK_SKIP;
    ptk_stat->ppos = ptk_stat->pstart;
    ptk_stat->ppos = Read_Vlq(ptk_stat->pstart, &ptk_stat->deltatime);
}

void Eng_NullEvent (track_status *ptk_stat)//L800488FC()
{
    //printf("Eng_NullEvent\n");
}

extern track_status *eng_tmp_ptrkstattbl;      //L8007F0F4
extern unsigned int eng_tmp_trks_active;       //L8007F0F8
extern unsigned int eng_tmp_trk_work_areas;    //L8007F0FC
extern char         *eng_tmp_ppos;             //L8007F100

void SeqEngine(void)//L80048904()
{
    track_status *ptrkstattbl;

    eng_tmp_trks_active = eng_pmsbase->trks_active;
    if(eng_tmp_trks_active)
    {
        //printf("eng_tmp_trks_active %d\n",eng_tmp_trks_active);
        eng_tmp_ptrkstattbl = eng_ptsbase;
        eng_tmp_trk_work_areas = eng_ntwa;

        while(eng_tmp_trk_work_areas--)
        {
            ptrkstattbl = eng_tmp_ptrkstattbl;
            if((ptrkstattbl->flags & TRK_ACTIVE))
            {
                if(!(ptrkstattbl->flags & TRK_STOPPED))
                {

                    ptrkstattbl->starppi += (ptrkstattbl->ppi);
                    ptrkstattbl->totppi += (ptrkstattbl->starppi >> 0x10);
                    ptrkstattbl->accppi += (ptrkstattbl->starppi >> 0x10);
                    ptrkstattbl->starppi &= 65535;
                    //printf("ptrkstattbl->starppi %d\n",ptrkstattbl->starppi);
                    //printf("ptrkstattbl->totppi %d\n",ptrkstattbl->totppi);
                    //printf("ptrkstattbl->accppi %d\n",ptrkstattbl->accppi);

                    if(!(ptrkstattbl->flags & TRK_TIMED) || (ptrkstattbl->totppi < ptrkstattbl->endppi))
                    {
                        while(eng_tmp_ptrkstattbl->deltatime <= eng_tmp_ptrkstattbl->accppi &&
                            ((eng_tmp_ptrkstattbl->flags & (TRK_STOPPED|TRK_ACTIVE)) == TRK_ACTIVE))
                        {
                            ptrkstattbl->accppi -= ptrkstattbl->deltatime;
                            eng_tmp_ppos =  (ptrkstattbl->ppos);

                            if (!(*eng_tmp_ppos < 7) && (*eng_tmp_ppos < 19))
							{
									CmdFuncArr[ptrkstattbl->patchtype][*eng_tmp_ppos]((track_status *)ptrkstattbl);

									ptrkstattbl->ppos += skip_table[*eng_tmp_ppos];
									ptrkstattbl->ppos = Read_Vlq(ptrkstattbl->ppos, &ptrkstattbl->deltatime);
							}
							else if(!(*eng_tmp_ppos < 19) && (*eng_tmp_ppos < 36))
							{
								DrvFunctions[*eng_tmp_ppos]((track_status *)ptrkstattbl);

                                if ((ptrkstattbl->flags & (TRK_ACTIVE|TRK_SKIP)) == TRK_ACTIVE)
                                {
                                    ptrkstattbl->ppos += skip_table[*eng_tmp_ppos];
                                    ptrkstattbl->ppos = Read_Vlq(ptrkstattbl->ppos, &ptrkstattbl->deltatime);
                                }
                                else
                                {
                                    ptrkstattbl->flags &= ~TRK_SKIP;
                                }
							}
							else
							{
								Eng_SeqEnd((track_status *)ptrkstattbl);
							}
                        }
                    }
                    else
                    {
                            CmdFuncArr[ptrkstattbl->patchtype][5]((track_status *)ptrkstattbl);
                    }
                }

                if(!--eng_tmp_trks_active) break;
            }

            eng_tmp_ptrkstattbl++;
        }
    }
    CmdFuncArr[eng_ptsbase->patchtype][2]((track_status *)eng_ptsbase);
}
