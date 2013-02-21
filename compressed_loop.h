/*
 * File:    compressed_loop.h 
 * Purpose: Provides constatns and structures for compressed_loop.c
 * Copyright 1999-2003 by Paul `Rusty' Russell & Klaus Knopper.
 *
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
#ifndef _COMPRESSED_LOOP_H
#define _COMPRESSED_LOOP_H

#define CLOOP_HEADROOM 128

/* The cloop header usually looks like this:          */
/* #!/bin/sh                                          */
/* #V2.00 Format                                      */
/* ...padding up to CLOOP_HEADROOM...                 */
/* block_size (32bit number, network order)           */
/* num_blocks (32bit number, network order)           */

struct cloop_head
{
	char preamble[CLOOP_HEADROOM];
	u_int32_t block_size;
	u_int32_t num_blocks;
};

/* data_index (num_blocks 64bit pointers, network order)...      */
/* compressed data (gzip block compressed format)...             */

struct cloop_tail
{
	u_int32_t index_size;
	u_int32_t num_blocks;
};

struct block_info
{
	loff_t offset;		/* 64-bit offsets of compressed block */
	u_int32_t size;		/* 32-bit compressed block size */
	u_int32_t optidx;	/* 32-bit index number */
};

static inline char *build_index(struct block_info *offsets, unsigned long n)
{
	u_int32_t *ofs32 = (u_int32_t *) offsets;
	loff_t    *ofs64 = (loff_t *) offsets;
	
	if (ofs32[0] == 0) {
		if (ofs32[2]) { /* ACCELERATED KNOPPIX V1.0 */
			while (n--) {
				offsets[n].offset = __be64_to_cpu(offsets[n].offset);
				offsets[n].size = ntohl(offsets[n].size);
			}
			return "128BE accelerated knoppix 1.0";
		}
		else { /* V2.0 */
			loff_t last = __be64_to_cpu(ofs64[n]);
			while (n--) {
				offsets[n].size = last - 
					(offsets[n].offset = __be64_to_cpu(ofs64[n])); 
				last = offsets[n].offset;
			}
			return "64BE v2.0";
		}
	}
	else if (ofs32[1] == 0) { /* V1.0 */
		loff_t last = __be64_to_cpu(ofs64[n]);
		while (n--) {
			offsets[n].size = last - 
				(offsets[n].offset = __le64_to_cpu(ofs64[n])); 
			last = offsets[n].offset;
		}
		return "64LE v1.0";
	}
	else if (ntohl(ofs32[0]) == (4*n) + 0x8C) { /* V0.68 */
		loff_t last = ntohl(ofs32[n]);
		while (n--) {
			offsets[n].size = last - 
				(offsets[n].offset = ntohl(ofs32[n])); 
			last = offsets[n].offset;
		}
		return "32BE v0.68";
	}
	else { /* V3.0 */
		int i;
		loff_t j;
		
		for (i = n; i-- > 0; )
			offsets[i].size = ntohl(ofs32[i]); 
		for (i = 0, j = sizeof(struct cloop_head); i < n; i++) {
			offsets[i].offset = j;
			j += offsets[i].size;
		}
		return "32BE v3.0";
	}
}

/* Cloop suspend IOCTL */
#define CLOOP_SUSPEND 0x4C07

#endif /*_COMPRESSED_LOOP_H*/
