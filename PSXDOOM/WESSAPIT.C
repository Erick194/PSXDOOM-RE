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

void updatetrackstat(track_status *ptk_stat, TriggerPlayAttr *attr)//L80043E08
{
	//CODE----------------------------------------------------
	int tempmask;
	char *temp;
	char action[8];

	if ((attr == NULL) || (!attr->mask))
		return;

	tempmask = attr->mask;

	if (tempmask & TRIGGER_VOLUME) //0x01
	{
		ptk_stat->volume_cntrl = attr->volume;
	}

	if (tempmask & TRIGGER_PAN) //0x02
	{
		ptk_stat->pan_cntrl = attr->pan;
	}

	if (tempmask & (TRIGGER_VOLUME | TRIGGER_PAN)) //0x03
	{
		ptk_stat->volume_cntrl = attr->volume;
		ptk_stat->pan_cntrl = attr->pan;

		temp = ptk_stat->ppos; //copy
		ptk_stat->ppos = action;  //action
		action[0] = 12;
		ptk_stat->ppos[1] = ptk_stat->volume_cntrl;
		CmdFuncArr[ptk_stat->patchtype][12](ptk_stat);
		ptk_stat->ppos = temp; //restore
	}
	else
	{
		if (tempmask & TRIGGER_VOLUME) //0x01
		{
			ptk_stat->volume_cntrl = attr->volume;

			temp = ptk_stat->ppos; //copy
			ptk_stat->ppos = action;  //action
			action[0] = 12;
			ptk_stat->ppos[1] = ptk_stat->volume_cntrl;
			CmdFuncArr[ptk_stat->patchtype][12](ptk_stat);
			ptk_stat->ppos = temp; //restore
		}
		else if (tempmask & TRIGGER_PAN)
		{
			ptk_stat->pan_cntrl = attr->pan;

			temp = ptk_stat->ppos; //copy
			ptk_stat->ppos = action;  //action
			action[0] = 13;
			ptk_stat->ppos[1] = ptk_stat->pan_cntrl;
			CmdFuncArr[ptk_stat->patchtype][13](ptk_stat);
			ptk_stat->ppos = temp; //restore
		}
	}

	if (tempmask & TRIGGER_PATCH) //0x04
	{
		ptk_stat->patchnum = attr->patch;
	}

	if (tempmask & TRIGGER_PITCH) //0x08
	{
		ptk_stat->pitch_cntrl = attr->pitch;

		temp = ptk_stat->ppos; //copy
		ptk_stat->ppos = action;  //action
		action[0] = 9;
		ptk_stat->ppos[1] = (char)(ptk_stat->pitch_cntrl & 0xff);
		ptk_stat->ppos[2] = (char)(ptk_stat->pitch_cntrl >> 8);
		CmdFuncArr[ptk_stat->patchtype][9](ptk_stat);
		ptk_stat->ppos = temp; //restore
	}

	if (tempmask & TRIGGER_MUTEMODE) //0x10
	{
		if (ptk_stat->mutemask & (1 << attr->mutemode))
		{
			ptk_stat->flags |= TRK_MUTE;
		}
		else {
			ptk_stat->flags &= ~TRK_MUTE;
		}
	}

	if (tempmask & TRIGGER_TEMPO) //0x20
	{
		ptk_stat->qpm = attr->tempo;
		ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(),
			ptk_stat->ppq,
			ptk_stat->qpm);
	}

	if (tempmask & TRIGGER_TIMED) //0x40
	{
		ptk_stat->endppi = ptk_stat->totppi + attr->timeppq;
		ptk_stat->flags |= TRK_TIMED;
	}

	if (tempmask&TRIGGER_LOOPED) //0x80
	{
		ptk_stat->flags |= TRK_LOOPED;
	}
}

void wess_seq_trigger_type (int seq_num, unsigned long seq_type)//800440FC
{
#ifndef BLOCK_SOUND_WESS

    sequence_data *psq_info;

    psq_info = pm_stat->pmod_info->pseq_info+seq_num; /* pointer math */

    wess_seq_structrig (psq_info, seq_num, seq_type, NoHandle, NULL);
#endif
}

void wess_seq_trigger_type_special (int              seq_num,
                                    unsigned long    seq_type,
                                    TriggerPlayAttr *attr)//8004414C
{
#ifndef BLOCK_SOUND_WESS

    sequence_data *psq_info;

    psq_info = pm_stat->pmod_info->pseq_info+seq_num; /* pointer math */

    wess_seq_structrig (psq_info, seq_num, seq_type, NoHandle, attr);
#endif
}

void wess_seq_update_type_special(unsigned long seq_type, TriggerPlayAttr *attr)//800441A0
{
#ifndef BLOCK_SOUND_WESS

	/* immediate stop of sequence */
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	if (!Is_Module_Loaded())
	{
		return;
	}

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
				if (psq_stat->seq_type == seq_type)
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
							if (attr != 0)
							{
								updatetrackstat(ptmp, attr);
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

void wess_seq_stoptype (unsigned long type_number)//8004431C
{
#ifndef BLOCK_SOUND_WESS

    /* immediate stop of sequence */
    char nt,na;
    sequence_status *psq_stat;
    track_status *ptmp;
    char *lpdest;
    int li,lj;

    if(!Is_Module_Loaded())
    {
        return;
    }

    SeqOn = 0;

    /* search for all sequences with this number and turn them off */
    nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
    na = pm_stat->seqs_active;
    psq_stat = pm_stat->pseqstattbl;
    if(na)
    {
        while(nt--)
        {
            if (psq_stat->flags & SEQ_ACTIVE)
            {
                if(psq_stat->seq_type==type_number)
                {
                    li = psq_stat->tracks_active;
                    lj = pm_stat->max_trks_perseq;
                    /* *lpdest refers to an active track if not 0xFF */
                    lpdest = psq_stat->ptrk_indxs;
                    while(lj--)
                    {
                        if(*lpdest!=0xFF)
                        {
                            ptmp = (pm_stat->ptrkstattbl+(*lpdest));
                            CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
                            if(!--li) break;
                        }
                        lpdest++;
                    }
                }
                if(!--na) break;
            }
            psq_stat++;
        }
    }

    SeqOn = 1;
#endif
}
