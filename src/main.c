#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "structs.h"

uint8_t *data;

void parse_child(uint32_t ptr_chinds_index);

void parse_nk(uint32_t ptr_nk) {
	nk_struct *nk1 = (nk_struct *)(data + ptr_nk);
	assert(!nk_check((nk1)));
	nk_print(nk1);

	if (ptr_not_null(nk1->ptr_params_index)) {
		unsigned int count_index_records = nk1->count_params;

		index_struct *index_params = (index_struct *)(data + nk1->ptr_params_index);
		assert(!index_check(index_params, count_index_records));
		index_print(index_params, count_index_records);

		unsigned int i;
		for (i=0; i<count_index_records; ++i) {
			vk_struct *vk1 = (vk_struct *)(data + index_params->ptr_blocks[i]);
			assert(!vk_check(vk1));
			vk_print(vk1);
		}
	}

	parse_child(nk1->ptr_chinds_index);
}


void parse_child(uint32_t ptr_chinds_index) {
	if (!ptr_not_null(ptr_chinds_index)) return;
	signature_struct *sig = (signature_struct *)(data + ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature:
	{
		lf_struct *lf1 = (lf_struct *)(data + ptr_chinds_index);
		assert(!lf_check(lf1));
		lf_print(lf1);
		unsigned int i;
		for (i=0; i<lf1->count_records; ++i)
			parse_nk(lf1->records[i].ptr_nk);
		break;
	}
	case lh_signature:
	{
		//lh_struct *lh1 = (lh_struct *)(data + ptr_chinds_index);
printf("lh\n\n");
		break;
	}
	case li_signature:
	{
		//li_struct *li1 = (li_struct *)(data + ptr_chinds_index);
printf("li\n\n");
		break;
	}
	case ri_signature:
	{
		//ri_struct *ri1 = (ri_struct *)(data + ptr_chinds_index);
printf("ri\n\n");
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
}

int main (int argc, char *argv[]) {
	structs_check_size();

	assert(argc == 2);
	int fd = open(argv[1], O_RDONLY);

	regf_struct *header = malloc(regf_struct_size);
	ssize_t red = read(fd, header, regf_struct_size);
	assert(red == regf_struct_size);
	assert(!regf_check(header));
	regf_print(header);
	size_t mmap_file_size = header->size_data_area;

	data = mmap(NULL, mmap_file_size, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, regf_header_size);
	assert(data != MAP_FAILED);



	hbin_struct *hbin1 = (hbin_struct *)data;
	assert(!hbin_check(hbin1));
	hbin_print(hbin1);


	parse_nk(header->ptr_root_nk);



	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

