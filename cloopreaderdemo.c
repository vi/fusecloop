#include "cloopreader.h"
/*
 * It was cloopreader.c in former life
 */

struct cloop_data cd;

int main(){  
    int fh;
    fh=OP(open("hw.cloop",O_RDONLY));
    cloop_init(&cd,fh);

    char buff[16];
    fprintf(stderr,"cloop_read returned %d\n",cloop_read(&cd,1024*1024*4-10,buff,16));
    OP(write(1,buff,16/*blocksize*/));

    return 0;
}
