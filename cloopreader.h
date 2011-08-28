#ifndef CLOOPREADER_H
#define CLOOPREADER_H

/* #define DEBUG */

#include "common_header.h"
#include <stdio.h>

/* OPA -- _O_peration with _p_error (with _a_dditional parameter) */
#define OPA(expr,test) ({\
    typeof(expr) ret=expr; \
    if(test){\
	fprintf(stderr,__FILE__":%d: run-time: \""#expr"\": %s\n",\
	    __LINE__,strerror(errno)); \
	bfuncinfo("run-time error in \"%s\": %s",#expr,strerror(errno)); \
    }\
    ret;  \
})

#define NEG ret==-1
#define ZER ret==0
#define NUL ret==NULL
#define OP(expr) OPA(expr, NEG)

#define btc __be64_to_cpu

#define ALLOC(var,size) var=(typeof(var))malloc(size)

#define ulong unsigned long

struct cloop_data{
    int fh;
    int numblocks;
    ulong blocksize;

    struct block_info *toc; /* Data index */
    size_t tocsize;

    unsigned char* cblock;  /* Compressed block */
    unsigned char* pblock;  /* Plain block */

    ulong cblocksize;     /* Size of compressed block */
    ulong cblocksizecur;  /* Size of allocated memory */

    int curblock; /* Number of current block (which is decompressed to pblock) */
};

/* Read all requested data */
int read_all(int fh, void* block, size_t size);

/* Initialize cloop reader */
int cloop_init(struct cloop_data *c, int fh);

/* Change cloop "page" */
int cloop_swap(struct cloop_data *c,ulong page);

/* Read data from cloop (may decrease buffer size)*/
int cloop_read(struct cloop_data* c, off_t offset,void* buf,ulong size);

/* Read all requested data from cloop */
int cloop_read_all(struct cloop_data* c, off_t offset,void* buf,ulong size);

#endif
