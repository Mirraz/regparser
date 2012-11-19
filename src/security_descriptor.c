#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "security_descriptor_declare.h"
#include "security_descriptor.h"

/* ********************************** */

#define flag_is_seted(value, flag) ((value) & (flag))
/* "ptr < sec_size" -- overflow protection */
#define check_ptr(ptr, PREFIX) (ptr < sec_size && ptr + PREFIX##_struct_size <= sec_size)

/* ********************************** */

static const flag_desc sd_flags_desc[] = sd_flags_desc_value;
static const enum8_desc ace_types_desc[] = ace_types_desc_value;
static const enum8_desc ace_flags_desc[] = ace_flags_desc_value;
static const flag_desc access_mask_flags_desc[] = access_mask_flags_desc_value;

uint8_t *sec_data = NULL;
uint32_t sec_size = 0;

/* ********************************** */

void secstructs_check_size() {
	check_struct_size(sd);
	check_array_size(sd_flags_desc, sd_flags_count);
	check_struct_size(acl);
	check_struct_size(ace);
	check_array_size(ace_types_desc, ace_types_count+1);
	check_array_size(ace_flags_desc, ace_flags_count);
	check_struct_size(ace_mask_sid);
	check_array_size(access_mask_flags_desc, access_mask_flags_count);
	check_struct_size(sid);
}

/* ********************************** */

sd_struct *sd_init(uint8_t *sec_data_, uint32_t sec_size_) {
	sec_data = sec_data_;
	sec_size = sec_size_;

	sd_struct *s = (sd_struct *)(sec_data);

	assert_check1(s->revision == 1);
	assert_check1(flag_is_seted(s->control, SDF_SR));

	if (flag_is_seted(s->control, SDF_OD)) {
		assert_check1(s->offset_owner == 0);
	} else {
		assert_check1(s->offset_owner + 8 <= sec_size);
	}

	if (flag_is_seted(s->control, SDF_GD)) {
		assert_check1(s->offset_group == 0);
	} else {
		assert_check1(s->offset_group + 8 <= sec_size);
	}

	if (flag_is_seted(s->control, SDF_SP)) {
		/*assert_check1(!flag_is_seted(s->control, SDF_SD));*/
		assert_check1(s->offset_sacl + 8 <= sec_size);
	} else {
		assert_check1(s->offset_sacl == 0);
	}

	if (flag_is_seted(s->control, SDF_DP)) {
		/*assert_check1(!flag_is_seted(s->control, SDF_DD));*/
		assert_check1(s->offset_dacl + 8 <= sec_size);
	} else {
		assert_check1(s->offset_dacl == 0);
	}

	return s;
}

sid_struct *sid_init(uint32_t ptr) {
	assert_check1(check_ptr(ptr, sid));
	sid_struct *s = (sid_struct *)(sec_data + ptr);

	assert_check1(s->revision == 1);
	assert_check1(s->sub_authority_count <= 15);

	return s;
}

acl_struct *acl_init(uint32_t ptr) {
	assert_check1(check_ptr(ptr, acl));
	acl_struct *s = (acl_struct *)(sec_data + ptr);

	assert_check1(s->revision == 2 || s->revision == 4);
	assert_check1(s->sbz1 == 0);
	assert_check1(s->size < sec_size && ptr + s->size <= sec_size);
	assert_check1(acl_struct_size + s->ace_count*4 <= s->size);
	assert_check1(s->sbz2 == 0);

	return s;
}

/* ********************************** */

void sid_print(sid_struct *s) {
	fprintf(fout, "S");
	uint8_t *ia = s->identifier_authority;
	uint64_t identifier_authority =
			((uint64_t)(ia[0]) << 40) | ((uint64_t)(ia[1]) << 32) |
			((uint64_t)(ia[2]) << 24) | ((uint64_t)(ia[3]) << 16) |
			((uint64_t)(ia[4]) <<  8) | (uint64_t)(ia[5]);
	fprintf(fout, "-%lld", (long long int)identifier_authority);
	unsigned int i;
	for (i=0; i<s->sub_authority_count; ++i) {
		fprintf(fout, "-%d", s->sub_authority[i]);
	}
}

void access_mask_print(uint32_t mask) {
	unsigned int i;
	for (i=0; i<access_mask_flags_count; ++i)
		if (mask & access_mask_flags_desc[i].flag)
			fprintf(fout, "%s ", access_mask_flags_desc[i].short_name);
}

void ace_print(ace_struct *s) {
	unsigned int i;
	
	/* type */
	for (i=0; i<ace_types_count && ace_types_desc[i].value != s->type ; ++i);
	fprintf(fout, "type = %s\n", ace_types_desc[i].name);
	
	/* flags */
	fprintf(fout, "flags: ");
	for (i=0; i<ace_flags_count; ++i)
		if (s->flags & ace_flags_desc[i].value)
			fprintf(fout, "%s ", ace_flags_desc[i].name);
	fprintf(fout, "\n");
	
	switch (s->type) {
	case ACCESS_ALLOWED_ACE_TYPE:
	case ACCESS_DENIED_ACE_TYPE:
	case SYSTEM_AUDIT_ACE_TYPE: {
		ace_mask_sid_struct *ace = (ace_mask_sid_struct *)s;
		fprintf(fout, "mask: ");
		access_mask_print(ace->mask);
		fprintf(fout, "\n");
		fprintf(fout, "sid = ");
		sid_print((sid_struct *)ace->sid);
		fprintf(fout, "\n");
		fprintf(fout, "-----\n");
		break;
	}
	default: {
		fprintf(fout, "unsupported ace type\n");
		break;
	}
	}
}

void acl_print(acl_struct *s) {
	uint32_t remainder = s->size - acl_struct_size;
	uint32_t prev_ace_size = acl_struct_size;
	ace_struct *ace = (ace_struct *)s;
	unsigned int i;
	for (i=0; i<s->ace_count; ++i) {
		ace = (ace_struct *)((uint8_t *)ace + prev_ace_size);
		assert_check1(remainder >= ace->size);

		ace_print(ace);

		prev_ace_size = ace->size;
		remainder -= ace->size;
	}
}

void sd_print(sd_struct *s) {
	fprintf(fout, "flags: ");
	unsigned int i;
	for (i=0; i<sd_flags_count; ++i)
		if (s->control & sd_flags_desc[i].flag)
			fprintf(fout, "%s ", sd_flags_desc[i].short_name);
	fprintf(fout, "\n");

	if (!flag_is_seted(s->control, SDF_OD)) {
		fprintf(fout, "owner = ");
		sid_print(sid_init(s->offset_owner));
		fprintf(fout, "\n");
	}
	if (!flag_is_seted(s->control, SDF_GD)) {
		fprintf(fout, "group = ");
		sid_print(sid_init(s->offset_group));
		fprintf(fout, "\n");
	}
	if (flag_is_seted(s->control, SDF_SP)) {
		fprintf(fout, "[sacl]\n");
		/*acl_print(acl_init(s->offset_sacl));*/
	}
	if (flag_is_seted(s->control, SDF_DP)) {
		fprintf(fout, "[dacl]\n");
		acl_print(acl_init(s->offset_dacl));
	}
}

void sec_data_parse(uint8_t *sec_data_, uint32_t sec_data_size_) {
	sd_print(sd_init(sec_data_, sec_data_size_));
}

/* ********************************** */
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

FILE *fout;

int main (int argc, char *argv[]) {
	secstructs_check_size();

#define sd_size 256

	assert(argc == 2);
	int fd = open(argv[1], O_RDONLY);
	size_t mmap_file_size = sd_size;
	uint8_t *data = mmap(NULL, mmap_file_size,
			PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
	assert(data != MAP_FAILED);



	fout = stdout;
	sec_data_parse(data, sd_size);




	assert(!munmap(data, mmap_file_size));
	assert(!close(fd));

	return 0;
}
*/
