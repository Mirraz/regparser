#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "structs.h"

void nk_recur(nk_struct *s) {
	assert(s != NULL);

	fprintf(fout, "[key name]\n");
	nk_print_name(s);
	fprintf(fout, "\n");

	if (ptr_not_null(s->ptr_class_name)) {
		fprintf(fout, "[key class]\n");
		nk_print_class(s);
		fprintf(fout, "\n");
	}

	if (ptr_not_null(s->ptr_sk)) {
		fprintf(fout, "[sk]\n");
		nk_print_sk(s);
		fprintf(fout, "\n");
	}

	if (s->count_params != 0) {
		fprintf(fout, "[params]\n");
		nk_ls_params(s);
	}

	if (ptr_not_null(s->ptr_chinds_index)) {
		fprintf(fout, "[childs]\n");
		nk_ls_childs(s);
	}
	fprintf(fout, "===============\n");

	if (ptr_not_null(s->ptr_chinds_index)) {
		parse_childs(s->ptr_chinds_index, &nk_recur);
	}
}

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

	hbin_struct *hbin1 = hbin_init(0);
	nk_struct *nk = nk_init(header->ptr_root_nk);
	nk_recur(nk);

	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

