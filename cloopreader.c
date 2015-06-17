/*
 * File:    cloopreader.c 
 * Purpose: cloop file reader for fusecloop.
 * Copyright (c)  2007 Vitaly  "_Vi" Shukela
 * Copyright 1999-2003 by Paul `Rusty' Russell & Klaus Knopper.
 *
 * Contact Email: public_vi@tut.by
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "debug.h"
#include "cloopreader.h"

static int read_all(int fh, void* block, size_t size){
    bfuncinfo("fh=%d block=0x%lx size=0x%lx",
        fh,(ulong)block,(ulong)size);
    char* bl=(char*)block;
    int ret;
    for(;size;){
        for(;;){
            bprintf("invoking read(%d,0x%lx,0x%lx)\n",
                fh,(ulong)bl,(ulong)size);
            ret=read(fh,bl,size);
            bprintf("    returned %d, errno=%d\n",ret,errno);
            if(ret==-1&&errno==EINTR)continue;
            if(ret==-1||ret==0)return -1;
            break;
        }
        size-=ret;
        bl+=ret;
    }
    return 0;
}

int cloop_init(struct cloop_data *c, int fh){
    bfuncinfo("fh=%d",fh);
    c->fh=fh;
    struct cloop_head head;
    OP(read_all(c->fh,&head,sizeof head));   /* read Header */

    c->numblocks=ntohl(head.num_blocks);
    c->blocksize=ntohl(head.block_size);

    bprintf("block_size=%lx  num_blocks=%x\n", c->blocksize, c->numblocks);

    ALLOC(c->pblock,c->blocksize);

    c->tocsize=sizeof(*c->toc) * c->numblocks;
    if (c->numblocks == -1) {
	struct cloop_tail tail;
	loff_t end = lseek(c->fh,0,SEEK_END); /* lseek(,-n,SEEK_END) buggy ? */

	OP(lseek(c->fh, end - sizeof(tail), SEEK_SET)); 
	OP(read_all(c->fh, &tail, sizeof(tail)));
	c->numblocks = ntohl(tail.num_blocks);
	c->tocsize = ntohl(tail.index_size) * c->numblocks;
	OP(lseek(c->fh, end - sizeof(tail) - c->tocsize, SEEK_SET));
    }
    ALLOC(c->toc,sizeof(*c->toc) * c->numblocks);

    OP(read_all(c->fh,c->toc,c->tocsize));  /* read Data Index */
    build_index(c->toc, c->numblocks);
    c->cblocksizecur=0;
    c->curblock=-1;
    return 0;
};

int cloop_swap(struct cloop_data *c,ulong page){
    bfuncinfo("page=0x%lx",page);
    ulong destlen; 

    if(page==c->curblock){
	bprintf("No use to switch page\n");
	return 0;
    }
    if(page>=c->numblocks){errno=EFAULT;return -1;}
    c->curblock=page;

    bprintf("Seeking to 0x%Lx\n",c->toc[page].offset);
    OP(lseek(c->fh,c->toc[page].offset, SEEK_SET)); 

    c->cblocksize=c->toc[page].size;
    bprintf("Compressed size=%lu\n",c->cblocksize);
    if(c->cblocksize > c->cblocksizecur){
	if(c->cblocksizecur)free(c->cblock);
	c->cblocksizecur=(c->cblocksize&~0xFF)+0x100;
	bprintf("allocating 0x%lx bytes\n",c->cblocksizecur);
	ALLOC(c->cblock,c->cblocksizecur);
	OPA(c->cblock,ZER);
    }

    OP(read_all(c->fh, c->cblock, c->cblocksize));

    destlen=c->blocksize;
    bprintf(
	    "pblock=0x%lx &destlen=0x%lx cblock=0x%lx cblocksize=%lu\n",
	    (ulong)c->pblock,(ulong)&destlen,(ulong)c->cblock,c->cblocksize
	    );
    switch(uncompress(c->pblock,&destlen,c->cblock,c->cblocksize)){
	case Z_OK: break;
	#define entry(x)\
	case x: bprintf( #x"\n"); break; 
	entry(Z_MEM_ERROR)
	entry(Z_BUF_ERROR)
	entry(Z_DATA_ERROR)
	#undef entry
	default: bprintf("Z_UNKNOWN_ERROR\n");
    }
    if(destlen!=c->blocksize)bprintf("Size mismatch\n");
    return 0;
}

int cloop_read(struct cloop_data* c, off_t offset,void* buf,ulong size){
    bfuncinfo("offset=0x%llx size=0x%lx buf=0x%lx",offset,size,(ulong)buf);
    int page;
    if(!size)return 0;
    page = offset / c->blocksize;
    if(page >= c->numblocks){
	// End of file;
	return 0;
    }
    offset %= c->blocksize;
    bprintf("page=%d offset=0x%llx size=0x%lx\n",page,offset,size);
    if(cloop_swap(c,page))return -1;
    bprintf("size=0x%lx c->blocksize=0x%lx\n",size,c->blocksize);
    if(offset+size > c->blocksize){
	size=c->blocksize-offset; /* chop to size of block */
	bprintf("size updated: size=0x%lx\n",size);
    }
    memcpy(buf,c->pblock+offset,size);
    return size;
}

int cloop_read_all(struct cloop_data* c, off_t offset,void* buf,ulong size){
    bfuncinfo("offset=0x%llx size=0x%lx buf=0x%lx sizeof(off_t)=%d",offset,size,(ulong)buf,sizeof(off_t));
    char* bl=(char*)buf;
    int ret;
    int siz=size;
    for(;siz;){
	ret=cloop_read(c,offset,bl,siz);
	if(ret==-1||ret==0)return -1;
	siz-=ret;
        bl+=ret;
	offset+=ret;
    }
    return size;
}

