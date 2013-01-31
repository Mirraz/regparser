#ifndef STRING_TYPE_H_
#define STRING_TYPE_H_

#include <stdint.h>

typedef size_t str_len_type;
typedef struct {
	const char *str;	// not '\0' terminated
	str_len_type len;	// bytes (not symbols) count
} string;

void string_free(string *p_str);
string string_new_from_ansi(const uint8_t *in, size_t in_count);
// in_count - count of uint16_t elements
string string_new_from_unicode(const uint16_t *in, size_t in_count);
#ifndef NDEBUG
#include <stdio.h>
void string_fprint(FILE *fout, string str);
void string_print(string str);
#endif
int string_compare(string a, string b);

#endif /* STRING_TYPE_H_ */
