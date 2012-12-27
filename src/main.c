#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"
#include "regfile.h"

#define ptr_not_null(ptr) ((ptr) != (uint32_t)-1)

FILE *fout;

void usage(const char *prog_name) {
	printf("Usage: %s [-h] [-r] regfile_path key_path\n", prog_name);
}

int main (int argc, char *argv[]) {
	structs_check_size();
	
	bool recursive_flag = false;
	const char *regfile_path = NULL;
	const char *key_path = NULL;
	
	static const struct option long_options[] = {
		{"help",       no_argument,       NULL, 'h'},
		{"recursive",  no_argument,       NULL, 'r'},
		{0, 0, 0, 0}
	};
	static const char short_options[] = "hr";
	do {
		int option_index = 0;
		int c = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (c == -1) break;
		switch (c) {
		case 'r':
			recursive_flag = true;
			break;
		case 'h':
		case '?':
		default:
			usage(argv[0]);
			return 0;
		}
	} while (1);
	if (argc != optind + 2) {
		usage(argv[0]);
		return 0;
	}
	regfile_path = argv[optind++];
	key_path = argv[optind];
	
	
	int fd = open(regfile_path, O_RDONLY);

	static regf_struct header_data;
	regf_struct *header = &header_data;
	ssize_t red = read(fd, header, regf_struct_size);
	assert(red == regf_struct_size);
	regf_init(header);

	size_t mmap_file_size = header->size_data_area;
	uint8_t *data = mmap(NULL, mmap_file_size,
			PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, regf_header_size);
	assert(data != MAP_FAILED);
	set_data(data);

	fout = stdout;
	nk_struct *root = nk_init(header->ptr_root_nk);
	nk_struct *s = nk_cd(root, key_path);

	if (s != NULL) {
		if (recursive_flag) {
			nk_recur(s);
		} else {
			nk_print_verbose(s);
		}
	} else {
		fprintf(fout, "Not found\n");
	}

	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

