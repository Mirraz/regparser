#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"
#include "regfile.h"

FILE *fout;

int main (int argc, char *argv[]) {
	structs_check_size();

	assert(argc == 2);
	int fd = open(argv[1], O_RDONLY);

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

	nk_recur(nk_init(header->ptr_root_nk));

	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

