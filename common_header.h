/*
 * File:    common_header.c 
 * Purpose: Header file for extract_compressed_fs and fusecloop.
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

#ifndef COMMON_HEADER
#define COMMON_HEADER

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <zlib.h>
#if defined(__FreeBSD__)
#include <sys/endian.h>
#include <netinet/in.h>
typedef uint64_t loff_t;
#ifndef be64toh
static __inline __uint64_t
__bswap64(__uint64_t _x)
{

	return ((_x >> 56) | ((_x >> 40) & 0xff00) | ((_x >> 24) & 0xff0000) |
	    ((_x >> 8) & 0xff000000) | ((_x << 8) & ((__uint64_t)0xff << 32)) |
	    ((_x << 24) & ((__uint64_t)0xff << 40)) |
	    ((_x << 40) & ((__uint64_t)0xff << 48)) | ((_x << 56)));
}
#if BYTE_ORDER == LITTLE_ENDIAN
#define be64toh(x)	__bswap64(x)
#else 
#define be64toh(x)
#endif
#endif /* be64toh */
#define __be64_to_cpu be64toh
#else  /* __FreeBSD__ */
#ifndef __APPLE__
#include <asm/byteorder.h>
#else /* __APPLE__ */
#include <libkern/OSByteOrder.h>
#define __be64_to_cpu OSSwapBigToHostInt64
#define __le64_to_cpu OSSwapLittleToHostInt64
typedef off_t loff_t;
#endif /* __APPLE__ */
#include <arpa/inet.h> /* ntohl */
#endif /* __FreeBSD__ */
#include "compressed_loop.h"

struct compressed_block
{
	size_t size;
	void *data;
};

#endif  /* COMMON_HEADER */
