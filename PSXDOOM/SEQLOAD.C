// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update 20/10/2019 [GEC]

#include "seqload.h"
#define _ALIGN4_ 1

#include <stdio.h>

extern pmasterstat *pm_stat;

extern int seq_loader_enable;           //80075790
extern char *loaderfile;		        //80075794
extern pmasterstat *ref_pm_stat;        //80075798
extern int ref_max_seq_num;             //8007579C
extern char Driver_Init;                //800757A0
extern char Reset_Gates;                //800757A1
extern int	opencount;				    //800757A4
extern int(*Seq_Error_func)(int, int);  //800757A8
extern int Seq_Error_module;            //800757AC
extern Wess_File_IO_Struct *fp_seq_file; //800757B0

extern track_header scratch_trk_hdr;    //8007EDE8
extern track_header seq_track_header;   //8007EE74
extern track_header base_track_header;  //8007EE8C

static void wess_seq_err(int code)//800444B0
{
    if (Seq_Error_func) {
		Seq_Error_func(Seq_Error_module, code);
	}
}

void wess_seq_loader_install_error_handler(int (*error_func)(int, int), int module)//800444E8
{
    Seq_Error_func = error_func;
	Seq_Error_module = module;
}

int Is_Seq_Seq_Num_Valid(int seqnum)//80044500
{
    if (seqnum >= 0)
    {
        if(ref_max_seq_num <= seqnum)
            return 0;

        return 1;
    }
    return 0;
}

int open_sequence_data(void)//8004452C
{
    if (opencount == 0)
    {
        fp_seq_file = (Wess_File_IO_Struct *)module_open(loaderfile);
        if(fp_seq_file == 0)
        {
            wess_seq_err(SEQLOAD_FOPEN);
            return 0;
        }
    }

    opencount += 1;
    return 1;
}

void close_sequence_data(void)//80044598
{
    if (opencount == 1)
	{
		module_close(fp_seq_file);
	}
	if (opencount > 0)
	{
		opencount -= 1;
	}
}

int wess_seq_load_sub(int seqnum, void *memptr)//800445EC
{
	sequence_data *psq_info;
	track_data	  *ptrk_info;
	int seqload;
	int seqread;
	int seqseek;
	int numtracks, k, loadit, tracknum, readbytes;
	char *pmem;

	//printf("wess_seq_load_sub\n");

	pmem = memptr;

	if (seq_loader_enable)
	{
		if (!Is_Seq_Seq_Num_Valid(seqnum))
			return 0;

		if (!open_sequence_data())
		{
			wess_seq_err(SEQLOAD_FOPEN);
			return 0;
		}

		psq_info = (ref_pm_stat->pmod_info->pseq_info + seqnum); /* pointer math */
		psq_info->ptrk_info = (track_data *)pmem;

		if (psq_info->trkstoload == 0)
			pmem += sizeof(track_data);
		else
			pmem += (psq_info->trkstoload * sizeof(track_data));

		seqseek = module_seek(fp_seq_file, psq_info->fileposition, PSXCD_SEEK_SET);
		if (seqseek)
		{
			wess_seq_err(SEQLOAD_FSEEK);
			return 0;
		}

		readbytes = sizeof(seq_header);
		seqread = module_read(&(ref_pm_stat->pmod_info->pseq_info + seqnum)->seq_hdr, readbytes, fp_seq_file);

		if (seqread != readbytes)
		{
			wess_seq_err(SEQLOAD_FREAD);
			return 0;
		}

		numtracks = (ref_pm_stat->pmod_info->pseq_info + seqnum)->seq_hdr.tracks;

		tracknum = 0;
		while (numtracks--)
		{
			readbytes = sizeof(track_header);
			seqread = module_read(&seq_track_header, readbytes, fp_seq_file);

			if (seqread != readbytes)
			{
				wess_seq_err(SEQLOAD_FREAD);
				return 0;
			}

			loadit = 0;

			if ((seq_track_header.voices_type == NoSound_ID) ||
				(seq_track_header.voices_type == GENERIC_ID))
			{
				loadit = 1;
			}
			else
			{
				k = ref_pm_stat->patch_types_loaded;
				while (k--)
				{
					if (((seq_track_header.voices_type == (ref_pm_stat->ppat_info + k)->hw_tl_list.hardware_ID) &&
						(((seq_track_header.voices_class == SNDFX_CLASS) && ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_SOUND_EFFECTS)) ||
						((seq_track_header.voices_class == SFXDRUMS_CLASS) && ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_SOUND_EFFECTS)) ||
							((seq_track_header.voices_class == MUSIC_CLASS) && ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_MUSIC)) ||
							((seq_track_header.voices_class == DRUMS_CLASS) && ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_DRUMS)))))
					{
						loadit = 1;
						break;
					}
				}
			}

			if (loadit == 0)
			{
				seqseek = module_seek(fp_seq_file, (seq_track_header.labellist_count * sizeof(long)) + seq_track_header.data_size, PSXCD_SEEK_CUR);
				if (seqseek != 0)
				{
					wess_seq_err(SEQLOAD_FSEEK);
					return 0;
				}
			}
			else
			{
				ptrk_info = ((ref_pm_stat->pmod_info->pseq_info + seqnum)->ptrk_info + tracknum);

				memcpy(&ptrk_info->trk_hdr, &seq_track_header, sizeof(seq_track_header));

				if (seq_track_header.voices_type == GENERIC_ID)
				{
					ptrk_info->trk_hdr.voices_type = NoSound_ID;

					if (seq_track_header.voices_class == SNDFX_CLASS || seq_track_header.voices_class == SFXDRUMS_CLASS)
					{
						for (k = 0; k < ref_pm_stat->patch_types_loaded; k++)
						{
							if ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load  & TAG_SOUND_EFFECTS)
							{
								ptrk_info->trk_hdr.voices_type = (unsigned char)(pm_stat->ppat_info + k)->hw_tl_list.hardware_ID;
								break;
							}
						}
					}
					else if (scratch_trk_hdr.voices_class == MUSIC_CLASS)
					{
						for (k = 0; k < ref_pm_stat->patch_types_loaded; k++)
						{
							if ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load  & TAG_MUSIC)
							{
								ptrk_info->trk_hdr.voices_type = (unsigned char)(pm_stat->ppat_info + k)->hw_tl_list.hardware_ID;
								break;
							}
						}
					}
					else if (scratch_trk_hdr.voices_class == DRUMS_CLASS)
					{
						for (k = 0; k < ref_pm_stat->patch_types_loaded; k++)
						{
							if ((ref_pm_stat->ppat_info + k)->hw_tl_list.flags_load  & TAG_DRUMS)
							{
								ptrk_info->trk_hdr.voices_type = (unsigned char)(pm_stat->ppat_info + k)->hw_tl_list.hardware_ID;
								break;
							}
						}
					}
				}

				ptrk_info->plabellist = (unsigned long *)pmem;
				pmem += (ptrk_info->trk_hdr.labellist_count * sizeof(long));

				readbytes = (ptrk_info->trk_hdr.labellist_count * sizeof(long));
				seqread = module_read(ptrk_info->plabellist, readbytes, fp_seq_file);

				if (seqread != readbytes)
				{
					wess_seq_err(SEQLOAD_FREAD);
					return 0;
				}

				ptrk_info->ptrk_data = (char *)pmem;
				pmem += (ptrk_info->trk_hdr.data_size);

#if _ALIGN4_ == 1
				//force align to word boundary because previous pmem adjust
				//may wind up with odd address
				pmem += (unsigned int)pmem & 1;
				pmem += (unsigned int)pmem & 2;
#endif

				readbytes = (ptrk_info->trk_hdr.data_size);
				seqread = module_read(ptrk_info->ptrk_data, readbytes, fp_seq_file);

				if (seqread != readbytes)
				{
					wess_seq_err(SEQLOAD_FREAD);
					return 0;
				}
			}

			tracknum++;
		}

		if (psq_info->trkstoload == 0)
		{
			ptrk_info = (ref_pm_stat->pmod_info->pseq_info + seqnum)->ptrk_info;

			memcpy(&ptrk_info->trk_hdr, &base_track_header, sizeof(base_track_header));

			ptrk_info->plabellist = (unsigned long *)pmem;
			ptrk_info->ptrk_data = (char *)pmem;

			*ptrk_info->ptrk_data = Driver_Init;
			*(ptrk_info->ptrk_data + 1) = Reset_Gates;
			pmem += (ptrk_info->trk_hdr.data_size);

#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
			pmem += (unsigned int)pmem & 1;
			pmem += (unsigned int)pmem & 2;
#endif
			(ref_pm_stat->pmod_info->pseq_info+seqnum)->seq_hdr.tracks = 1;
		}
		else
		{
			(ref_pm_stat->pmod_info->pseq_info+seqnum)->seq_hdr.tracks = psq_info->trkstoload;
		}

		close_sequence_data();
	}

	k = (unsigned int)((unsigned)pmem - (unsigned)memptr);
	return k;
}

int wess_seq_loader_init(void *input_pm_stat, char *seqfile, enum OpenSeqHandleFlag flag)//80044D98
{
    //printf("wess_seq_loader_init\n");

    seq_loader_enable = 0;
    loaderfile = seqfile;
    ref_pm_stat = (pmasterstat *)input_pm_stat;

    if (ref_pm_stat)
    {
		seq_loader_enable = 1;
        ref_max_seq_num = ref_pm_stat->pmod_info->mod_hdr.sequences;

        base_track_header.priority = 0x80;
        base_track_header.initvolume_cntrl = 0x7f;
        base_track_header.initpan_cntrl = 0x40;
        base_track_header.initppq = 120;
        base_track_header.initqpm = 120;
        base_track_header.voices_type = 0;
        base_track_header.voices_max = 0;
        base_track_header.reverb = 0;
        base_track_header.voices_class = 0;
        base_track_header.initpatchnum = 0;
        base_track_header.initpitch_cntrl = 0;
        base_track_header.substack_count = 0;
        base_track_header.mutebits = 0;
        base_track_header.labellist_count = 0;
        base_track_header.data_size = 2;

		if (flag == YesOpenSeqHandle)
		{
			if (!open_sequence_data())
			{
				wess_seq_err(SEQLOAD_FOPEN);
				return 0;
			}
		}
	}

	return 1;
}

void wess_seq_loader_exit(void)//80044EAC
{
    close_sequence_data();
    seq_loader_enable = 0;
}

int wess_seq_sizeof(int seqnum)//80044ED4
{
    sequence_data *psq_info;

	if (seq_loader_enable)
	{
		if (!Is_Seq_Seq_Num_Valid(seqnum))
			return 0;

		psq_info = ref_pm_stat->pmod_info->pseq_info+seqnum; /* pointer math */

		if (psq_info->ptrk_info)
			return 0;

		return (psq_info->trkinfolength);
	}
	return 0;
}


int wess_seq_load(int seqnum, void *memptr)//80044F64
{
    if (seq_loader_enable)
	{
		if (!Is_Seq_Seq_Num_Valid(seqnum))
            return 0;

		if ((ref_pm_stat->pmod_info->pseq_info+seqnum)->ptrk_info)
			return 0;

        //printf("wess_seq_load %d\n", seqnum);
		return wess_seq_load_sub(seqnum, memptr);
	}

	return 0;
}

int wess_seq_free(int seqnum)//80045008
{
    sequence_data *psq_info;

	if (seq_loader_enable)
	{
		if (!Is_Seq_Seq_Num_Valid(seqnum))
			return 0;

		psq_info = ref_pm_stat->pmod_info->pseq_info+seqnum; /* pointer math */

		if (psq_info->ptrk_info)
		{
			psq_info->ptrk_info = 0;
			return 1;
		}
	}
	return 0;
}
