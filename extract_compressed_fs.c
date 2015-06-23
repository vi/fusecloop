/* Extracts a filesystem back from a compressed fs file */
#define _LARGEFILE64_SOURCE
#include "common_header.h"
#include <stdint.h>
#include <inttypes.h>
#define CLOOP_PREAMBLE "#!/bin/sh\n" "#V2.0 Format\n" "insmod cloop.o file=$0 && mount -r -t iso9660 /dev/cloop $1\n" "exit $?\n"

#if __APPLE__
#define lseek64 lseek
#endif

int main(int argc, char *argv[])
{
	int handle;
	struct cloop_head head;
	unsigned int i;
	unsigned long num_blocks, block_size, zblock_maxsize;
	unsigned char *buffer, *clear_buffer;
	struct block_info *offsets;

	if (argc < 2 || argv[1][0] == '-') {
		fprintf(stderr, "Usage: extract_compressed_fs file [--convert-to-v2] > output\n");
		exit(1);
	}

	handle = open(argv[1], O_RDONLY);
	if (handle < 0) {
		perror("Opening compressed file\n");
		exit(1);
	}

	if (read(handle, &head, sizeof(head)) != sizeof(head)) {
		perror("Reading compressed file header\n");
		exit(1);
	}

	num_blocks = ntohl(head.num_blocks);
	block_size = ntohl(head.block_size);
	zblock_maxsize =  block_size + block_size/1000 + 12 + 4;
	buffer = malloc(zblock_maxsize);
	clear_buffer = malloc(block_size);
	fprintf(stderr, "%lu blocks of size %lu. Preamble:\n%s\n", 
		num_blocks, block_size, head.preamble);

	if (num_blocks == -1) {
		struct cloop_tail tail;
		loff_t end = lseek64(handle, 0, SEEK_END);
		if (lseek64(handle, end - sizeof(tail), SEEK_SET) < 0 ||
		    read(handle, &tail, sizeof(tail)) != sizeof(tail) ||
		    lseek64(handle, end - sizeof(tail) - 
		    	  (ntohl(tail.num_blocks) * ntohl(tail.index_size)), 
		    	  SEEK_SET) < 0) {
			perror("Reading tail\n");
			exit(1);
		}
		head.num_blocks = tail.num_blocks;
		num_blocks = ntohl(head.num_blocks);
		i = num_blocks * ntohl(tail.index_size);
	}
	else i = num_blocks * sizeof(*offsets);
	offsets = malloc(i);
	if (!offsets || read(handle, offsets, i) != i) {
		perror("Reading index\n");
		exit(1);
	}
	
	fprintf(stderr, "Index %s.\n", build_index(offsets, num_blocks));
	
	if (argc > 2) {
		loff_t data, offset = ((num_blocks + 1) * sizeof(offset)) + sizeof(head);
		
		strcpy(head.preamble, CLOOP_PREAMBLE);
		write(STDOUT_FILENO, &head, sizeof(head));
		for (i = 0; i < num_blocks; i++) {
			data = __be64_to_cpu(offset);
			write(STDOUT_FILENO, &data, sizeof(data));
			offset += offsets[i].size;
		}
		data = __be64_to_cpu(offset);
		write(STDOUT_FILENO, &data, sizeof(data));
		for (i = 0; i < num_blocks && lseek64(handle, offsets[i].offset, SEEK_SET) >= 0; i++) {
			read(handle, buffer, offsets[i].size);
			write(STDOUT_FILENO, buffer, offsets[i].size);
		}
		return 0;
	}
	
	for (i = 0; i < num_blocks; i++) {
		unsigned long destlen = block_size;
		unsigned int size = offsets[i].size;

		if (lseek64(handle, offsets[i].offset, SEEK_SET) < 0) {
			fprintf(stderr, "lseek to %" PRId64 ": %s\n",
				(int64_t)offsets[i].offset, strerror(errno));
			exit(1);
		}
                
		if (size > zblock_maxsize) {
			fprintf(stderr, 
				"Size %u for block %u (offset %" PRId64 ") too big\n",
				size, i, (int64_t)offsets[i].offset);
			exit(1);
		}
		read(handle, buffer, size);

		fprintf(stderr, "Block %u at %" PRId64 " length %u => %lu\n",
			i, (int64_t)offsets[i].offset, size, destlen);
		if (i == 3) {
			fprintf(stderr,
				"Block head:%02X%02X%02X%02X%02X%02X%02X%02X\n",
				buffer[0],
				buffer[1],
				buffer[2],
				buffer[3],
				buffer[4],
				buffer[5],
				buffer[6],
				buffer[7]);
			fprintf(stderr,
				"Block tail:%02X%02X%02X%02X%02X%02X%02X%02X\n",
				buffer[3063],
				buffer[3064],
				buffer[3065],
				buffer[3066],
				buffer[3067],
				buffer[3068],
				buffer[3069],
				buffer[3070]);
		}
		switch (uncompress(clear_buffer, &destlen,
				   buffer, size)) {
		case Z_OK:
			break;

		case Z_MEM_ERROR:
			fprintf(stderr, "Uncomp: oom block %u\n", i);
			exit(1);

		case Z_BUF_ERROR:
			fprintf(stderr, "Uncomp: not enough out room %u\n", i);
			exit(1);

		case Z_DATA_ERROR:
			fprintf(stderr, "Uncomp: input corrupt %u\n", i);
			exit(1);

		default:
			fprintf(stderr, "Uncomp: unknown error %u\n", i);
			exit(1);
		}
		if (destlen != block_size) {
			fprintf(stderr, "Uncomp: bad len %u (%lu not %lu)\n", i,
				destlen, block_size);
			exit(1);
		}
		write(STDOUT_FILENO, clear_buffer, block_size);
	}
	return 0;
}
