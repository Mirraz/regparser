#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "regfile.h"

FILE *flog;

void print_childs(uint32_t ptr) {
	string_and_ptr_list list = nk_get_childs_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_print(list.entries[i].str); printf(" %08X\n", list.entries[i].ptr);
	}
	string_and_ptr_list_free(&list);
}

void test1() {
	uint32_t ptr = regfile_init("NTUSER.DAT");

	print_childs(ptr);

	regfile_uninit();
}

void test2_1(uint32_t ptr) {
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

void test2_2(uint32_t ptr) {
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

void test2_3(uint32_t ptr) {
	params_parsed_list list = nk_get_params_parsed_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		uint32_t vk_ptr = list.entries[i].ptr;
		param_parsed_full param = vk_get_parsed(vk_ptr);
		param_print(param.value, param.type);
		param_parsed_full_free(&param);
	}
	params_parsed_list_free(&list);
}

void test2() {
	uint32_t ptr = regfile_init("NTUSER.DAT");

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

	test2_3(ptr);

	regfile_uninit();
}

void print_params_parsed_full(uint32_t ptr) {
	params_parsed_list list = nk_get_params_parsed_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		param_parsed val = list.entries[i];
		string_print(val.name); printf("\n");
		string_print(val.type); printf("\n");
		printf("%u\n", val.size_value);
		string_print(val.value_brief); printf("\n");
		printf("%08X\n", val.ptr);

		printf("-----------\n");

		uint32_t vk_ptr = list.entries[i].ptr;
		param_parsed_full param = vk_get_parsed(vk_ptr);
		param_print(param.value, param.type);
		param_parsed_full_free(&param);

		printf("===========\n");
	}
	params_parsed_list_free(&list);
}

void test3() {
	uint32_t ptr = regfile_init("../reg_data_files/config/system");

	string child_name;

	child_name.str = "ControlSet001"; child_name.len = 13;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "Enum"; child_name.len = 4;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "ACPI"; child_name.len = 4;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "ACPI0003"; child_name.len = 8;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	child_name.str = "1"; child_name.len = 1;
	ptr = nk_find_child(ptr, child_name);
	assert(ptr_not_null(ptr));

	print_params_parsed_full(ptr);

	regfile_uninit();
}

void print_path(uint32_t ptr) {
	string_list path_list = nk_get_path_list(ptr);
	unsigned int j;
	for (j=0; j<path_list.size; ++j) {
		string str = path_list.entries[j];
		printf("%.*s", (unsigned int)str.len, str.str);
		if (j != path_list.size-1) printf("/");
	}
	string_list_free(&path_list);
}

void print_stats(uint32_t ptr) {
	nk_stats stats = nk_get_stats(ptr);

	printf("childs count: %u\n", stats.count_childs);
	printf("params count: %u\n", stats.count_params);
	printf("class name: "); string_print(stats.class_name); printf("\n");
	printf("creation time: "); string_print(stats.time_creation); printf("\n");
	printf("self ptr: %08X\n", stats.ptr_self);

	nk_stats_free(&stats);
}

void test_recur(uint32_t ptr) {
	string_and_ptr_list list = nk_get_childs_list(ptr);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_and_ptr child = list.entries[i];
		string_print(child.str); printf("\n");

		print_path(child.ptr); printf("\n");

		print_stats(child.ptr);

		printf("____________________________\n");
		print_params_parsed_full(child.ptr);
		printf("############################\n");

		test_recur(child.ptr);
	}
	string_and_ptr_list_free(&list);
}

void test_recut_start(int argc, char **argv) {
	assert(argc == 2);

	uint32_t ptr_root_nk = regfile_init(argv[1]);
	if (ptr_root_nk == ptr_null) return;

	test_recur(ptr_root_nk);

	regfile_uninit();
}

uint32_t change_path(uint32_t ptr, const char *path) {
	do {
		char *end = strchrnul(path, '/');
		string item = {.str = path, .len = end - path};
		if (item.len != 0) {

			ptr = nk_find_child(ptr, item);
			if (ptr == ptr_null) return ptr_null;

		}
		if (*end == '\0') break;
		path = end + 1;
	} while(1);
	return ptr;
}

void test_key_path(int argc, char **argv) {
	assert(argc == 3);

	uint32_t ptr = regfile_init(argv[1]);
	if (ptr == ptr_null) return;

	ptr = change_path(ptr, argv[2]);
	if (ptr == ptr_null) {printf("keypath not found\n"); return;}

	print_path(ptr); printf("\n");

	print_stats(ptr);

	printf("~~~~~~~~~~\n");
	print_childs(ptr);
	printf("~~~~~~~~~~\n");
	print_params_parsed_full(ptr);

	regfile_uninit();

}

void test_key_path_fprint_reg(int argc, char **argv) {
	assert(argc == 3);

	uint32_t ptr = regfile_init(argv[1]);
	if (ptr == ptr_null) return;

	ptr = change_path(ptr, argv[2]);
	if (ptr == ptr_null) {printf("keypath not found\n"); return;}

	fprint_reg_header(stderr);
	nk_fprint_reg(stderr, ptr);

	regfile_uninit();
}

int main(int argc, char **argv) {
flog = fopen("/tmp/debug.log", "w");

	test_key_path_fprint_reg(argc, argv);
	//test_key_path(argc, argv);
	//test_recut_start(argc, argv);

	fprintf(stdout, "Success\n");

fclose(flog);
	return 0;
}
