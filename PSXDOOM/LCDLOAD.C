// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update 20/10/2019 [GEC]

#include <libspu.h>
#include "psxcd.h"
#include "psxspu.h"
#include "wessapi.h"
#include "lcdload.h"
#include "doomdata.h"

extern int lcd_open;	//L800758F4
extern char *lcd_ppat_data_patches;//L800758F8
extern char *lcd_ppat_data_patchmaps;//L800758FC
extern char *lcd_ppat_data_patchinfo;//L80075900
extern char *lcd_ppat_data_track;//L80075904
extern patch_group_data *lcd_ppat_info;//L80075908;
extern unsigned long lcd_sample_pos;//L8007590C
extern unsigned int lcd_sectorbuf_pos;//L80075910
extern unsigned int lcd_totaltogo;//L80075914

extern short lcd_sectorbuf_code ; //L80075918
extern char *lcd_sectorbuf;//L8007591C

extern int lcd_cd_status; //L80075920

extern int  lcd_sync_intr;//L80075924
extern char lcd_sync_result[8];//0x80075928

extern Wess_Data_IO_Struct lcd_data_fileref;//0x8007f104

#define CHUNKSIZE 2048
extern char chunk[CHUNKSIZE];//80096394
extern char chunk2[CHUNKSIZE];//80096BA4

int wess_dig_lcd_loader_init(void *input_pm_stat)//80048C54
{
    pmasterstat *pm_stat;
    patch_group_data *pat_info;
	char *ppat_data;
	int k;

	pm_stat = input_pm_stat;

	lcd_open = 0;
	if (!pm_stat)
		return 0;

	for (k = 0; k < pm_stat->pmod_info->mod_hdr.patch_types_infile; k++)
	{
		pat_info = (pm_stat->ppat_info + k);

		if (pat_info->pat_grp_hdr.patch_id == SNDHW_TAG_DRIVER_ID)
			break;
	}

	lcd_ppat_info = pat_info;

	if (lcd_ppat_info != 0)
	{
		ppat_data = lcd_ppat_info->ppat_data;
		lcd_open = 1;

		lcd_ppat_data_patches = ppat_data;

		//printf("patches %d\n",lcd_ppat_info->pat_grp_hdr.patches);
		ppat_data += (lcd_ppat_info->pat_grp_hdr.patches * sizeof(patches_header));
		lcd_ppat_data_patchmaps = ppat_data;

		//printf("patchmaps %d\n",lcd_ppat_info->pat_grp_hdr.patchmaps);
		ppat_data += (lcd_ppat_info->pat_grp_hdr.patchmaps * sizeof(patchmaps_header));
		lcd_ppat_data_patchinfo = ppat_data;

		//printf("patchinfo %d\n",lcd_ppat_info->pat_grp_hdr.patchinfo);
		ppat_data += (lcd_ppat_info->pat_grp_hdr.patchinfo * sizeof(patchinfo_header));
		lcd_ppat_data_track = ppat_data;
		return 1;
	}

	return 0;
}

void wess_dig_set_sample_position(int samplenum, char *samplepos)//80048D3C
{
    patchinfo_header *patchinfo;

	if (lcd_ppat_data_patchinfo)
	{
		patchinfo = (patchinfo_header *) (lcd_ppat_data_patchinfo + (samplenum * sizeof(patchinfo_header)));
		patchinfo->sample_pos = (unsigned long)samplepos;
	}
}

Wess_Data_IO_Struct *wess_dig_lcd_data_open(char *filename)//80048D68
{
    Wess_File_IO_Struct *fp;

	fp = psxcd_open(filename);

	lcd_data_fileref = *fp;

	return(&lcd_data_fileref);
}

int wess_dig_lcd_data_read(unsigned char *buffer,void *destptr, SampleBlock *sampblk, int override)//80048DDC
{
    patchinfo_header *patchinfo;
	unsigned int samplenum, totaltogo, readbytes, totaltogo_tmp;
	unsigned long spuptr;

	spuptr = 0;
	readbytes = 0;
	totaltogo = CHUNKSIZE;

    while(totaltogo != 0)
    {
        //InitNextSampleBlock
        if (lcd_totaltogo == 0)
        {
            if (lcd_sectorbuf_pos > 0)
            {
                samplenum = (short) *(lcd_sectorbuf + (lcd_sectorbuf_pos << 1));
                patchinfo = (patchinfo_header *)(lcd_ppat_data_patchinfo + (samplenum * sizeof(patchinfo_header)));

                if (!(patchinfo->sample_pos) || (override != 0))
                {
                    patchinfo->sample_pos = lcd_sample_pos;
                    if (sampblk != 0)
                    {
                        sampblk->sampindx[sampblk->numsamps] = samplenum;
                        sampblk->samppos[sampblk->numsamps] = patchinfo->sample_pos >> 3;
                        sampblk->numsamps += 1;
                        //printf("save to sample [%d] [%d]\n", patchinfo->sample_pos, patchinfo->sample_pos >> 3);
                    }
                }
            }

            if (lcd_sectorbuf_code <= lcd_sectorbuf_pos)
            {
                return readbytes;
            }

            lcd_sectorbuf_pos++;
            samplenum = (short) *(lcd_sectorbuf + (lcd_sectorbuf_pos << 1));
            patchinfo = (patchinfo_header *)(lcd_ppat_data_patchinfo + (samplenum * sizeof(patchinfo_header)));

            lcd_totaltogo = patchinfo->sample_size;
            lcd_sample_pos = ((unsigned long)destptr + spuptr);

            if (((end_of_sram - (unsigned long)destptr) + spuptr) < patchinfo->sample_size)
            {
                lcd_totaltogo = 0;
                return readbytes;
            }
            if (totaltogo == 0) {
                return readbytes;
            }
        }

        totaltogo_tmp = totaltogo;

        //Read Data
        if (lcd_totaltogo < totaltogo)
        {
            samplenum = (short) *(lcd_sectorbuf + (lcd_sectorbuf_pos << 1));
            patchinfo = (patchinfo_header *)(lcd_ppat_data_patchinfo + (samplenum * sizeof(patchinfo_header)));

            if (!(patchinfo->sample_pos) || (override != 0))
            {
                SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
                SpuSetTransferStartAddr(((unsigned long)destptr + spuptr));
                SpuWrite((buffer + spuptr), lcd_totaltogo);
                readbytes += lcd_totaltogo;
            }
            totaltogo = totaltogo_tmp - lcd_totaltogo;
            spuptr += lcd_totaltogo;
            lcd_totaltogo = 0;
        }
        else
        {
            samplenum = (short) *(lcd_sectorbuf + (lcd_sectorbuf_pos << 1));
            patchinfo = (patchinfo_header *)(lcd_ppat_data_patchinfo + (samplenum * sizeof(patchinfo_header)));

            if (!(patchinfo->sample_pos) || (override != 0))
            {
                SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
                SpuSetTransferStartAddr(((unsigned long)destptr + spuptr));
                SpuWrite(buffer+spuptr, totaltogo_tmp);
                readbytes += totaltogo_tmp;
            }

            spuptr += totaltogo_tmp;
            lcd_totaltogo -= totaltogo_tmp;
            totaltogo = 0;
        }
    }

	return (readbytes);
}

int wess_dig_lcd_psxcd_sync(void)//8004911C
{
    unsigned long block;

	block = millicount;
	block += 8000;

	while (millicount < block)
	{
		lcd_sync_intr = CdSync(1, (u_char *)&lcd_sync_result);

		if (lcd_sync_intr == CdlDiskError)
		{
			CdFlush();
			return 1;
		}

		if (lcd_sync_intr == CdlComplete) return 0;
	}

	return 1;
}

int wess_dig_lcd_load(char *lcdfilename, void *memptr, SampleBlock *sampblk, int override)//800491C4
{
	Wess_File_IO_Struct *fileptr;
	char *destptr;
	int countbytes, bytestocopy, readbytes;
	boolean read1, read2;

	psxcd_disable_callbacks();

ReadAgain:
	do
	{
		do
		{
			do
			{
				lcd_sectorbuf_pos = 0;
				lcd_totaltogo = 0;
				readbytes = 0;

				psxcd_init_pos();
				psxcd_set_data_mode();

				fileptr = wess_dig_lcd_data_open(lcdfilename);

				if (*(int *)&fileptr->file.pos == 0)
					return 0;

				bytestocopy = fileptr->file.size;

				CdIntToPos(CdPosToInt(&fileptr->file.pos), &fileptr->new_io_loc);
				CdControl(CdlSetloc, (u_char *)&fileptr->new_io_loc, 0);
				CdControl(CdlReadN, (u_char *)&fileptr->new_io_loc, 0);
				do
				{
					lcd_cd_status = CdReady(1, 0);
					if (lcd_cd_status == CdlDataReady) goto DataReady1;
				} while (lcd_cd_status != CdlDiskError);
				CdFlush();
			DataReady1:
			} while (lcd_cd_status == CdlDiskError);

			CdGetSector((u_long *)&sectorbuf, CHUNKSIZE / 4);
			lcd_sectorbuf_code = (short)*sectorbuf; // read lcd count indx
			lcd_sectorbuf = (char *)&sectorbuf;
		} while (100 < lcd_sectorbuf_code);

		if (bytestocopy < CHUNKSIZE)
			bytestocopy = 0;
		else
			bytestocopy += -CHUNKSIZE;

		read1 = true;
		read2 = true;
		destptr = memptr;
		lcd_sectorbuf_code = (short)*sectorbuf; // read lcd count indx
		while (bytestocopy != 0)
		{
			if (read1)
			{
				do
				{
					lcd_cd_status = CdReady(1, 0);
					if (lcd_cd_status == CdlDataReady) goto DataReady2;
				} while (lcd_cd_status != CdlDiskError);
				CdFlush();
			DataReady2:
				if (lcd_cd_status == CdlDiskError) goto ReadAgain;

				CdGetSector((u_long *)&chunk, CHUNKSIZE / 4);

				if (bytestocopy < CHUNKSIZE)
					bytestocopy = 0;
				else
					bytestocopy += -CHUNKSIZE;

				countbytes = wess_dig_lcd_data_read(chunk, destptr, sampblk, override);
				readbytes += countbytes;
				destptr += countbytes;
				read2 = true;
				read1 = false;
			}
			else
			{
				if (read2)
				{
					do
					{
						lcd_cd_status = CdReady(1, 0);
						if (lcd_cd_status == CdlDataReady) goto DataReady3;
					} while (lcd_cd_status != CdlDiskError);
					CdFlush();
				DataReady3:
					if (lcd_cd_status == CdlDiskError) goto ReadAgain;

					CdGetSector((u_long *)&chunk2, CHUNKSIZE / 4);

					if (bytestocopy < CHUNKSIZE)
						bytestocopy = 0;
					else
						bytestocopy += -CHUNKSIZE;

					SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
					countbytes = wess_dig_lcd_data_read(chunk2, destptr, sampblk, override);
					readbytes += countbytes;
					destptr += countbytes;
					read2 = false;
				}
				else
				{
					do
					{
						lcd_cd_status = CdReady(1, 0);
						if (lcd_cd_status == CdlDataReady) goto DataReady4;
					} while (lcd_cd_status != CdlDiskError);
					CdFlush();
				DataReady4:
					if (lcd_cd_status == CdlDiskError) goto ReadAgain;

					CdGetSector((u_long *)&chunk, CHUNKSIZE / 4);

					if (bytestocopy < CHUNKSIZE)
						bytestocopy = 0;
					else
						bytestocopy += -CHUNKSIZE;

					SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
					countbytes = wess_dig_lcd_data_read(chunk, destptr, sampblk, override);
					readbytes += countbytes;
					destptr += countbytes;
					read2 = true;
				}
			}
		}

		if (lcd_cd_status != CdlDiskError)
		{
			CdControl(CdlPause, 0, 0);
			if (wess_dig_lcd_psxcd_sync() == 0)
			{
				SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
				psxcd_enable_callbacks();
				return (readbytes);
			}
		}
	} while (true);
}
