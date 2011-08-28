/* Extracts a filesystem back from a compressed fs file */
#include "common_header.h"

int main(int argc, char *argv[])
{
	int handle;
	struct cloop_head head;
	unsigned int i;
	unsigned char *buffer, *clear_buffer;

	if (argc != 2) {
		fprintf(stderr, "Need filename\n");
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

	buffer = malloc(ntohl(head.block_size) + ntohl(head.block_size)/1000
			+ 12 + 4);
	clear_buffer = malloc(ntohl(head.block_size));
	fprintf(stderr, "%u blocks of size %u. Preamble:\n%s\n", 
		ntohl(head.num_blocks), ntohl(head.block_size), head.preamble);

	for (i = 0; i < ntohl(head.num_blocks); i++) {
		int currpos;
		unsigned long destlen = ntohl(head.block_size);
		loff_t offset[2];
		unsigned int size;

		read(handle, &offset, 2*sizeof(loff_t));
                lseek(handle, -sizeof(loff_t), SEEK_CUR);
                
		currpos = lseek(handle, 0, SEEK_CUR);
		if (lseek(handle, __be64_to_cpu(offset[0]), SEEK_SET) < 0) {
			fprintf(stderr, "lseek to %Lu: %s\n",
				__be64_to_cpu(offset[0]), strerror(errno));
			exit(1);
		}
                
                size=__be64_to_cpu(offset[1])-__be64_to_cpu(offset[0]);
		if (size > ntohl(head.block_size) + ntohl(head.block_size)/1000
		    + 12 + 4) {
			fprintf(stderr, 
				"Size %u for block %u (offset %Lu) too big\n",
				size, i, __be64_to_cpu(offset[0]));
			exit(1);
		}
		read(handle, buffer, size);
		if (lseek(handle, currpos, SEEK_SET) < 0) {
			perror("seeking");
			exit(1);
		}

		fprintf(stderr, "Block %u length %u => %lu\n",
			i, size, destlen);
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
		if (destlen != ntohl(head.block_size)) {
			fprintf(stderr, "Uncomp: bad len %u (%lu not %u)\n", i,
				destlen, ntohl(head.block_size));
			exit(1);
		}
		write(STDOUT_FILENO, clear_buffer, ntohl(head.block_size));
	}
	return 0;
}
