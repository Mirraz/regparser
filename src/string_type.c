#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <iconv.h>
#include "string_type.h"

void string_free(string str) {
	free((char *)str.str);
}

string string_new_from_ansi(const char *in, unsigned int in_size) {
	char *res_str = malloc(in_size);
	memcpy(res_str, in, in_size);
	string res = {.str = (const char *)res_str, .len = in_size};
	return res;
}

string string_new_from_unicode(const unsigned char *in, size_t in_size) {
	char *res_str = NULL;
	iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
	if (cd == (iconv_t)(-1)) goto err;

	size_t buf_size = in_size * 2;			// ???
	char *buf = malloc(buf_size);
	if (buf == NULL) goto err2;

	size_t out_size = buf_size;
	char *out = buf;
	size_t r = iconv(cd, (char **)&in, &in_size, &out, &out_size);
	if (r < 0 || in_size != 0) goto err3;
	size_t res_size = buf_size - out_size;

	res_str = malloc(res_size);
	if (res_str == NULL) goto err3;
	memcpy(res_str, buf, res_size);

err3:
	free(buf);
err2:
	iconv_close(cd);
err:
	;
	string res = {.str = (const char *)res_str, .len = res_size};
	return res;
}

#ifndef NDEBUG
#include <stdio.h>
void string_print(string str) {
	printf("%u[%.*s]", (unsigned int)str.len, (int)str.len, str.str);
}
#endif

int string_compare(string a, string b) {
	int res = strncmp(a.str, b.str, (a.len < b.len ? a.len : b.len));
	return (res != 0 ? res : ((int)a.len - (int)b.len));
}

/* ********************************** */

string_and_ptr_list list_new(unsigned int size) {
	string_and_ptr_list res;
	res.size = size;
	if (size != 0) {
		res.entries = malloc(res.size * sizeof(res.entries[0]));
		assert(res.entries != NULL);
	} else {
		res.entries = NULL;
	}
	return res;
}

void list_free(string_and_ptr_list *p_list) {
	string_and_ptr_list list = *p_list;
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_free(list.entries[i].str);
	}
	free(list.entries);
	p_list->entries = NULL;
	p_list->size = 0;
}

/* ********************************** */

/*
#include <stdio.h>

int main() {
	unsigned char in[] = {
		0xc9, 0x45, 0x10, 0x46, 0xc6, 0x4b, 0x95, 0x4b, 0x34, 0xd8, 0x3d, 0xdc,
		0x34, 0xd8, 0x15, 0xdc, 0xb2, 0x25, 0xdb, 0x25, 0xc9, 0x25, 0x1c, 0x2c,
		0x19, 0x2c, 0x00, 0xd8, 0x45, 0xdf, 0x00, 0xd8, 0x41, 0xdf, 0x64, 0x27,
		0x23, 0x27, 0x8a, 0x27, 0xad, 0x27, 0xbb, 0x03, 0x14, 0x2d, 0xc5, 0x10,
		0x33, 0x25, 0x3c, 0xd8, 0x61, 0xdc, 0xd8, 0x2e, 0x3c, 0xd8, 0xce, 0xdc,
		0xa0, 0x2c, 0xaf, 0x0e, 0xcb, 0x01, 0xda, 0x00, 0x3c, 0xd8, 0x10, 0xdc,
		0x24, 0x0d, 0x35, 0xd8, 0x7d, 0xdd, 0x35, 0xd8, 0xe6, 0xdc, 0x35, 0xd8,
		0x3d, 0xdd, 0x35, 0xd8, 0xb2, 0xde, 0x35, 0xd8, 0xfb, 0xde, 0x35, 0xd8,
		0x27, 0xdf, 0x35, 0xd8, 0x0a, 0xdf, 0x35, 0xd8, 0x85, 0xdf, 0x35, 0xd8,
		0xb3, 0xdf, 0x35, 0xd8, 0xde, 0xdf, 0x35, 0xd8, 0x9d, 0xdf, 0x30, 0x22,
		0xa0, 0x22, 0x9b, 0x22, 0x34, 0xd8, 0x91, 0xdd, 0x34, 0xd8, 0x22, 0xdd,
		0x34, 0xd8, 0x20, 0xdd, 0x34, 0xd8, 0x2b, 0xdd
	};
	printf("%d\n", sizeof(in));
	string str = string_new_from_unicode(in, sizeof(in));
	printf("%u[%.*s]\n", (unsigned int)str.len, (int)str.len, str.str);
	string_free(str);
	return 0;
}
*/
