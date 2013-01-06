#ifndef STRING_TYPE_H_
#define STRING_TYPE_H_

#include <stdint.h>

typedef size_t str_len_type;
typedef struct {
	const char *str;	// not '\0' terminated
	str_len_type len;	// bytes (not symbols) count
} string;

void string_free(string str);
string string_new_from_ansi(const unsigned char *in, size_t in_size);
string string_new_from_unicode(const unsigned char *in, size_t in_size);
#ifndef NDEBUG
void string_print(string str);
#endif
int string_compare(string a, string b);

#endif /* STRING_TYPE_H_ */
