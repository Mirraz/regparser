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

uint32_t test2(uint32_t ptr) {
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

	return ptr;
}

void test3(uint32_t ptr) {

	ptr = test2(ptr);

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

void test4(uint32_t ptr) {
	ptr = test2(ptr);

	string_list list = nk_get_path_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_print(list.entries[i]);
		printf("\n");
	}
	string_list_free(&list);

}

void param_print(param_value value, uint32_t vk_type) {
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		unsigned int i;
		for (i=0; i<value.hex.size; ++i) {
			printf("%02X ", value.hex.data[i]);
			if (!((i+1)&15)) printf("\n");
		}
		if (i&15) printf("\n");
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		string_print(value.str);
		printf("\n");
		break;
	}
	case REG_DWORD:
	case REG_DWORD_BIG_ENDIAN: {
		printf("0x%08X\n", value.dword);
		break;
	}
	case REG_QWORD: {
		printf("0x%016llX\n", (long long unsigned int)value.qword);
		break;
	}
	case REG_MULTI_SZ: {
		unsigned int i;
		for (i=0; i<value.multi_str.size; ++i) {
			string_print(value.multi_str.entries[i]);
			printf("\n");
		}
		break;
	}
	}
}

void test5(uint32_t ptr) {
	ptr = test2(ptr);

	params_parsed_list list = nk_get_params_parsed_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		uint32_t vk_ptr = list.entries[i].ptr;
		uint32_t vk_type = vk_get_type(vk_ptr);
		param_value value = vk_get_value(vk_ptr);
		param_print(value, vk_type);
		param_value_free(&value, vk_type);
	}
	params_parsed_list_free(&list);

}

int main() {
	uint32_t ptr_root = regfile_init("NTUSER.DAT");

	test5(ptr_root);

	regfile_uninit();
	return 0;
}
