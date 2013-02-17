#ifndef REGFILE_H_
#define REGFILE_H_

#include <stdint.h>
#include "string_type.h"

#define ptr_is_null(ptr) ((ptr) == (uint32_t)-1)
#define ptr_not_null(ptr) ((ptr) != (uint32_t)-1)
#define ptr_null ((uint32_t)-1)

typedef enum {
	REG_NONE =		0x00,
	REG_SZ =		0x01,
	REG_EXPAND_SZ =	0x02,
	REG_BINARY =	0x03,
	REG_DWORD =		0x04,
	REG_DWORD_LITTLE_ENDIAN = REG_DWORD,
	REG_DWORD_BIG_ENDIAN = 0x05,
	REG_LINK =		0x06,
	REG_MULTI_SZ =	0x07,
	REG_RESOURCE_LIST = 0x08,
	REG_FULL_RESOURCE_DESCRIPTOR = 0x09,
	REG_RESOURCE_REQUIREMENTS_LIST = 0x0A,
	REG_QWORD =		0x0B
} param_types;
#define param_types_count 12

uint32_t regfile_init(const char *regfile_path);
int regfile_uninit();

uint32_t nk_find_child(uint32_t ptr, string name);
uint32_t nk_get_parent(uint32_t ptr);

typedef struct {
	string str;
	uint32_t ptr;
} string_and_ptr;

typedef struct {
	string_and_ptr *entries;
	unsigned int size;
} string_and_ptr_list;

void string_and_ptr_list_free(string_and_ptr_list *p_list);
string_and_ptr_list nk_get_childs_list(uint32_t ptr);
string_and_ptr_list nk_get_params_names_list(uint32_t ptr);

typedef struct {
	string name;
	string type;
	uint32_t size_value;
	string value_brief;
	uint32_t ptr;
} param_parsed;

typedef struct {
	param_parsed *entries;
	unsigned int size;
} params_parsed_list;

void params_parsed_list_free(params_parsed_list *p_list);
params_parsed_list nk_get_params_parsed_list(uint32_t ptr);

typedef struct {
	string *entries;
	unsigned int size;
} string_list;

typedef union {
	uint32_t dword;
	uint64_t qword;
	struct {
		uint8_t *data;
		uint32_t size;
	} hex;
	string str;
	string_list multi_str;
} param_value;

typedef struct {
	string name;
	uint32_t type;
	string type_str;
	uint32_t size_value;
	param_value value;
} param_parsed_full;

void param_parsed_full_free(param_parsed_full *p_param);
param_parsed_full vk_get_parsed(uint32_t ptr);

void string_list_free(string_list *p_list);
string_list nk_get_path_list(uint32_t ptr);

typedef struct {
	uint32_t count_childs;
	uint32_t count_params;
	string class_name;
	uint64_t time_creation;
	uint32_t ptr_self;
} nk_stats;

void nk_stats_free(nk_stats *p_stats);
nk_stats nk_get_stats(uint32_t ptr);

typedef enum {
	DELKEY_MODE_DISABLE,
	DELKEY_MODE_ONLY_DEL,
	DELKEY_MODE_MIX
} DELKEY_MODE;

void delkey_init(DELKEY_MODE mode);

#endif /* REGFILE_H_ */
