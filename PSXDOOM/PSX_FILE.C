/* PSX_FILE.c */

#include "doomdef.h"
#include "r_local.h"

/*
============================================================================

READ/OPEN/CLOSE/SEEK/CLEAR PSX FILE BASED ROUTINES

============================================================================
*/

PsxCd_File	files[4];		//800A9B74 psxdoom

/*
== == == == == == == == == ==
=
= ClearFiles
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

void ClearFiles(void)//L80031D84()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		files[i].file.size = 0;
	}
}

/*
== == == == == == == == == ==
=
= OpenFile
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

int OpenFile(char *filename)//L80031DAC()
{
	PsxCd_File *fp;
	int i, j;

	fp = psxcd_open(filename);

	if (!fp)
		I_Error("Cannot open %s", filename);

	//get free file
	for (i = 0; i < 4; i++)
	{
		if (files[i].file.size == 0)
			break;
	}

	if (!(i < 4))
		I_Error("OpenFile: Too many open files!");

	//copy file pointer
	files[i] = *fp;
	return i;
}

/*
== == == == == == == == == ==
=
= CloseFile
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

void CloseFile(int file_num)//L80031EA8()
{
	files[file_num].file.size = 0;
	psxcd_close(&files[file_num]);
}

/*
== == == == == == == == == ==
=
= SeekFile
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

int SeekFile(int file_num, int seekpos, int seekmode)//L80031EF4()
{
    PsxCd_File *fileptr;
    int tell;

    //printf("seekpos %d\n",seekpos);

    fileptr = &files[file_num];
	psxcd_seek(fileptr, seekpos, seekmode);
	tell = psxcd_tell(fileptr);
	//printf("tell %d\n",tell);
	return tell;
}

/*
== == == == == == == == == ==
=
= ReadFile
=
= Exclusive Psx Doom
== == == == == == == == == ==
*/

void ReadFile(int file_num, void *destptr, unsigned int readbytes)//L800322F8()
{
	unsigned long tell;
	unsigned int rbytes;
	PsxCd_File *fileptr;

	fileptr = &files[file_num];
	//printf("0x%X, [%d]", (unsigned long)fileptr,  file_num);

	tell = psxcd_tell(fileptr);

	rbytes = readbytes;
	if (0x2000 < readbytes)
		rbytes = 0x2000;	//max 32 kb

	psxcd_seek(fileptr, 0, PSXCD_SEEK_SET);
	psxcd_read(destptr, rbytes, fileptr);
	psxcd_seek(fileptr, tell, PSXCD_SEEK_SET);

	rbytes = psxcd_read(destptr, readbytes, fileptr);
	if (rbytes != readbytes)
		I_Error("ReadFile: error reading %d of %d bytes\n", rbytes, readbytes);
}
