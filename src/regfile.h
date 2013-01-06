#ifndef REGFILE_H_
#define REGFILE_H_

#include <stdint.h>
#include "string_type.h"

#define ptr_is_null(ptr) ((ptr) == (uint32_t)-1)
#define ptr_not_null(ptr) ((ptr) != (uint32_t)-1)
#define ptr_null ((uint32_t)-1)

uint32_t regfile_init(const char *regfile_path);
int regfile_uninit();

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
uint32_t nk_find_child(uint32_t ptr, string name);
string_and_ptr_list nk_get_params_names_list(uint32_t ptr);

#endif /* REGFILE_H_ */
