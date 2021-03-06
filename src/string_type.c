#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <iconv.h>
#include "string_type.h"

void string_free(string *p_str) {
	assert((p_str->len == 0) == (p_str->str == NULL));
	free((char *)p_str->str);
	p_str->str = NULL;
	p_str->len = 0;
}

string string_new_from_ansi(const uint8_t *in, size_t in_count) {
	char *res_str;
	if (in_count != 0) {
		res_str = malloc(in_count);
		memcpy(res_str, in, in_count);
	} else {
		res_str = NULL;
	}
	string res = {.str = (const char *)res_str, .len = in_count};
	return res;
}

string string_new_from_unicode(const uint16_t *in, size_t in_count) {
	char *res_str = NULL;
	size_t res_size = 0;
	if (in_count == 0) goto err;

	iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
	if (cd == (iconv_t)(-1)) goto err;

	size_t buf_size = in_count * 4;			// ???
	char *buf = malloc(buf_size);
	if (buf == NULL) goto err2;

	size_t in_size = in_count * 2;
	size_t out_size = buf_size;
	char *out = buf;
	size_t r = iconv(cd, (char **)&in, &in_size, &out, &out_size);
	if (r == (size_t)-1 || in_size != 0) goto err3;
	res_size = buf_size - out_size;

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
void string_fprint(FILE *fout, string str) {
	fprintf(fout, "%u[%.*s]", (unsigned int)str.len, (int)str.len, str.str);
}
void string_print(string str) {
	string_fprint(stdout, str);
}

#endif

int string_compare(string a, string b) {
	int res = strncmp(a.str, b.str, (a.len < b.len ? a.len : b.len));
	return (res != 0 ? res : ((int)a.len - (int)b.len));
}

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
	printf("%u\n", (unsigned int)sizeof(in));
	string str = string_new_from_unicode((uint16_t *)in, sizeof(in)/2);
	printf("%u[%.*s]\n", (unsigned int)str.len, (int)str.len, str.str);
	string_free(&str);
	return 0;
}
*/
