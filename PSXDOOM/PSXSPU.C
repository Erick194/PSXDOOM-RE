// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include <libspu.h>
#include "psxspu.h"
#include "wessapi.h"

extern void SpuSetCommonAttr (SpuCommonAttr *attr);

#define MALLOC_MAX 1
extern char spu_malloc_rec[SPU_MALLOC_RECSIZ * (MALLOC_MAX + 1)];

extern unsigned int psxspu_initialized;
extern unsigned int psxspu_status;
extern unsigned long end_of_sram;

extern int psxspu_master_vol;
extern int psxspu_master_fadecount;
extern int psxspu_master_vol_fixed;
extern int psxspu_master_destvol;
extern int psxspu_master_changeval;

extern int psxspu_cd_vol;
extern int psxspu_cd_fadecount;
extern int psxspu_cd_vol_fixed;
extern int psxspu_cd_destvol;
extern int psxspu_cd_changeval;

extern SpuReverbAttr rev_attr;//8007EEA4

unsigned long SpuGetReverbOffsetAddr[10] = // equivale a la función SpuGetReverbOffsetAddr()
{
    520192, 514368, 516288, 505792, 495648,
    479776, 461120, 425920, 425920, 508928
};

#define END_O_SRAM 520192
void psxspu_init_reverb(long  rev_mode, short depthleft, short depthright, long  delay, long  feedback)//80045098
{
    psxspu_status = 0;

    rev_attr.mask = (SPU_REV_MODE|SPU_REV_DEPTHL|SPU_REV_DEPTHR|SPU_REV_DELAYTIME|SPU_REV_FEEDBACK);
    rev_attr.mode = rev_mode | SPU_REV_MODE_CLEAR_WA;
    rev_attr.depth.left = depthleft;
    rev_attr.depth.right = depthright;
    rev_attr.delay = delay;
    rev_attr.feedback = feedback;

    SpuSetReverbModeParam (&rev_attr);
    SpuSetReverbDepth (&rev_attr);

    if(rev_mode == PSXSPU_REV_MODE_OFF)
    {
        SpuSetReverb(SPU_OFF);
        end_of_sram = END_O_SRAM;
    }
    else
    {
        SpuSetReverb(SPU_ON);
        end_of_sram = SpuGetReverbOffsetAddr[rev_mode];//SpuGetReverbOffsetAddr();
    }

    SpuSetReverbVoice((rev_mode != PSXSPU_REV_MODE_OFF), SPU_ALLCH);
    psxspu_status = 1;

    //printf("End_Of_Sram %d\n", end_of_sram);
}

void psxspu_set_reverb_depth(short depthleft,short depthright)//80045178
{
    psxspu_status = 0;
    rev_attr.depth.left = depthleft;
	rev_attr.depth.right = depthright;
	SpuSetReverbDepth(&rev_attr);
	psxspu_status = 1;

}

void psxspu_init(void)//800451c0
{
    SpuCommonAttr com_attr;

    if (psxspu_initialized == 0)
	{
	    //printf("psxspu_init\n");
		psxspu_status = 0;
		psxspu_initialized = 1;

		SpuInit();
		SpuInitMalloc(MALLOC_MAX, spu_malloc_rec);
		SpuSetTransMode(SpuTransferByDMA);

		psxspu_init_reverb(0, 0, 0, 0, 0);

		com_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDVOLR| SPU_COMMON_CDREV | SPU_COMMON_CDMIX);
		com_attr.mvol.left = MAX_MASTER_VOL;
		com_attr.mvol.right = MAX_MASTER_VOL;
		com_attr.cd.volume.left = MAX_CD_VOL;
		com_attr.cd.volume.right = MAX_CD_VOL;
		com_attr.cd.reverb = 0;
		com_attr.cd.mix = 1;
		SpuSetCommonAttr(&com_attr);
		psxspu_status = 1;
	}
}

void psxspu_set_master_volume(int vol)//8004526C
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
    com_attr.mvol.left = vol;
	com_attr.mvol.right = vol;
	com_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_set_cd_volume(int vol)//800452B0
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
    com_attr.cd.volume.left = vol;
	com_attr.cd.volume.right = vol;
	com_attr.mask = (SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR);
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_setcdmixon(void)//800452F4
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
	com_attr.mask = (SPU_COMMON_CDMIX);
	com_attr.cd.mix = 1;
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_setcdmixoff(void)//8004533C
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
	com_attr.mask = (SPU_COMMON_CDMIX);
	com_attr.cd.mix = 0;
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_fadeengine(void)//8004537C
{
    if (psxspu_status != 0)
	{
		if (psxspu_cd_fadecount > 0)
		{
			psxspu_cd_fadecount -= 1;
			psxspu_cd_vol_fixed += psxspu_cd_changeval;

			if (psxspu_cd_fadecount == 0)
				psxspu_cd_vol_fixed = psxspu_cd_destvol;

			psxspu_cd_vol = psxspu_cd_vol_fixed >> 16;

			psxspu_set_cd_volume(psxspu_cd_vol);
		}

		if (psxspu_master_fadecount > 0)
		{
			psxspu_master_fadecount -= 1;
			psxspu_master_vol_fixed += psxspu_master_changeval;

			if (psxspu_master_fadecount == 0)
				psxspu_master_vol_fixed = psxspu_master_destvol;

			psxspu_master_vol = psxspu_master_vol_fixed >> 16;
			psxspu_set_master_volume(psxspu_master_vol);
		}
	}
}

void psxspu_set_cd_vol(int vol)//80045490
{
    psxspu_status = 0;
    psxspu_cd_vol = vol;
	psxspu_cd_vol_fixed = psxspu_cd_vol << 16;
	psxspu_cd_fadecount = 0;
	psxspu_set_cd_volume(psxspu_cd_vol);
	psxspu_status = 1;
}

int  psxspu_get_cd_vol(void)//800454EC
{
    return psxspu_cd_vol;
}

void psxspu_start_cd_fade(int msec, int destvol)//800454FC
{
    psxspu_status = 0;
    if (WessTimerActive == 0)
    {
        psxspu_cd_fadecount = 0;
    }
    else
    {
        psxspu_cd_destvol = destvol * 0x10000;
        psxspu_cd_fadecount = (msec * 0x78) / 1000 + 1;
        psxspu_cd_changeval = (psxspu_cd_destvol - psxspu_cd_vol_fixed) / psxspu_cd_fadecount;
    }
    psxspu_status = 1;
}

void psxspu_stop_cd_fade(void)//800455B4
{
    psxspu_status = 0;
	psxspu_cd_fadecount = 0;
	psxspu_status = 1;
}

int  psxspu_get_cd_fade_status(void)//800455D8
{
    return (psxspu_cd_fadecount < 2) ^ 1;
}

void psxspu_set_master_vol(int vol)//800455F0
{
    psxspu_status = 0;
    psxspu_master_vol = vol;
	psxspu_master_vol_fixed = psxspu_master_vol << 16;
	psxspu_master_fadecount = 0;
	psxspu_set_master_vol(psxspu_master_vol);
	psxspu_status = 1;
}

int  psxspu_get_master_vol(void)//8004564C
{
    return psxspu_master_vol;
}

void psxspu_start_master_fade(int msec, int destvol)//8004565C
{
    psxspu_status = 0;
	if (WessTimerActive == 0)
    {
        psxspu_master_fadecount = 0;
    }
    else
    {
        psxspu_master_destvol = destvol * 0x10000;
        psxspu_master_fadecount = (msec * 0x78) / 1000 + 1;
        psxspu_master_changeval = (psxspu_master_destvol - psxspu_master_vol_fixed) / psxspu_master_fadecount;
    }
    psxspu_status = 1;
}

void psxspu_stop_master_fade(void)//80045714
{
    psxspu_status = 0;
	psxspu_master_fadecount = 0;
	psxspu_status = 1;
}

int  psxspu_get_master_fade_status(void)//80045738
{
    return (psxspu_master_fadecount < 2) ^ 1;
}
