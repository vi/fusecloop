/* Creates a compressed file */
#include "common_header.h"

#define CLOOP_PREAMBLE "#!/bin/sh\n" "#V3.0 Format\n" "insmod cloop.o file=$0 && mount -r -t iso9660 /dev/cloop $1\n" "exit $?\n"
#define CHUNK 65536
#define DEFAULT_BLOCKSIZE 65536

static void quit(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

static int readblock(unsigned char *buffer, int n)
{
	int i;
	
	memset(buffer, 0, n);
	for (i = 0 ; i < n;) {
		int j = read(STDIN_FILENO, buffer + i, n - i);
		if (j < 0 && errno == EINTR) continue;
		if (j <= 0) break;
		i += j;
	}
	return i;
}

int main(int argc, char *argv[])
{
	struct cloop_head head;
	struct cloop_tail tail;
	unsigned long  block_size = 0;
	unsigned char *compressed, *uncompressed;
	uint32_t *index;
	int n, indexmax, zlenmax;
	
	if (argc > 1) {
		if (argv[1][0] < '0' || argv[1][0] > '9')
			quit("Usage : create_compressed_fs [block size] < input > output");
		block_size = atoi(argv[1]);
	}
	if (block_size < 4096)
		block_size = DEFAULT_BLOCKSIZE;
	fprintf(stderr, "Block size is %lu\n", block_size);
	zlenmax = block_size + block_size/1000 + 12;

	memset(&head, 0, sizeof(head));
	strcpy(head.preamble, CLOOP_PREAMBLE);
	head.num_blocks = -1;
	head.block_size = htonl(block_size);
	write(STDOUT_FILENO, &head, sizeof(head));
	
	compressed = malloc(zlenmax);
	uncompressed = malloc(block_size);
	index = malloc(indexmax = CHUNK);
	if (!compressed || !uncompressed || !index)
		quit("Malloc failed");
	
	for (n = 0; readblock(uncompressed, block_size); n++) {
		unsigned long len = zlenmax;
		
		if (compress2(compressed, &len, uncompressed, block_size, 
				Z_BEST_COMPRESSION) != Z_OK)
			quit("Compression failed");
		fprintf(stderr, "Block %u length %lu => %lu\n",
			n, block_size, len);
		write(STDOUT_FILENO, compressed, len);
		if (n * sizeof(*index) >= indexmax) {
			index = realloc(index, indexmax += CHUNK);
			if (!index)
				quit("Realloc");
		}
		index[n] = htonl(len);
	}
	write(STDOUT_FILENO, index, n * sizeof(*index));
	tail.index_size = htonl(sizeof(*index));
	tail.num_blocks = htonl(n);
	write(STDOUT_FILENO, &tail, sizeof(tail));
	return 0;
}
