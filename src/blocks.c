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

	uint32_t ptr_segm = 0;
	do {
		hbin_struct *hbin = hbin_init(ptr_segm);
		fprintf(fout, "segment: ptr = %08X size = %08X\n", ptr_segm, hbin->size_segment);
		uint32_t ptr_block = ptr_segm + hbin_struct_size;
		do {
			value_struct *block = (value_struct *)(data + ptr_block);
			uint32_t block_size = abs(block->size);
			fprintf(fout, "block: ptr = %08X size = %08X (%s) ", ptr_block, block_size, (block->size < 0 ? "USED" : "FREE"));
			
			if (block_size >= 6) {
				signature_struct *sig_block = (signature_struct *)block;
				switch (sig_block->signature) {
				case nk_signature:
				case vk_signature:
				case sk_signature:
				case lf_signature:
				case lh_signature:
				case li_signature:
				case ri_signature:
				case db_signature:
				{
					const char* sig = (const char*)&sig_block->signature;
					fprintf(fout, "%c%c\n", sig[0], sig[1]);
					break;
				}
				default:
					fprintf(fout, "NOTSIG\n");
					break;
				}
			} else {
				fprintf(fout, "NOTSIG\n");
			}
			
			ptr_block += block_size;
		} while (ptr_block < ptr_segm + hbin->size_segment);
		assert(ptr_block == ptr_segm + hbin->size_segment);
		ptr_segm += hbin->size_segment;
	} while(ptr_segm < header->size_data_area);
	assert(ptr_segm == header->size_data_area);

	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}

