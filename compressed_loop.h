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

/* Cloop suspend IOCTL */
#define CLOOP_SUSPEND 0x4C07

#endif /*_COMPRESSED_LOOP_H*/
