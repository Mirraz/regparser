#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "structs.h"

int main (int argc, char *argv[]) {
	regf_check_struct_size();
	hbin_check_struct_size();
	nk_check_struct_size();

	assert(argc == 2);
	int fd = open(argv[1], O_RDONLY);

	regf_struct *header = malloc(regf_struct_size);
	ssize_t red = read(fd, header, regf_struct_size);
	assert(red == regf_struct_size);
	assert(!regf_check(header));
	regf_print(header);
	size_t mmap_file_size = header->size_data_area;

	uint8_t *data = mmap(NULL, mmap_file_size, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, regf_header_size);
	assert(data != MAP_FAILED);




	hbin_struct *hbin1 = (hbin_struct *)data;
	assert(!hbin_check(hbin1));
	hbin_print(hbin1);

	nk_struct *nkroot = (nk_struct *)(data + header->ptr_root_nk);
	assert(!nk_check((nkroot)));
	nk_print(nkroot);




	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

