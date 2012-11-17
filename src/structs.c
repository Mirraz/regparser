#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "codepages.h"
#include "structs.h"

#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#error
#endif

uint32_t size_data_area;

/* ********************************** */

#define check_struct_size(PREFIX) assert(sizeof(PREFIX##_struct) == PREFIX##_struct_size)

void structs_check_size() {
	check_struct_size(regf);
	check_struct_size(hbin);

	check_struct_size(nk);
	check_struct_size(vk);
	check_struct_size(sk);
	check_struct_size(lf);
	check_struct_size(lh);
	check_struct_size(li);
	check_struct_size(ri);
	check_struct_size(db);

	check_struct_size(value);
	check_struct_size(index);

	check_struct_size(signature);
}

#undef check_struct_size

/* ********************************** */

#define assert_check(condition, errnum) assert(condition)
//#define assert_check(condition, errnum) do{if (!(condition)) return errnum;}while(0)

#define check_signatire(PREFIX) (s->signature == PREFIX##_signature)
#define check_ptr(ptr) (ptr_is_null(ptr) || ptr < size_data_area)
#define check_size(size) (size < size_data_area)
#define check_block_size() check_size(abs(s->size))

#define print(format, args...) do{fprintf(stdout, format, ##args);}while(0)
#define printl(format, args...) do{fprintf(stdout, format "\n", ##args);}while(0)

#define print_signature_word() do{ \
	char *sig = (char *)&(s->signature); \
	printl("signature = %c%c", sig[0], sig[1]); \
}while(0)

#define printl_field(name, format) printl(#name " = " format , s->name)
#define printl_field16(name) printl_field(name, "%04X")
#define printl_field32(name) printl_field(name, "%08X")
#define printl_field64(name) printl(#name " = %016X", (unsigned int)s->name)

#define printl_size() do{ \
	print(s->size > 0 ? "FREE\n" : "USED\n"); \
	printl("size = %08X", abs(s->size)); \
}while(0)

/* ********************************** */

int regf_check(regf_struct *s) {
	assert(s != NULL);

	assert_check(check_signatire(regf), 2);

	assert_check(s->stuff1 == 1, 1);
	assert_check(s->subversion == 0, 1);
	assert_check(s->stuff2 == 1, 1);
	assert_check(s->stuff3 >= 1 && s->stuff3 <= 8, 1);
	// TODO: check checksum

	size_data_area = s->size_data_area;
	assert_check(check_ptr(s->ptr_root_nk), 1);

	return 0;
}

void regf_print(regf_struct *s) {
	assert(s != NULL);

	char *sig = (char *)&(s->signature);
	printl("signature = %c%c%c%c", sig[0], sig[1], sig[2], sig[3]);

	printl_field32(opened_transaction);
	printl_field32(closed_transaction);
	printl_field64(begin_transaction_number);
	printl_field32(stuff1);
	printl_field32(file_version);
	printl_field32(subversion);
	printl_field32(stuff2);
	printl_field32(ptr_root_nk);
	printl_field32(size_data_area);
	printl_field32(stuff3);

	printl("reg_file_path = ");
	fprintStringUnicode(stdout, s->reg_file_path, 460/2);
	printl("");

	printl("checksum = %08X", s->checksum);
	printl("");
}

/* ********************************** */

int hbin_check(hbin_struct *s) {
	assert(s != NULL);

	assert_check(check_signatire(hbin), 2);
	assert_check(check_ptr(s->ptr_self), 1);
	//if (s->ptr_self != (void *)s - ptr_data_area_begin) return 1;
	assert_check(check_size(s->size_segment), 1);
	assert_check(s->stuff1 == 0, 1);
	assert_check(s->stuff2 == 0, 1);

	return 0;
}

void hbin_print(hbin_struct *s) {
	assert(s != NULL);

	char *sig = (char *)&(s->signature);
	printl("signature = %c%c%c%c", sig[0], sig[1], sig[2], sig[3]);

	printl_field32(ptr_self);
	printl_field32(size_segment);
	printl_field32(stuff1);
	printl_field32(stuff2);
	printl_field64(time_changing_begin);
	printl_field32(stuff3);
	printl("");
}

/* ********************************** */

int nk_check(nk_struct *s) {
	assert(s != NULL);

	assert_check(check_signatire(nk), 2);
	assert_check(check_block_size(), 1);
	assert_check( (uint32_t)nk_struct_size + s->size_key_name <= abs(s->size), 1);
	// if key_name is in unicode, then size_key_name must be even
	assert_check( (s->flag & 0x20) || (s->size_key_name & 1) == 0, 1);
	assert_check(check_size(s->size_key_class), 1);

	assert_check(check_ptr(s->ptr_parent), 1);
	assert_check(check_ptr(s->ptr_chinds_index), 1);
	assert_check(check_ptr(s->ptr_params_index), 1);
	assert_check(check_ptr(s->ptr_sk), 1);
	assert_check(check_ptr(s->ptr_class_name), 1);

	return 0;
}

void nk_print(nk_struct *s) {
	assert(s != NULL);

	printl_size();
	print_signature_word();
	printl_field16(flag);
	printl_field64(time_creation);
	printl_field32(stuff1);
	printl_field32(ptr_parent);
	printl_field32(count_chinds);
	printl_field32(stuff2);
	printl_field32(ptr_chinds_index);
	printl_field32(stuff3);
	printl_field32(count_params);
	printl_field32(ptr_params_index);
	printl_field32(ptr_sk);
	printl_field32(ptr_class_name);
	unsigned int i;
	for (i=0; i<5; ++i) printl("stuff4[%d] = %08X", i, s->stuff4[i]);
	printl_field16(size_key_name);
	printl_field16(size_key_class);
	print("key_name = ");
	if (s->flag & 0x20) {
		/* ansi */
		for (i=0; i<(s->size_key_name); ++i) print("%c", s->key_name[i]);
	} else {
		/* unicode */
		fprintStringUnicode(stdout, s->key_name, (s->size_key_name)>>1);
	}
	printl("");
	printl("");
}

/* ********************************** */

int vk_check(vk_struct *s) {
	assert(s != NULL);

	assert_check(check_signatire(vk), 2);
	assert_check(check_block_size(), 1);
	assert_check(vk_struct_size + s->size_param_name <= abs(s->size), 1);
	if (!(s->size_param_value & 0x80000000)) {
		assert_check(check_size(s->size_param_value), 1);
		assert_check(check_ptr(s->ptr_param_value), 1);
	} else {
		assert_check((s->size_param_value & ~0x80000000) <= 4, 1);
	}

	return 0;
}

void vk_print(vk_struct *s) {
	assert(s != NULL);

	printl_size();
	print_signature_word();

	printl_field16(size_param_name);

	if (!(s->size_param_value & 0x80000000)) {
		printl_field32(size_param_value);
		printl_field32(ptr_param_value);
	} else {
		printl("size_param_value = %08X; in place value",
				s->size_param_value & ~0x80000000);
		printl("param_value = %08X", s->ptr_param_value);
	}

	printl_field32(param_type);
	printl_field16(stuff1);
	printl_field16(stuff2);

	print("param_name = ");
	unsigned int i;
	for (i=0; i<s->size_param_name; ++i)
		print("%c", s->param_name[i]);
	printl("");
	printl("");
}

/* ********************************** */

int lf_check(lf_struct *s) {
	assert(s != NULL);

	assert_check(check_signatire(lf), 2);
	assert_check(check_block_size(), 1);
	assert_check(lf_struct_size + s->count_records * sizeof(s->records[0]) <= abs(s->size), 1);
	unsigned int i;
	for (i=0; i<s->count_records; ++i)
		assert_check(check_ptr(s->records[i].ptr_nk), 1);

	return 0;
}

void lf_print(lf_struct *s) {
	assert(s != NULL);

	printl_size();
	print_signature_word();
	printl_field16(count_records);
	unsigned int i;
	for (i=0; i<s->count_records; ++i) {
		print("records[%d] : ptr = %08X, ", i, s->records[i].ptr_nk);
		char *str = (char *)&(s->records[i].name_begin);
		unsigned int j;
		print("name = ");
		for (j=0; j<4; ++j) {
			if (str[j] == 0) break;
			print("%c", str[j]);
		}
		printl("");
	}
	printl("");
}

/* ********************************** */

int index_check(index_struct *s, unsigned int count_records) {
	assert(s != NULL);

	assert_check(check_block_size(), 2);
	assert_check(count_records * 4 + index_struct_size <= abs(s->size), 2);

	unsigned int i;
	for (i=0; i<count_records; ++i) {
		assert_check(check_ptr(s->ptr_blocks[i]), 1);
	}

	return 0;
}

void index_print(index_struct *s, unsigned int count_records) {
	assert(s != NULL);

	printl_size();

	unsigned int i;
	for (i=0; i<count_records; ++i) {
		printl("ptr_blocks[%d] = %08X", i, s->ptr_blocks[i]);
	}
	printl("");
}
