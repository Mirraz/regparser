#include <stdio.h>
#include <assert.h>
#include <endian.h>
#include <stdbool.h>

#include "codepages.h"
#include "structs.h"

#define correct_field16(name) do{s->name = le16toh(s->name);}while(0)
#define correct_field32(name) do{s->name = le32toh(s->name);}while(0)
#define correct_field64(name) do{s->name = le64toh(s->name);}while(0)

#define check_struct_size(PREFIX) void PREFIX##_check_struct_size() { \
	assert(sizeof(PREFIX##_struct) == PREFIX##_struct_size); \
}

#define check_signature_word(c1, c2) do{ \
	char *sig = (char *)&(s->signature); \
	if (sig[0] != c1 || sig[1] != c2) return 2; \
}while(0)

#define print(format, args...) do{fprintf(stdout, format, ##args);}while(0)
#define printl(format, args...) do{fprintf(stdout, format "\n", ##args);}while(0)

#define print_signature_word() do{ \
	char *sig = (char *)&(s->signature); \
	printl("signature = %c%c", sig[0], sig[1]); \
}while(0)

#define printl_field(name, format) printl(#name " = " format , s->name)
#define printl_field16(name) printl_field(name, "%04X")
#define printl_field32(name) printl_field(name, "%08X")
#define printl_field64(name) printl_field(name, "%016X")

/* ********************************** */

check_struct_size(regf)

void regf_correct(regf_struct *s) {
	assert(s != NULL);

	correct_field32(opened_transaction);
	correct_field32(closed_transaction);
	correct_field64(begin_transaction_number);
	correct_field32(stuff1);
	correct_field32(file_version);
	correct_field32(subversion);
	correct_field32(stuff2);
	correct_field32(ptr_root_nk);
	correct_field32(size_data_area);
	correct_field32(stuff3);
	correct_field32(checksum);
}

int regf_check(regf_struct *s) {
	assert(s != NULL);

	char *sig = (char *)&(s->signature);
	if (sig[0] != 'r' || sig[1] != 'e' || sig[2] != 'g' || sig[3] != 'f') return 2;

	if (s->stuff1 != 1) return 1;
	if (s->subversion != 0) return 1;
	if (s->stuff2 != 1) return 1;
	if (!(s->stuff3 >= 1 && s->stuff3 <= 8)) return 1;
	// TODO: check checksum

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
}

/* ********************************** */

check_struct_size(hbin)

void hbin_correct(hbin_struct *s) {
	assert(s != NULL);

	correct_field32(ptr_self);
	correct_field32(size_segment);
	correct_field32(stuff1);
	correct_field32(stuff2);
	correct_field64(time_changing_begin);
	correct_field32(stuff3);
}

int hbin_check(hbin_struct *s) {
	assert(s != NULL);

	char *sig = (char *)&(s->signature);
	if (sig[0] != 'h' || sig[1] != 'b' || sig[2] != 'i' || sig[3] != 'n') return 2;
	//if (s->ptr_self != self_seek - regf_header_size) return 1;
	if (s->stuff1 != 0) return 1;
	if (s->stuff2 != 0) return 1;

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
}

/* ********************************** */

check_struct_size(nk)

void nk_correct(nk_struct *s) {
	assert(s != NULL);

	s->size = - le32toh(s->size);
	correct_field16(flag);
	correct_field64(time_creation);
	correct_field32(stuff1);
	correct_field32(ptr_parent);
	correct_field32(count_chinds);
	correct_field32(stuff2);
	correct_field32(ptr_chinds_index);
	correct_field32(stuff3);
	correct_field32(count_params);
	correct_field32(ptr_params_index);
	correct_field32(ptr_sk);
	correct_field32(ptr_class_name);
	unsigned int i;
	for (i=0; i<5; ++i) s->stuff4[i] = le32toh(s->stuff4[i]);
	correct_field16(size_key_name);
	correct_field16(size_key_class);
}

int nk_check(nk_struct *s) {
	assert(s != NULL);

	check_signature_word('n', 'k');
	if (!( (uint32_t)nk_struct_size + s->size_key_name <= s->size )) return 2;
	// if key_name is in unicode, then size_key_name must be even
	if (!(s->flag & 0x20) && (s->size_key_name & 1) != 0) return 1;

	return 0;
}

void nk_print(nk_struct *s) {
	assert(s != NULL);

	printl_field32(size);
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
	printl("key_name =");
	if (s->flag & 0x20) {
		/* ansi */
		for (i=0; i<(s->size_key_name); ++i) print("%c", s->key_name[i]);
	} else {
		/* unicode */
		fprintStringUnicode(stdout, s->key_name, (s->size_key_name)>>1);
	}
	printl("");
}

/* ********************************** */

