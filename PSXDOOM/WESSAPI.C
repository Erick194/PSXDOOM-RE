// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

    /*-----------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                    Application Programming Interface Routines
                            by Scott Patterson
    */
    /*----------------------------------------------------------------*/
//#include "win_port.h"

//#ifndef BLOCK_SOUND_WESS
#include "kernel.h"
#include "wessarc.h"
#include "wessapi.h"
#include "wessseq.h"
//#include "wessapix.h"

#define MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE 4//10

#define _ALIGN4_ 1

static int conditional_read(int    readflag,
                            char **ptrtomem,
                            int    readsize);

static void filltrackstat(track_status *ptk_stat,
                          track_data *ptk_info,
                          TriggerPlayAttr *attr);

static void assigntrackstat(track_status *ptk_stat,
                            track_data *ptk_info);

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

#if 0 /* NO USED */
static void err(int code) {

    if(Error_func) {
        Error_func(Error_module, code);
    }
}
#endif

static void zeroset(char *pdest, unsigned long size)//80041BE8
{
    while(size--) *pdest++ = 0;
}

void wess_install_error_handler(int (*error_func)(int, int), int module)//80041C14
{
#ifndef BLOCK_SOUND_WESS
    Error_func=error_func;
    Error_module=module;
#endif
}

void * wess_get_master_status (void)//80041C2C
{
#ifndef BLOCK_SOUND_WESS

    return pm_stat;
#endif
}

int Is_System_Active(void)//80041C3C
{
#ifndef BLOCK_SOUND_WESS
    if(!sysinit)
    {
        return 0;
    }
    return 1;
#endif
}

int Is_Module_Loaded(void)//80041C4C
{
#ifndef BLOCK_SOUND_WESS
    if(!module_loaded)
    {
        return 0;
    }
    return 1;
#endif
}

int Is_Seq_Num_Valid(int seq_num)//80041C5C
{
 #ifndef BLOCK_SOUND_WESS
    if((seq_num<0) || (seq_num>=max_seq_num))
    {
        return 0;
    } else if((pm_stat->pmod_info->pseq_info+seq_num)->ptrk_info==NULL) {
        return 0;
    }
    return 1;
#endif
}

void Register_Early_Exit(void)//80041CB8
{
#ifndef BLOCK_SOUND_WESS
    if(!early_exit)
    {
        early_exit = 1;
    }
#endif
}

void wess_install_handler (void)//80041CDC
{
#ifndef BLOCK_SOUND_WESS
    init_WessTimer();
#endif
}

void wess_restore_handler (void)//80041CFC
{
#ifndef BLOCK_SOUND_WESS
    exit_WessTimer();
#endif
}

int wess_init (void)//80041D1C
{
#ifndef BLOCK_SOUND_WESS
    int initok;

    initok = 0;

    if(!sysinit)
    {
        SeqOn = 0; /* make sure the SeqEngine is disabled */

        if(!WessTimerActive)
        {
            wess_install_handler();
        }
        wess_low_level_init();

        sysinit = 1;
        initok = 1;
    }
    return (initok);
#endif
}

void wess_exit (enum RestoreFlag rflag)//80041D80
{
#ifndef BLOCK_SOUND_WESS

    if(!Is_System_Active())
    {
        return;
    }

    if(sysinit)
    {
        if(module_loaded)
        {
            wess_unload_module();
        }

        wess_low_level_exit();
        sysinit = 0;
        if(rflag|WessTimerActive)
        {
            wess_restore_handler();
        }
    }
#endif
}


char *wess_get_wmd_start(void)//80041E0C
{
#ifndef BLOCK_SOUND_WESS
    return(wmd_mem);
#endif
}

char *wess_get_wmd_end(void)//80041E1C
{
#ifndef BLOCK_SOUND_WESS
    return(wmd_end);
#endif
}

static void free_mem_if_mine(void)//80041E2C
{
#ifndef BLOCK_SOUND_WESS
    if(wmd_mem_is_mine)
    {
        if(wmd_mem!=NULL)
        {
            wess_free(wmd_mem);
            wmd_mem=NULL;
        }
        wmd_mem_is_mine = 0;
    }
#endif
}

void wess_unload_module (void)//80041E7C
{
#ifndef BLOCK_SOUND_WESS
    int ptif,mi;

    if(module_loaded)
    {
        wess_seq_stopall();
        SeqOn = 0;

        /* shutdown the loaded drivers and SeqEngine */

        CmdFuncArr[NoSound_ID][DriverExit]((track_status *)pm_stat);

        ptif = pm_stat->patch_types_loaded;
        for(mi=0;mi<ptif;mi++)
        {
            if (((pm_stat->ppat_info + mi)->hw_tl_list.flags_load & (TAG_SOUND_EFFECTS| TAG_MUSIC| TAG_DRUMS)) != 0)
            {
                CmdFuncArr[(pm_stat->ppat_info+mi)->hw_tl_list.hardware_ID][DriverExit]((track_status *)pm_stat);
            }
        }

        free_mem_if_mine();

        module_loaded = 0;
    }
#endif
}

static void module_memcpy(void *pdest, void *psrc, unsigned long bytestocopy)//80041F88
{
    while(bytestocopy--)
    {
        *(char *)pdest++ = *(char *)psrc++;
    }
}

int conditional_read(int readflag, char **ptrtomem, int readsize)//80041FBC
{
#ifndef BLOCK_SOUND_WESS

    if (readflag)
	{
	    //printf("conditional_read %d\n", readsize);
		module_memcpy(*ptrtomem, wmd_file_ptr, readsize);
		wmd_file_ptr += readsize;
		*ptrtomem += readsize;

#if _ALIGN4_ == 1
		//force align to word boundary because previous
		//pmem adjust may wind up with odd address
		*ptrtomem += (unsigned int)*ptrtomem & 1;
		*ptrtomem += (unsigned int)*ptrtomem & 2;
#endif
	}
	else
	{
		wmd_file_ptr += readsize;
	}
	return (1);
#endif
}

int wess_load_module (void *wmd_ptr,
                      char *memory_pointer,
                      int memory_allowance,
                      int **settings_tag_lists)
{
#ifndef BLOCK_SOUND_WESS

    int i, j, k, n, z, types, num, indx, loadit;
	int tracks_toload;
	int readrequest, readresult;
	char max_tracks_inseq, max_voices_intrk, max_substack_intrk;
	char *pdest;
	char *pmem;
	unsigned long patfpos, trkinfosize;
	char *tempwmd;
	int setting, flag, flag2;

	mem_limit = memory_allowance;
	tempwmd = wmd_ptr;

	if (module_loaded)
	{
		wess_unload_module();
	}

	num_sd = get_num_Wess_Sound_Drivers(settings_tag_lists);
	//printf("num_sd %d\n", num_sd);

	if (memory_pointer == NULL)
	{
		wmd_mem_is_mine = 1;
		wmd_mem = (char *)wess_malloc(memory_allowance);
		if(wmd_mem == NULL)
		{
			return(module_loaded);
		}
	}
	else {
		wmd_mem_is_mine = 0;
		wmd_mem = memory_pointer;
	}

	wmd_size = memory_allowance;
	zeroset(wmd_mem, wmd_size);

	max_seq_num = 0;

	if (!Is_System_Active())
	{
		free_mem_if_mine();
		return (module_loaded);
	}

	if (tempwmd == NULL)
	{
		free_mem_if_mine();
		return (module_loaded);
	}

	/*if (!(fp_wmd_file = module_open(wmd_filename)))
	{
		err(wess_FOPEN);
		free_mem_if_mine();
		return (module_loaded);
	}*/

	max_tracks_inseq = MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE;
	//printf("max_tracks_inseq %d\n", max_tracks_inseq);
	max_voices_intrk = 0;
	max_substack_intrk = 0;

	/* loads a related group of patches and sequences */
	/* a module has the information necessary to set up sequencer work
	areas and data areas */
	/*
	The module loading sequence works as follows :
	*/

	pmem = wmd_mem;

	/*
	- allocate space for a master_status_structure
	- update the pmasterstat pointer
	*/

	pm_stat = (master_status_structure *)pmem;
	pmem += sizeof(*pm_stat);

	pm_stat->fp_module = (int)fp_wmd_file;//falta verificar
	pm_stat->pabstime = &millicount;

	/*
	- allocate for the module_data structure
	- update the pmod_info pointer
	*/

	pm_stat->pmod_info = (module_data *)pmem;
	pmem += sizeof(*pm_stat->pmod_info);

	/*
	- read in sizeof(pm_stat->pmod_info->mod_hdr)
	bytes from the .lmd file into the pm_stat->pmod_info->mod_hdr
	structure.
	*/

	wmd_file_ptr2 = tempwmd;
	wmd_file_ptr = wmd_file_ptr2;

	readrequest = sizeof(pm_stat->pmod_info->mod_hdr);
	//module_read(&pm_stat->pmod_info->mod_hdr, tempwmd, readrequest);
	module_memcpy(&pm_stat->pmod_info->mod_hdr, tempwmd, readrequest);
	wmd_file_ptr += readrequest;

	//readresult = module_read(&pm_stat->pmod_info->mod_hdr, readrequest, fp_wmd_file);
	/*if (readrequest != readresult)
	{
		//err(wess_FREAD);
		free_mem_if_mine();
		return(0);
	}*/

	if ((pm_stat->pmod_info->mod_hdr.module_id_text != WESS_SSSP_TEXT) ||
		(pm_stat->pmod_info->mod_hdr.module_version != WESS_CORE_VERSION))
	{
		free_mem_if_mine();
		return(0);
	}
	//printf("WESS_SSSP_TEXT %x\n", pm_stat->pmod_info->mod_hdr.module_id_text);
	//printf("WESS_CORE_VERSION %x\n", pm_stat->pmod_info->mod_hdr.module_version);

	/*
	--init work structures --------------------------------------------
	- allocate and initialize space for
	pmod_info->mod_hdr.seq_work_areas sequence_status structures
	and update the pmseqstattbl pointer and zero seqs_active.
	*/

	pm_stat->pseqstattbl = (sequence_status *)pmem;
	pmem += sizeof(*pm_stat->pseqstattbl) *
		pm_stat->pmod_info->mod_hdr.seq_work_areas;

	/*
	- allocate and initialize space for
	pmod_info->mod_hdr.trk_work_areas track_status structures
	and update the pmtrkstattbl pointer and zero trks_active.
	*/

	pm_stat->ptrkstattbl = (track_status *)pmem;
	pmem += sizeof(*pm_stat->ptrkstattbl) *
		pm_stat->pmod_info->mod_hdr.trk_work_areas;

	/*
	--load data structures --------------------------------------------
	- update pm_stat->patch_types with the sb_num global and
	allocate and copy in pm_stat->patch_types bytes
	of the patch_id_list and update the pm_stat->ppat_id_list pointer.
	*/

	pm_stat->patch_types_loaded = num_sd;

	/*
	- allocate and initialize space for master_volumes of each
	patch type loaded.
	*/

	pm_stat->pmaster_volume = (char *)pmem;
	pmem += sizeof(char) * pm_stat->patch_types_loaded;
#if _ALIGN4_ == 1
	//force align to word boundary because previous pmem adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
#endif


	pdest = pm_stat->pmaster_volume;
	n = pm_stat->patch_types_loaded;
	while (n--)
	{
		*pdest++ = 0x80;
	}


	/*
	- allocate and initialize space for
	pm_stat->patch_types patch_group_data structures
	and update the pm_stat->ppat_info pointer.
	*/

	pm_stat->ppat_info = (patch_group_data *)pmem;
	pmem += sizeof(*pm_stat->ppat_info) * pm_stat->patch_types_loaded;

	//printf("here\n");

	if (settings_tag_lists)
	{
		k = pm_stat->patch_types_loaded;
		//printf("k %d\n", k);
		while (k--)
		{
			/* get the settings tags for this patch type */

			/*
			copy settings_tag_lists to the patch's sndhw_tags array
			*/
			z = 0;

			while (settings_tag_lists[k][z] != SNDHW_TAG_END)
			{
				(pm_stat->ppat_info + k)->sndhw_tags[z] = settings_tag_lists[k][z];
				(pm_stat->ppat_info + k)->sndhw_tags[z + 1] = settings_tag_lists[k][z + 1];

				if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_DRIVER_ID)
				{
					(pm_stat->ppat_info + k)->hw_tl_list.hardware_ID = (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
				}
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_SOUND_EFFECTS)
				{
					setting = (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
					flag = (pm_stat->ppat_info + k)->hw_tl_list.flags_load;

					flag2 = flag & ~TAG_SOUND_EFFECTS;
					flag = (flag & 1 | setting) & 1;
					(pm_stat->ppat_info + k)->hw_tl_list.flags_load = (flag2 | flag);
				}
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_MUSIC)
				{
					setting = (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
					flag = (pm_stat->ppat_info + k)->hw_tl_list.flags_load;

					flag2 = flag & ~TAG_MUSIC;
					flag = ((flag >> 1 & 1 | setting) & 1) << 1;
					(pm_stat->ppat_info + k)->hw_tl_list.flags_load = (flag2 | flag);
				}
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_DRUMS)
				{
					setting = (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
					flag = (pm_stat->ppat_info + k)->hw_tl_list.flags_load;

					flag2 = flag & ~TAG_DRUMS;
					flag = ((flag >> 2 & 1 | setting) & 1) << 2;
					(pm_stat->ppat_info + k)->hw_tl_list.flags_load = (flag2 | flag);
				}

				/*if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_DRIVER_ID)
					(pm_stat->ppat_info + k)->hw_tl_list.hardware_ID = (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_SOUND_EFFECTS)
					(pm_stat->ppat_info + k)->hw_tl_list.sfxload |= (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_MUSIC)
					(pm_stat->ppat_info + k)->hw_tl_list.musload |= (pm_stat->ppat_info + k)->sndhw_tags[z + 1];
				else if ((pm_stat->ppat_info + k)->sndhw_tags[z] == SNDHW_TAG_DRUMS)
					(pm_stat->ppat_info + k)->hw_tl_list.drmload |= (pm_stat->ppat_info + k)->sndhw_tags[z + 1];*/

				z += 2;
			}
		}
	}

	//printf("z %d\n", z);

	/*
	- for pm_stat->patch_types scan through the
	pm_stat->pmod_info->pmod_hdr.patch_types_infile
	and load each patch_area with a matching patch_id
	*/

	pm_stat->voices_total = 0;

	j = pm_stat->pmod_info->mod_hdr.patch_types_infile;
	while (j--)
	{
		readrequest = sizeof(scratch_pat_grp_hdr);
		//printf("readrequest %d\n", readrequest);
		//module_read(&scratch_pat_grp_hdr, wmd_file_ptr, readrequest);
		module_memcpy(&scratch_pat_grp_hdr, wmd_file_ptr, readrequest);
		wmd_file_ptr += readrequest;

		//printf("load_flags %d\n", (unsigned int)scratch_pat_grp_hdr.load_flags);

		/*readresult = module_read(&scratch_pat_grp_hdr, readrequest, fp_wmd_file);
		if (readrequest != readresult)
		{
			err(wess_FREAD);
			free_mem_if_mine();
			return(0);
		}*/

		k = pm_stat->patch_types_loaded;
		while (k--)
		{
			if (scratch_pat_grp_hdr.patch_id == (pm_stat->ppat_info + k)->hw_tl_list.hardware_ID)
			{
				memcpy(&(pm_stat->ppat_info + k)->pat_grp_hdr,
					&scratch_pat_grp_hdr,
					sizeof(scratch_pat_grp_hdr));

				/*(pm_stat->ppat_info + k)->pat_grp_hdr.load_flags = scratch_pat_grp_hdr.load_flags;
				(pm_stat->ppat_info + k)->pat_grp_hdr.patch_id = scratch_pat_grp_hdr.patch_id;
				//(pm_stat->ppat_info + k)->pat_grp_hdr.hw_voice_limit = scratch_pat_grp_hdr.hw_voice_limit;//
				//(pm_stat->ppat_info + k)->pat_grp_hdr.pad1 = scratch_pat_grp_hdr.pad1;//
				(pm_stat->ppat_info + k)->pat_grp_hdr.patches = scratch_pat_grp_hdr.patches;
				//(pm_stat->ppat_info + k)->pat_grp_hdr.patch_size = scratch_pat_grp_hdr.patch_size;//
				(pm_stat->ppat_info + k)->pat_grp_hdr.patchmaps = scratch_pat_grp_hdr.patchmaps;
				//(pm_stat->ppat_info + k)->pat_grp_hdr.patchmap_size = scratch_pat_grp_hdr.patchmap_size;//
				(pm_stat->ppat_info + k)->pat_grp_hdr.patchinfo = scratch_pat_grp_hdr.patchinfo;
				//(pm_stat->ppat_info + k)->pat_grp_hdr.patchinfo_size = scratch_pat_grp_hdr.patchinfo_size;//
				(pm_stat->ppat_info + k)->pat_grp_hdr.drummaps = scratch_pat_grp_hdr.drummaps;
				//(pm_stat->ppat_info + k)->pat_grp_hdr.drummap_size = scratch_pat_grp_hdr.drummap_size;//
				(pm_stat->ppat_info + k)->pat_grp_hdr.extra_data_size = scratch_pat_grp_hdr.extra_data_size;*/

				pm_stat->voices_total += scratch_pat_grp_hdr.hw_voice_limit;

				(pm_stat->ppat_info + k)->ppat_data = pmem;

				//patfpos = module_tell(fp_wmd_file);

				//?? Checkear
				patfpos = (unsigned long)((unsigned long)wmd_file_ptr - (unsigned long)wmd_file_ptr2);
				//printf("patfpos %d\n", patfpos);


				(pm_stat->ppat_info + k)->data_fileposition = patfpos;

				if (!conditional_read(scratch_pat_grp_hdr.load_flags & LOAD_PATCHES,
					&pmem,
					(int)scratch_pat_grp_hdr.patches *
					(int)scratch_pat_grp_hdr.patch_size))
				{
					return(0);
				}

				if (!conditional_read(scratch_pat_grp_hdr.load_flags & LOAD_PATCHMAPS,
					&pmem,
					(int)scratch_pat_grp_hdr.patchmaps *
					(int)scratch_pat_grp_hdr.patchmap_size))
				{
					return(0);
				}

				if (!conditional_read(scratch_pat_grp_hdr.load_flags & LOAD_PATCHINFO,
					&pmem,
					(int)scratch_pat_grp_hdr.patchinfo *
					(int)scratch_pat_grp_hdr.patchinfo_size))
				{
					return(0);
				}

				if (!conditional_read(scratch_pat_grp_hdr.load_flags & LOAD_DRUMMAPS,
					&pmem,
					(int)scratch_pat_grp_hdr.drummaps *
					(int)scratch_pat_grp_hdr.drummap_size))
				{
					return(0);
				}

				if (!conditional_read(scratch_pat_grp_hdr.load_flags & LOAD_EXTRADATA,
					&pmem,
					(int)scratch_pat_grp_hdr.extra_data_size))
				{
					return(0);
				}

				break;
			}
		}
	}

	/*
	- allocate and initialize space for
	voice_total voice_status structures
	and update the pmvoicestattbl pointer and zero voices_active.
	*/

	pm_stat->pvoicestattbl = (voice_status *)pmem;
	pmem += sizeof(*pm_stat->pvoicestattbl) * pm_stat->voices_total;

	/*
	- initialize patch_type parameter for each voice work area.
	only the amount of hardware voices possible for each
	patch_type loaded will have voice work areas!!!
	you will run out of voice work areas for a given patch type
	at the same time you have run out of hardware polyphony!!!
	eh,eh,this is cool!,eh,eh
	*/

	if (pm_stat->patch_types_loaded)
	{
		types = pm_stat->patch_types_loaded;
		n = 0;
		num = (pm_stat->ppat_info + n)->pat_grp_hdr.hw_voice_limit;
		indx = 0;

		for (i = 0; i<pm_stat->voices_total; i++)
		{
			if (types)
			{
				if (num--)
				{
					(pm_stat->pvoicestattbl + i)->patchtype =
						(pm_stat->ppat_info + n)->pat_grp_hdr.patch_id;
					(pm_stat->pvoicestattbl + i)->refindx = indx++;//ó 0
				}
				else {
					n++;
					if (--types)
					{
						num = (pm_stat->ppat_info + n)->pat_grp_hdr.hw_voice_limit;
						indx = 0;
						if (num--)
						{
							(pm_stat->pvoicestattbl + i)->patchtype =
								(pm_stat->ppat_info + n)->pat_grp_hdr.patch_id;
							(pm_stat->pvoicestattbl + i)->refindx = indx++;
						}
					}
				}
			}
		}
	}

	/*
	- allocate and initialize space for
	pmod_info->mod_hdr.sequences sequence_data structures
	and update the pmod_info->pseq_info pointer.
	*/

	pm_stat->pmod_info->pseq_info = (sequence_data *)pmem;
	pmem += sizeof(*pm_stat->pmod_info->pseq_info) *
		pm_stat->pmod_info->mod_hdr.sequences;

	/*
	- for mod_hdr.sequences fill the sequence_data structures :
	read in pseq_info->seq_hdr,
	save fp_wmd_file pointer and
	scan throuch pseq_info->seq_hdr.tracks of track_headers
	seeking over the data to get to each header and
	increment track_toload for each voice_type matching any
	members of the pm_stat->ppat_id_list
	update pseq_info->ptrk_info pointer and
	allocate the resultant track_toload track_data structures
	seek to the saved fp_wmd_file pointer and read in each
	track_hdr with a voice_type matching the pm_stat->ppat_id_list
	and allocate and read in the corresponding data section :
	point pseq_info->ptrk_info->ptrk_data at this area,
	read in pseq_info->ptrk_info->data_size to this pointer.
	seek over non matching tracks.
	assign pseq_info->tracks = tracks_toload,
	for pseq_info->tracks fill the track_data structures that are
	of correct voices_type :
	read in sizeof(pseq_info->ptrk_info->trk_hdr),
	if it is in the tracks_toload the
	allocate pseq_info->ptrk_info->trk_hdr.voices_max chars,
	point pseq_info->ptrk_info->pvoices_indx at this area,
	allocate pseq_info->ptrk_info->trk_hdr.substack_size pointers,
	point pseq_info->ptrk_info->psubstack at this area,
	allocate pseq_info->ptrk_info->trk_hdr.labellist_size pointers,
	point pseq_info->ptrk_info->plabellist at this area,
	allocate pseq_info->ptrk_info->trk_hdr.data_size chars,
	update pm_stat->pmod_info->pseq_info->seq_hdr.tracks for
	each sequence with the number of tracks actually loaded.
	*/

	for (i = 0; i<pm_stat->pmod_info->mod_hdr.sequences; i++)
	{
		patfpos = (unsigned long)((unsigned long)wmd_file_ptr - (unsigned long)wmd_file_ptr2);
        //printf("--------\n");
        //printf("num %d\n",i);
        //printf("patfpos %d\n", patfpos);
		readrequest = sizeof(pm_stat->pmod_info->pseq_info->seq_hdr);
		//printf("readrequest %d\n", readrequest);

		//module_read(&(pm_stat->pmod_info->pseq_info + i)->seq_hdr, wmd_file_ptr, readrequest);
		module_memcpy(&(pm_stat->pmod_info->pseq_info + i)->seq_hdr, wmd_file_ptr, readrequest);
		wmd_file_ptr += readrequest;

		/*patfpos = module_tell(fp_wmd_file);

		readrequest = sizeof(pm_stat->pmod_info->pseq_info->seq_hdr);
		readresult = module_read(&(pm_stat->pmod_info->pseq_info + i)->seq_hdr, readrequest, fp_wmd_file);
		if (readrequest != readresult)
		{
			err(wess_FREAD);
			free_mem_if_mine();
			return(0);
		}*/


		tracks_toload = 0;
		trkinfosize = 0;


		j = (pm_stat->pmod_info->pseq_info + i)->seq_hdr.tracks;
		//printf("tracks %d\n", j);
		while (j--)
		{
			readrequest = sizeof(scratch_trk_hdr);
			//printf("readrequest %d\n", readrequest);
			//module_read(&scratch_trk_hdr, wmd_file_ptr, readrequest);
			module_memcpy(&scratch_trk_hdr, wmd_file_ptr, readrequest);

			/*printf("voices_type %d\n",scratch_trk_hdr.voices_type);
            printf("voices_max %d\n",scratch_trk_hdr.voices_max);
            printf("priority %d\n",scratch_trk_hdr.priority);
            printf("lockchannel %d\n",scratch_trk_hdr.lockchannel);
            printf("voices_class %d\n",scratch_trk_hdr.voices_class);
            printf("reverb %d\n",scratch_trk_hdr.reverb);
            printf("initpatchnum %d\n",scratch_trk_hdr.initpatchnum);
            printf("initpitch_cntrl %d\n",scratch_trk_hdr.initpitch_cntrl);
            printf("initvolume_cntrl %d\n",scratch_trk_hdr.initvolume_cntrl);
            printf("initpan_cntrl %d\n",scratch_trk_hdr.initpan_cntrl);
            printf("substack_count %d\n",scratch_trk_hdr.substack_count);
            printf("mutebits %d\n",scratch_trk_hdr.mutebits);
            printf("initppq %d\n",scratch_trk_hdr.initppq);
            printf("initqpm %d\n",scratch_trk_hdr.initqpm);
            printf("labellist_count %d\n",scratch_trk_hdr.labellist_count);
            printf("data_size %d\n",scratch_trk_hdr.data_size);*/

			wmd_file_ptr += readrequest;

			/*readresult = module_read(&scratch_trk_hdr, readrequest, fp_wmd_file);
			if (readrequest != readresult)
			{
				err(wess_FREAD);
				free_mem_if_mine();
				return(0);
			}*/

			loadit = 0;
			if ((scratch_trk_hdr.voices_type == NoSound_ID) ||
				(scratch_trk_hdr.voices_type == GENERIC_ID))
			{
				loadit = 1;
			}
			else {
				k = pm_stat->patch_types_loaded;
				while (k--)
				{
					/*if (((scratch_trk_hdr.voices_type == (pm_stat->ppat_info + k)->hw_tl_list.hardware_ID) &&
						(((scratch_trk_hdr.voices_class == SNDFX_CLASS) && (pm_stat->ppat_info + k)->hw_tl_list.sfxload) ||
						((scratch_trk_hdr.voices_class == SFXDRUMS_CLASS) && (pm_stat->ppat_info + k)->hw_tl_list.sfxload) ||
							((scratch_trk_hdr.voices_class == MUSIC_CLASS) && (pm_stat->ppat_info + k)->hw_tl_list.musload) ||
							((scratch_trk_hdr.voices_class == DRUMS_CLASS) && (pm_stat->ppat_info + k)->hw_tl_list.drmload))))*/
					if (((scratch_trk_hdr.voices_type == (pm_stat->ppat_info + k)->hw_tl_list.hardware_ID) &&
						(((scratch_trk_hdr.voices_class == SNDFX_CLASS) && ((pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_SOUND_EFFECTS)) ||
						((scratch_trk_hdr.voices_class == SFXDRUMS_CLASS) && ((pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_SOUND_EFFECTS)) ||
							((scratch_trk_hdr.voices_class == MUSIC_CLASS) && ((pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_MUSIC)) ||
							((scratch_trk_hdr.voices_class == DRUMS_CLASS) && ((pm_stat->ppat_info + k)->hw_tl_list.flags_load & TAG_DRUMS)))))
					{
						loadit = 1;
						break;
					}
				}
			}

			//printf("loadit %d\n", loadit);
			if (loadit)
			{

				trkinfosize += sizeof(*pm_stat->pmod_info->pseq_info->ptrk_info) +
					(scratch_trk_hdr.labellist_count * sizeof(long)) +
					scratch_trk_hdr.data_size;

                //printf("trkinfosize %d\n", trkinfosize);
#if _ALIGN4_ == 1
				//force align to word boundary because previous size adjust
				//may wind up with odd address
				trkinfosize += trkinfosize & 1;
				trkinfosize += trkinfosize & 2;
#endif

				if (scratch_trk_hdr.voices_max > max_voices_intrk)
				{
					max_voices_intrk = scratch_trk_hdr.voices_max;
				}

				if (scratch_trk_hdr.substack_count > max_substack_intrk)
				{
					max_substack_intrk = scratch_trk_hdr.substack_count;
				}

				tracks_toload++;
			}

			wmd_file_ptr += (scratch_trk_hdr.labellist_count * sizeof(long)) +
				scratch_trk_hdr.data_size;

			/*if (module_seek(fp_wmd_file,
				(scratch_trk_hdr.labellist_count * sizeof(long)) +
				scratch_trk_hdr.data_size,
				SEEK_CUR))
			{
				err(wess_FSEEK);
				free_mem_if_mine();
				return(0);
			}*/
		}

		if (tracks_toload>max_tracks_inseq)
		{
			max_tracks_inseq = tracks_toload;
		}

		if (tracks_toload == 0)
		{
			/* since there are no tracks to load we create area for the
			track info + 2 (0 delta and TrkEnd) */
			trkinfosize = sizeof(*pm_stat->pmod_info->pseq_info->ptrk_info) + 2;
#if _ALIGN4_ == 1
			//force align to word boundary because previous size adjust
			//may wind up with odd address
			trkinfosize += trkinfosize & 1;
			trkinfosize += trkinfosize & 2;
#endif

		}

		(pm_stat->pmod_info->pseq_info + i)->trkstoload = tracks_toload;
		(pm_stat->pmod_info->pseq_info + i)->trkinfolength = trkinfosize;
		(pm_stat->pmod_info->pseq_info + i)->fileposition = patfpos;
	}

	/* now we dont load the track data anymore */
	/* now we dont load the track data anymore */
	/* now we dont load the track data anymore */

	/*
	- allocate pm_stat->max_tracks_inseq chars for the ptrk_indxs
	pointers in each sequence_status structure.
	update each pointer to each area.
	initialize indexes to 0xFF.
	*/

	pm_stat->pcalltable = (callback_status *)pmem;
	pmem += sizeof(*pm_stat->pcalltable) *
		pm_stat->pmod_info->mod_hdr.callback_areas;

	pm_stat->max_trks_perseq = max_tracks_inseq;

	for (i = 0; i<pm_stat->pmod_info->mod_hdr.seq_work_areas; i++)
	{
		(pm_stat->pseqstattbl + i)->pgates = (char *)pmem;
		pmem += sizeof(char) *
			pm_stat->pmod_info->mod_hdr.gates_per_seq;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif


		(pm_stat->pseqstattbl + i)->piters = (char *)pmem;
		pmem += sizeof(char) *
			pm_stat->pmod_info->mod_hdr.iters_per_seq;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif


		j = max_tracks_inseq;//
		pdest = (pm_stat->pseqstattbl + i)->ptrk_indxs = (char *)pmem;
		pmem += sizeof(char) * j;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif

		while (j--)
		{
			*pdest++ = 0xFF;
		}
	}

	/*
	- allocate pm_stat->max_voices_intrk chars for the pvoice_indxs
	pointers in each track_status structure.
	update each pointer to each area.
	initialize indexes to 0xFF.
	*/

	pm_stat->max_voices_pertrk = max_voices_intrk;

	pm_stat->max_substack_pertrk = max_substack_intrk;

	for (i = 0; i<pm_stat->pmod_info->mod_hdr.trk_work_areas; i++)
	{
		(pm_stat->ptrkstattbl + i)->refindx = i;
		(pm_stat->ptrkstattbl + i)->psubstack = (unsigned long *)pmem;
		/* (pm_stat->ptrkstattbl+i)->psp is set when sequence is triggered */
		pmem += sizeof(long) * pm_stat->max_substack_pertrk;
		(pm_stat->ptrkstattbl + i)->pstackend = (unsigned long *)pmem;
	}

	/* load the drivers here */
	/* load the drivers here */
	/* load the drivers for each patch type and update the CmdFuncArr */
	/* load the drivers here */
	/* load the drivers here */

	/* initialize the loaded drivers and SeqEngine */

	CmdFuncArr[NoSound_ID][DriverInit]((track_status *)pm_stat);

	for (i = 0; i<pm_stat->patch_types_loaded; i++)
	{
		/*if ((pm_stat->ppat_info + i)->hw_tl_list.sfxload ||
			(pm_stat->ppat_info + i)->hw_tl_list.musload ||
			(pm_stat->ppat_info + i)->hw_tl_list.drmload)*/
		if (((pm_stat->ppat_info + i)->hw_tl_list.flags_load & (TAG_SOUND_EFFECTS| TAG_MUSIC| TAG_DRUMS)) != 0)
		{
			CmdFuncArr[(pm_stat->ppat_info + i)->hw_tl_list.hardware_ID][DriverInit]((track_status *)pm_stat);
		}
	}

	//module_close(fp_wmd_file);

	max_seq_num = pm_stat->pmod_info->mod_hdr.sequences;

#if _ALIGN4_ == 1
	//force align to word boundary because previous pmem adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
#endif
	wmd_end = pmem;
	module_loaded = 1;
	SeqOn = 1;

	return (1);
#endif
}

void filltrackstat(track_status *ptk_stat, track_data *ptk_info, TriggerPlayAttr *attr)//80042E34
{
#ifndef BLOCK_SOUND_WESS

    int tempmask;

    //printf("---------------------------------------\n");
	ptk_stat->flags = (ptk_stat->flags | TRK_ACTIVE) & ~(TRK_TIMED | TRK_LOOPED | TRK_SKIP | TRK_OFF);
	ptk_stat->patchtype = ptk_info->trk_hdr.voices_type;
	ptk_stat->priority = ptk_info->trk_hdr.priority;
	ptk_stat->voices_active = 0;
	ptk_stat->sndclass = ptk_info->trk_hdr.voices_class;
	ptk_stat->voices_max = ptk_info->trk_hdr.voices_max;
	ptk_stat->starppi = 0;
	ptk_stat->accppi = 0;
	ptk_stat->totppi = 0;
	ptk_stat->psp = ptk_stat->psubstack;
	ptk_stat->ppq = ptk_info->trk_hdr.initppq;
	ptk_stat->labellist_count = ptk_info->trk_hdr.labellist_count;
	ptk_stat->data_size = ptk_info->trk_hdr.data_size;
	ptk_stat->mutemask = ptk_info->trk_hdr.mutebits;

	if ((attr == NULL) || (!attr->mask))
	{
		tempmask = 0;
	}
	else {
		tempmask = attr->mask;
	}

	if(tempmask & TRIGGER_VOLUME) //0x01
    {
        ptk_stat->volume_cntrl = attr->volume;
        //printf("T_volume_cntrl %d\n", ptk_stat->volume_cntrl);
    } else {
        ptk_stat->volume_cntrl = ptk_info->trk_hdr.initvolume_cntrl;
        //printf("volume_cntrl %d\n", ptk_stat->volume_cntrl);
    }

	if (tempmask & TRIGGER_PAN) //0x02
	{
		ptk_stat->pan_cntrl = attr->pan;
		//printf("T_pan_cntrl %d\n", ptk_stat->pan_cntrl);
	}
	else {
		ptk_stat->pan_cntrl = ptk_info->trk_hdr.initpan_cntrl;
		//printf("pan_cntrl %d\n", ptk_stat->pan_cntrl);
	}

	if (tempmask & TRIGGER_PATCH) //0x04
	{
		ptk_stat->patchnum = attr->patch;
		//printf("T_patchnum %d\n", ptk_stat->patchnum);
	}
	else {
		ptk_stat->patchnum = ptk_info->trk_hdr.initpatchnum;
		//printf("patchnum %d\n", ptk_stat->patchnum);
	}

	if (tempmask & TRIGGER_PITCH) //0x08
	{
		ptk_stat->pitch_cntrl = attr->pitch;
		//printf("T_pitch_cntrl %d\n", ptk_stat->pitch_cntrl);
	}
	else {
		ptk_stat->pitch_cntrl = ptk_info->trk_hdr.initpitch_cntrl;
		//printf("pitch_cntrl %d\n", ptk_stat->pitch_cntrl);
	}

	if (tempmask & TRIGGER_MUTEMODE) //0x10
	{
		if (ptk_stat->mutemask & (1 << attr->mutemode))
		{
			//ptk_stat->mute = 1;
			ptk_stat->flags |= TRK_MUTE;
			//printf("T_mute %d\n", ptk_stat->flags);
		}
		else {
			//ptk_stat->mute = 0;
			ptk_stat->flags &= ~TRK_MUTE;
			//printf("T_mute %d\n", ptk_stat->flags);
		}
	}
	else {
		//ptk_stat->mute = 0;
		ptk_stat->flags &= ~TRK_MUTE;
		//printf("mute %d\n", ptk_stat->flags);
	}

	if (tempmask & TRIGGER_TEMPO) //0x20
	{
		ptk_stat->qpm = attr->tempo;
		//printf("T_qpm %d\n", ptk_stat->qpm);
	}
	else {
		ptk_stat->qpm = ptk_info->trk_hdr.initqpm;
		//printf("qpm %d\n", ptk_stat->qpm);
	}

	ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(),
									ptk_stat->ppq,
									ptk_stat->qpm);

	//printf("ppi %d\n", ptk_stat->ppi);

	if (tempmask & TRIGGER_TIMED) //0x40
	{
		ptk_stat->endppi = ptk_stat->totppi + attr->timeppq;
		//ptk_stat->timed = 1;
		ptk_stat->flags |= TRK_TIMED;
		//printf("T_endppi %d\n", ptk_stat->endppi);
		//printf("timed %d\n", ptk_stat->flags);
	}
	else {
		//ptk_stat->timed = 0;
		ptk_stat->flags &= ~TRK_TIMED;
		//printf("timed %d\n", ptk_stat->flags);
	}

	if (tempmask&TRIGGER_LOOPED) //0x80
	{
		//ptk_stat->looped = 1;
		ptk_stat->flags |= TRK_LOOPED;
		//printf("T_looped %d\n", ptk_stat->flags);
	}
	else {
		//ptk_stat->looped = 0;
		ptk_stat->flags &= ~TRK_LOOPED;
		//printf("looped %d\n", ptk_stat->flags);
	}

	if (tempmask & TRIGGER_REVERB) //0x100
	{
		ptk_stat->reverb = attr->reverb;
		//printf("T_reverb %d\n", ptk_stat->reverb);
	}
	else {
		ptk_stat->reverb = ptk_info->trk_hdr.reverb;
		//printf("reverb %d\n", ptk_stat->reverb);
	}
#endif
}

void assigntrackstat(track_status *ptk_stat, track_data *ptk_info)//800430C0
{
#ifndef BLOCK_SOUND_WESS

    ptk_stat->data_space = ptk_info->trk_hdr.data_size;
    ptk_stat->labellist_max = ptk_info->trk_hdr.labellist_count;
    ptk_stat->pstart = ptk_info->ptrk_data;
    ptk_stat->ppos = Read_Vlq(ptk_stat->pstart,&ptk_stat->deltatime);
    ptk_stat->plabellist = ptk_info->plabellist;

    //printf("data_space %d\n", ptk_stat->data_space);
    //printf("labellist_max %d\n", ptk_stat->labellist_max);
    //printf("ppos %d\n", *ptk_stat->ppos);
    //printf("plabellist %d\n", *ptk_stat->plabellist);
#endif
}

/*------------------------------------------------------------------*/
/*
    Sequencer calls.
*/
/*------------------------------------------------------------------*/

int wess_seq_structrig (sequence_data *psq_info,
                        int seq_num,
                        int seq_type,
                        enum HandleFlag gethandle,
                        TriggerPlayAttr *attr)//80043124
{
#ifndef BLOCK_SOUND_WESS

    char i, j, limit;
	short n;
	char tracksfilled;
	char *pdest;
	sequence_status *psq_stat;
	track_data *ptk_info;
	track_status *ptk_stat;

	if (!Is_Seq_Num_Valid(seq_num))
	{
		return (0);
	}

	/* runs trigger function and update status structures for api */

	/*
	- save the sequencer information block pointer,
	if a sequence status structure is free:
	mark sequence as active,
	flag to start what tracks can be started,
	for each track started update psq_info->ptrk_indxs
	*/

	//    _disable();
	SeqOn = 0;


	/* find an open sequence structure */

	limit = pm_stat->pmod_info->mod_hdr.seq_work_areas;

	for (i = 0; i<limit; i++)
	{
		if (!((pm_stat->pseqstattbl + i)->flags & SEQ_ACTIVE))
		{
			break;
		}
	}
	if (i == limit)
	{
		SeqOn = 1;
		return (0); /* no sequence work area available */
	}
	else
    {
		tracksfilled = 0; /* used to check if any tracks are started */
		/*
		we found a sequence structure so fill it.
		*/

		psq_stat = pm_stat->pseqstattbl + i;          /* pointer math */

		/*
		for n tracks in the sequence find and open track structure
		and initialize it.
		*/
		n = psq_info->seq_hdr.tracks;
		limit = pm_stat->pmod_info->mod_hdr.trk_work_areas;
		pdest = psq_stat->ptrk_indxs;
		for (j = 0; j<limit; j++)
		{
			if (!((pm_stat->ptrkstattbl + j)->flags & TRK_ACTIVE))
			{
				ptk_stat = pm_stat->ptrkstattbl + j;  /* pointer math */
				ptk_info = psq_info->ptrk_info + tracksfilled;
				/* refindx was filled at init time */
				ptk_stat->seq_owner = i;

				filltrackstat(ptk_stat, ptk_info, attr);
				assigntrackstat(ptk_stat, ptk_info);

				if (gethandle)
				{
					ptk_stat->flags |= (TRK_HANDLED | TRK_STOPPED);

				}
				else {
					ptk_stat->flags &= ~(TRK_HANDLED | TRK_STOPPED);
					psq_stat->tracks_playing++;
				}
				//ptk_stat->flags |= TRK_ACTIVE;

				psq_stat->tracks_active++;
				pm_stat->trks_active++;
				*pdest++ = j; /* update ptrk_indxs for the sequence */
				tracksfilled++;

				if (!--n) break;
			}
		}

		/* if tracks were started, activate the sequence */
		if (tracksfilled)
		{
			psq_stat->seq_num = seq_num;
			psq_stat->seq_type = seq_type;
			if (gethandle)
			{
				psq_stat->flags = SEQ_HANDLE;
				psq_stat->playmode = SEQ_STATE_STOPPED;
			}
			else {
				psq_stat->flags &= ~SEQ_HANDLE;
				psq_stat->playmode = SEQ_STATE_PLAYING;
			}
			psq_stat->volume = 128;
			psq_stat->pan = 64;
			psq_stat->flags |= SEQ_ACTIVE;
			pm_stat->seqs_active++;
		}
		SeqOn = 1;

		if (tracksfilled)
		{
			return (i + 1);
		}
		else {
			return (0);
		}
	}
#endif
}



void wess_seq_trigger (int seq_num)//8004341C
{
#ifndef BLOCK_SOUND_WESS

    wess_seq_trigger_type (seq_num, 0);
#endif
}

void wess_seq_trigger_special (int              seq_num,
                               TriggerPlayAttr *attr)//8004343C
{
#ifndef BLOCK_SOUND_WESS

    wess_seq_trigger_type_special (seq_num, 0, attr);
#endif
}

int wess_seq_status (int sequence_number)//8004348C
{
#ifndef BLOCK_SOUND_WESS

    /* immediate stop of sequence */
	char nt;
	sequence_status *psq_stat;
	int status;

	if (!Is_Seq_Num_Valid(sequence_number))
	{
		return(SEQUENCE_INVALID);
	}

	status = SEQUENCE_INACTIVE;

	/* search for all sequences with this number and turn them off */
	nt = pm_stat->pmod_info->mod_hdr.seq_work_areas;
	psq_stat = pm_stat->pseqstattbl;
	while (nt--)
	{
		//if (psq_stat->active)
		if (psq_stat->flags & SEQ_ACTIVE)
		{
			if (psq_stat->seq_num == sequence_number)
			{
				if (psq_stat->playmode == SEQ_STATE_STOPPED)
				{
					status = SEQUENCE_STOPPED;
				}
				else if (psq_stat->playmode == SEQ_STATE_PLAYING) {
					status = SEQUENCE_PLAYING;
				}
				break; /* stop looking if we found one */
			}
		}
		psq_stat++;
	}

	return(status);
#endif
}

void wess_seq_stop (int sequence_number)//80043560
{
#ifndef BLOCK_SOUND_WESS

    /* immediate stop of sequence */
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	if (!Is_Seq_Num_Valid(sequence_number))
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
			//if (psq_stat->active)
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
							CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
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

void wess_seq_stopall (void)//800436EC
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

	/* immediate stop of all sequences */
	SeqOn = 0;

	/* search for all sequences and turn them off */
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
						CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
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
