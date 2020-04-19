// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update 20/10/2019 [GEC]

#include "seqload.h"
#define _ALIGN4_ 1

#include <stdio.h>

extern pmasterstat *pm_stat;

extern int seq_loader_enable;           //80075790
extern char *seqfilename;		        //80075794
extern pmasterstat *seq_pm_stat;        //80075798
extern int seq_maxsequences;            //8007579C
extern char Driver_Init;                //800757A0
extern char Reset_Gates;                //800757A1
extern int	seqopen;				    //800757A4
extern int(*Seq_Error_func)(int, int);  //800757A8
extern int Seq_Error_module;            //800757AC
extern Wess_File_IO_Struct *seqfileptr; //800757B0

extern track_header scratch_trk_hdr;    //8007EDE8
extern track_header seq_track_header;   //8007EE74
extern track_header base_track_header;  //8007EE8C


int wess_seq_range_sizeof(int seqfirst, int numseqs)//800497C0
{
    int count;

	count = 0;
	if (seq_loader_enable)
	{
		if (numseqs == 0)
			return 0;

		while (numseqs--)
		{
			count += wess_seq_sizeof(seqfirst);
			seqfirst += 1;
		}
	}
	return (count);
}

int wess_seq_range_load(int seqfirst, int numseqs, void *memptr)//8004984C
{
    int count;
    char *pmem;

    pmem = (char *)memptr;

	count = 0;
	if (seq_loader_enable)
	{
		if (!open_sequence_data() || !numseqs)
			return 0;

		while (numseqs--)
		{
		    //printf("----------------------\n");
			count += wess_seq_load(seqfirst, pmem+count);
			seqfirst += 1;
		}

		close_sequence_data();
	}

	return count;
}

int wess_seq_range_free(int seqfirst, int numseqs)//80049900
{
    if (seq_loader_enable)
	{
		if (numseqs == 0)
			return 0;

		while (numseqs--)
		{
			wess_seq_free(seqfirst);
			seqfirst += 1;
		}
		return 1;
	}
	return 0;
}
