// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include "kernel.h"
#include "wessarc.h"
#include "wessapi.h"
#include "wessseq.h"

extern int num_sd;                              //80075714
extern char *wmd_file_ptr;                      //80075718
extern char *wmd_file_ptr2;                     //8007571C
extern Wess_File_IO_Struct *fp_wmd_file;        //80075720
extern int sysinit;                             //80075724
extern int module_loaded;                       //80075728
extern int early_exit;                          //8007572C
extern int max_seq_num;                         //80075730
extern int mem_limit;                           //80075734
extern patch_group_header scratch_pat_grp_hdr;  //8007EDE8
extern track_header scratch_trk_hdr;            //8007EE04
extern int wmd_mem_is_mine;                     //80075738
extern char *wmd_mem;                           //8007573C
extern char *wmd_end;                           //80075740
extern int wmd_size;                            //80075744
extern int (*Error_func)(int, int);             //80075748
extern int Error_module;                        //8007574C
extern pmasterstat *pm_stat;                    //800A8538

void trackstart(track_status *ptmp, sequence_status *psq_stat)//800414A4
{
#ifndef BLOCK_SOUND_WESS

	if (ptmp->flags & TRK_STOPPED)
	{
		ptmp->flags &= ~TRK_STOPPED;
		if (++psq_stat->tracks_playing)
		{
			psq_stat->playmode = SEQ_STATE_PLAYING;
		}
	}
#endif
}

void trackstop(track_status *ptmp, sequence_status *psq_stat)//800414E8
{
#ifndef BLOCK_SOUND_WESS

	if (!(ptmp->flags & TRK_STOPPED))
	{
		ptmp->flags |= TRK_STOPPED;
		if (!--psq_stat->tracks_playing)
		{
			psq_stat->playmode = SEQ_STATE_STOPPED;
		}
	}
#endif
}


void wess_seq_pause(int sequence_number, enum MuteFlag mflag)//80041528
{
#ifndef BLOCK_SOUND_WESS

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	if (!Is_Seq_Num_Valid(sequence_number))
	{
		return;
	}

	/* immediate pause of sequence */
	SeqOn = 0;

	/* search for all sequences with this number and turn them off */
	nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_num == sequence_number)
				{
					li = psq_stat->tracks_active;
					lj = pm_stat->max_trks_perseq;
					/* *lpdest refers to an active track if not 0xFF */
					lpdest = psq_stat->ptrk_indxs;
					while (lj--)
					{
						if (*lpdest != 0xFF)
						{
							ptmp = (pm_stat->ptrkstattbl + (*lpdest));
							trackstop(ptmp, psq_stat);
							if (mflag == YesMute)
							{
								CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
							}
							if (!--li) break;
						}
						lpdest++;
					}
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	SeqOn = 1;
#endif
}

void wess_seq_restart(int sequence_number)//800416D0
{
#ifndef BLOCK_SOUND_WESS

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	if (!Is_Seq_Num_Valid(sequence_number))
	{
		return;
	}

	/* immediate restart of sequence */
	SeqOn = 0;

	/* search for all sequences with this number and turn them off */
	nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_num == sequence_number)
				{
					li = psq_stat->tracks_active;
					lj = pm_stat->max_trks_perseq;
					/* *lpdest refers to an active track if not 0xFF */
					lpdest = psq_stat->ptrk_indxs;
					while (lj--)
					{
						if (*lpdest != 0xFF)
						{
							ptmp = (pm_stat->ptrkstattbl + (*lpdest));
							trackstart(ptmp, psq_stat);
							if (!--li) break;
						}
						lpdest++;
					}
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	SeqOn = 1;
#endif
}

void wess_seq_pauseall(enum MuteFlag mflag, NoteState *pns)//8004183C
{
#ifndef BLOCK_SOUND_WESS

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate pause of all sequences */
	SeqOn = 0;

	//Doom PSX New
	if (mflag == YesMute)
	{
		start_record_music_mute(pns);
	}

	/* search for all sequences with this number and turn them off */
	nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				li = psq_stat->tracks_active;
				lj = pm_stat->max_trks_perseq;
				/* *lpdest refers to an active track if not 0xFF */
				lpdest = psq_stat->ptrk_indxs;
				while (lj--)
				{
					if (*lpdest != 0xFF)
					{
						ptmp = (pm_stat->ptrkstattbl + (*lpdest));
						if (mflag == YesMute)
						{
							CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
						}
						trackstop(ptmp, psq_stat);
						if (!--li) break;
					}
					lpdest++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	//Doom PSX New
	if (mflag == YesMute)
	{
		end_record_music_mute();
	}

	SeqOn = 1;
#endif
}

void wess_seq_restartall(NoteState *pns)//800419f8
{
#ifndef BLOCK_SOUND_WESS

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj, nc;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate restart of all sequences */
	SeqOn = 0;

	/* search for all sequences with this number and turn them off */
	nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				li = psq_stat->tracks_active;
				lj = pm_stat->max_trks_perseq;
				/* *lpdest refers to an active track if not 0xFF */
				lpdest = psq_stat->ptrk_indxs;
				while (lj--)
				{
					if (*lpdest != 0xFF)
					{
						ptmp = (pm_stat->ptrkstattbl + (*lpdest));
						trackstart(ptmp, psq_stat);

						if (pns != 0)
						{
							for (nc = 0; nc < pns->numnotes; nc++)
							{
								if (*lpdest == pns->nd[nc].track)
								{
									if (psq_stat->seq_num == pns->nd[nc].seq_num)
									{
										PSX_voicenote(ptmp,
												  pns->nd[nc].patchmap,
												  pns->nd[nc].patchinfo,
												  pns->nd[nc].keynum,
												  pns->nd[nc].velnum);
									}
								}
							}
						}

						if (!--li) break;
					}
					lpdest++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	if (pns != 0)
	{
		pns->numnotes = 0;
	}

	SeqOn = 1;
#endif
}
