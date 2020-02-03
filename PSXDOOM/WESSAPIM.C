// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include "kernel.h"
#include "wessarc.h"
#include "wessapi.h"
#include "wessseq.h"

extern unsigned char	master_sfx_volume;	//*L80075834
extern unsigned char	master_mus_volume;	//*L80075835
extern unsigned char	pan_status;			//*L80075836
extern unsigned char	release_rate;		//*L80075837
extern pmasterstat      *pm_stat;           //800A8538
extern int              SeqOn;              //80075778

char wess_master_sfx_volume_get (void)//80049548
{
#ifndef BLOCK_SOUND_WESS
    if (!Is_Module_Loaded())
	{
		return 0;
	}

	return (master_sfx_volume);
#endif
}

char wess_master_mus_volume_get (void)//80049578
{
#ifndef BLOCK_SOUND_WESS
    if (!Is_Module_Loaded())
	{
		return 0;
	}

	return (master_mus_volume);
#endif
}

void wess_master_sfx_vol_set (char volume)//800495A8
{
#ifndef BLOCK_SOUND_WESS
    if (!Is_Module_Loaded())
	{
		return;
	}

	master_sfx_volume = volume;
#endif
}
void wess_master_mus_vol_set (char volume)//800495E0
{
#ifndef BLOCK_SOUND_WESS
    char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	char *temp;
	int li, lj;
	char action[8];

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate pause of all sequences */
	SeqOn = 0;

	master_mus_volume = volume;

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

						if (ptmp->sndclass == MUSIC_CLASS)
						{
						    temp = ptmp->ppos; //copy
							ptmp->ppos = action;//action
							action[0] = 12;
							ptmp->ppos[1] = ptmp->volume_cntrl;
							CmdFuncArr[ptmp->patchtype][12](ptmp);
							ptmp->ppos = temp; //restore
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

	SeqOn = 1;
#endif
}

char wess_pan_mode_get (void)//800497A0
{
#ifndef BLOCK_SOUND_WESS
    return pan_status;
#endif
}

void wess_pan_mode_set (char mode)//800497B0
{
#ifndef BLOCK_SOUND_WESS
    pan_status = mode;
#endif
}


