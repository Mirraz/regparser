#include <assert.h>
#include <stdio.h>
#include "regfile.h"

void test1(uint32_t ptr) {
	string_and_ptr_list list = nk_get_childs_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_print(list.entries[i].str); printf(" %08X\n", list.entries[i].ptr);
	}
	string_and_ptr_list_free(&list);
}

void test2(uint32_t ptr) {
	string child_name;

	child_name.str = "Software"; child_name.len = 8;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "Microsoft"; child_name.len = 9;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "Windows"; child_name.len = 7;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "CurrentVersion"; child_name.len = 14;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "Explorer"; child_name.len = 8;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	params_parsed_list list = nk_get_params_parsed_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		param_parsed val = list.entries[i];
		string_print(val.name); printf("\n");
		string_print(val.type); printf("\n");
		printf("%u\n", val.size_value);
		string_print(val.value_brief); printf("\n");
		printf("%08X\n", val.ptr);
		printf("===========\n");
	}
	params_parsed_list_free(&list);
}

int main() {
	uint32_t ptr_root = regfile_init("NTUSER.DAT");

	test2(ptr_root);

	regfile_uninit();
	return 0;
}
