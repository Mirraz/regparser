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
	structs_check_size();

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

	nk_struct *nk_root = (nk_struct *)(data + header->ptr_root_nk);
	assert(!nk_check((nk_root)));
	nk_print(nk_root);

	if (ptr_not_null(nk_root->ptr_params_index)) {
		index_struct *index_params = (index_struct *)(data + nk_root->ptr_params_index);
		assert(!index_check(index_params));
		index_print(index_params);
	}

	if (ptr_not_null(nk_root->ptr_chinds_index)) {
		signature_struct *sig = (signature_struct *)(data + nk_root->ptr_chinds_index);
		switch (sig->signature) {
		case lf_signature:
		{
			//lf_struct *lf1 = (lf_struct *)(data + nk_root->ptr_chinds_index);
printf("lf\n");
			break;
		}
		case lh_signature:
		{
			//lh_struct *lh1 = (lh_struct *)(data + nk_root->ptr_chinds_index);
printf("lh\n");
			break;
		}
		case li_signature:
		{
			//li_struct *li1 = (li_struct *)(data + nk_root->ptr_chinds_index);
printf("li\n");
			break;
		}
		case ri_signature:
		{
			//ri_struct *ri1 = (ri_struct *)(data + nk_root->ptr_chinds_index);
printf("ri\n");
			break;
		}
		default:
		{
			break;
		}
		}
	}


	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

