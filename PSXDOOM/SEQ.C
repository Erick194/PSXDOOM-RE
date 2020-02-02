#include "kernel.h"
#include <libspu.h>

extern volatile int val;
extern volatile int num;

void SeqEngine(void)
{
     //printf("SeqEngine %d\n", val);
     val += 2;

    printf("SeqEngine  num ->%d\n", num);
}
