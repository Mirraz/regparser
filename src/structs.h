#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

#define ptr_is_null(ptr) ((ptr) == (uint32_t)-1)
#define ptr_not_null(ptr) ((ptr) != (uint32_t)-1)

/* ********************************** */

#define regf_header_size 0x1000

#pragma pack(push,1)
typedef struct {
	uint32_t signature;
	uint32_t opened_transaction;
	uint32_t closed_transaction;
	uint64_t begin_transaction_number;
	uint32_t stuff1;
	uint32_t file_version;
	uint32_t subversion;
	uint32_t stuff2;
	uint32_t ptr_root_nk;
	uint32_t size_data_area;
	uint32_t stuff3;
	uint8_t  reg_file_path[460];
	uint32_t checksum;
} __attribute__ ((__packed__)) regf_struct;
#pragma pack(pop)
#define regf_struct_size 0x200
#define regf_signature 0x66676572

#pragma pack(push,1)
typedef struct {
	uint32_t signature;
	uint32_t ptr_self;
	uint32_t size_segment;
	uint32_t stuff1;
	uint32_t stuff2;
	uint64_t time_changing_begin;
	uint32_t stuff3;
} __attribute__ ((__packed__)) hbin_struct;
#pragma pack(pop)
#define hbin_struct_size 0x20
#define hbin_signature 0x6E696268

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t flag;
	uint64_t time_creation;
	uint32_t stuff1;
	uint32_t ptr_parent;
	uint32_t count_chinds;
	uint32_t stuff2;
	uint32_t ptr_chinds_index;
	uint32_t stuff3;
	uint32_t count_params;
	uint32_t ptr_params_index;
	uint32_t ptr_sk;
	uint32_t ptr_class_name;
	uint32_t stuff4[5];
	uint16_t size_key_name;
	uint16_t size_key_class;
	uint8_t  key_name[];
} __attribute__ ((__packed__)) nk_struct;
#pragma pack(pop)
#define nk_struct_size 0x50
#define nk_signature 0x6B6E

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t size_param_name;
	uint32_t size_param_value;
	uint32_t ptr_param_value;
	uint32_t param_type;
	uint16_t flag;
	uint16_t stuff2;
	uint8_t  param_name[];
} __attribute__ ((__packed__)) vk_struct;
#pragma pack(pop)
#define vk_struct_size 0x18
#define vk_signature 0x6B76

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t stuff1;
	uint32_t ptr_prev_sk;
	uint32_t ptr_next_sk;
	uint32_t count_ref;
	uint32_t size_data;
	uint8_t  data[];
} __attribute__ ((__packed__)) sk_struct;
#pragma pack(pop)
#define sk_struct_size 0x18
#define sk_signature 0x6B73

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t count_records;
	struct {
		uint32_t ptr_nk;
		uint32_t name_begin;
	} records[];
} __attribute__ ((__packed__)) lf_struct;
#pragma pack(pop)
#define lf_struct_size 0x08
#define lf_signature 0x666C

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t count_records;
	struct {
		uint32_t ptr_nk;
		uint32_t name_hash;
	} records[];
} __attribute__ ((__packed__)) lh_struct;
#pragma pack(pop)
#define lh_struct_size 0x08
#define lh_signature 0x686C

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t count_records;
	uint32_t ptr_nks[];
} __attribute__ ((__packed__)) li_struct;
#pragma pack(pop)
#define li_struct_size 0x08
#define li_signature 0x696C

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t count_records;
	uint32_t ptr_indexes[];
} __attribute__ ((__packed__)) ri_struct;
#pragma pack(pop)
#define ri_struct_size 0x08
#define ri_signature 0x6972

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t count_records;
	uint32_t ptr_value_parts_index;
} __attribute__ ((__packed__)) db_struct;
#pragma pack(pop)
#define db_struct_size 0x0C
#define db_signature 0x6264

/* ========= */

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint8_t value[];
} __attribute__ ((__packed__)) value_struct;
#pragma pack(pop)
#define value_struct_size 0x04

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint32_t ptr_blocks[];
} __attribute__ ((__packed__)) index_struct;
#pragma pack(pop)
#define index_struct_size 0x04

/* ********************************** */

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t stuff1;
} __attribute__ ((__packed__)) signature_struct;
#pragma pack(pop)
#define signature_struct_size 0x08

/* ********************************** */

void structs_check_size();

regf_struct *regf_init(regf_struct *s);
void set_data(uint8_t *data);

hbin_struct *hbin_init(uint32_t ptr);
nk_struct *nk_init(uint32_t ptr);

/* FIXME */
extern FILE *fout;

void nk_print_name(nk_struct *s);
void nk_print_class(nk_struct *s);
void nk_print_sk(nk_struct *s);
void nk_ls_params(nk_struct *s);
void nk_ls_childs(nk_struct *s);

void parse_childs(uint32_t ptr_chinds_index, void (*cb)(nk_struct *));

#endif /* MAIN_H_ */
