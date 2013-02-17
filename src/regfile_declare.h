#ifndef REGFILE_DECLARE_H_
#define REGFILE_DECLARE_H_

#include <stdint.h>

#include "parse_common.h"

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
	uint32_t count_childs;
	uint32_t stuff2;
	uint32_t ptr_childs_index;
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

/* ========= */

#pragma pack(push,1)
typedef struct {
	int32_t size;
	uint16_t signature;
	uint16_t stuff1;
} __attribute__ ((__packed__)) signature_struct;
#pragma pack(pop)
#define signature_struct_size 0x08

/* ********************************** */

#define PARAM_PART_MAX 0x3FD8

/* ********************************** */

typedef struct {
	uint32_t type;
	const char* const name;
} param_type_desc_struct;

#define param_type_desc_value { \
		enum_desc_item(REG_NONE), \
		enum_desc_item(REG_SZ), \
		enum_desc_item(REG_EXPAND_SZ), \
		enum_desc_item(REG_BINARY), \
		enum_desc_item(REG_DWORD), \
		enum_desc_item(REG_DWORD_BIG_ENDIAN), \
		enum_desc_item(REG_LINK), \
		enum_desc_item(REG_MULTI_SZ), \
		enum_desc_item(REG_RESOURCE_LIST), \
		enum_desc_item(REG_FULL_RESOURCE_DESCRIPTOR), \
		enum_desc_item(REG_RESOURCE_REQUIREMENTS_LIST), \
		enum_desc_item(REG_QWORD), \
		{-1, "UNKNOWN TYPE"} \
}

#define param_type_reg_value { \
		{REG_NONE,                       "hex(0)"}, \
		{REG_SZ,                         ""}, \
		{REG_EXPAND_SZ,                  "hex(2)"}, \
		{REG_BINARY,                     "hex"}, \
		{REG_DWORD,                      "dword"}, \
		{REG_DWORD_BIG_ENDIAN,           "dword"}, \
		{REG_LINK,                       "hex(6)"}, \
		{REG_MULTI_SZ,                   "hex(7)"}, \
		{REG_RESOURCE_LIST,              "hex(8)"}, \
		{REG_FULL_RESOURCE_DESCRIPTOR,   "hex(9)"}, \
		{REG_RESOURCE_REQUIREMENTS_LIST, "hex(a)"}, \
		{REG_QWORD,                      "hex(b)"}, \
		{-1, "hex"} \
}

typedef enum {
	HIVE_NTUSER = 0,
	HIVE_SAM = 1,
	HIVE_SECURITY = 2,
	HIVE_SOFTWARE = 3,
	HIVE_SYSTEM = 4,
	HIVE_DEFAULT = 5,
	HIVE_USERDIFF = 6,
	HIVE_UNKNOWN = 7
} hive_enum;
#define hives_count 7

#define hive_filename_value { \
		"NTUSER.DAT", \
		"SAM", \
		"SECURITY", \
		"SOFTWARE", \
		"SYSTEM", \
		"DEFAULT", \
		"USERDIFF" \
}

#define hive_keypath_value { \
		"HKEY_CURRENT_USER", \
		"HKEY_LOCAL_MACHINE\\SAM", \
		"HKEY_LOCAL_MACHINE\\SECURITY", \
		"HKEY_LOCAL_MACHINE\\SOFTWARE", \
		"HKEY_LOCAL_MACHINE\\SYSTEM", \
		"HKEY_USERS\\.DEFAULT", \
		"", \
		"" \
}

/* ********************************** */

#endif /* REGFILE_DECLARE_H_ */
