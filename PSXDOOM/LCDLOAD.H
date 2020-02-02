    /*------------------------------------------------------------------*/
    /*
                      Williams Entertainment Sound System
                             by Scott Patterson

                   Application Programming Interface Routines:

                   - digital (in linear .LCD file) load functions
    */
    /*------------------------------------------------------------------*/

#define SAMPLE_RECORD_SIZE 100

typedef struct SampleBlock {
    unsigned short numsamps;//r*
    unsigned short sampindx[SAMPLE_RECORD_SIZE];//r*2
    unsigned short samppos[SAMPLE_RECORD_SIZE];//r*4
}SampleBlock;

extern int wess_dig_lcd_loader_init(void *input_pm_stat);

extern void wess_dig_set_sample_position(int samplenum, char *samplepos);

extern int wess_dig_lcd_load(char *lcdfilename,void *memptr,SampleBlock *sampblk, int override);


