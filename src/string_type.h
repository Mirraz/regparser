#ifndef STRING_TYPE_H_
#define STRING_TYPE_H_

#include <stdint.h>

typedef size_t str_len_type;
typedef struct {
	const char *str;	// not '\0' terminated
	str_len_type len;	// bytes (not symbols) count
} string;

void string_free(string str);
string string_new_from_ansi(const char *in, unsigned int in_size);
string string_new_from_unicode(const unsigned char *in, size_t in_size);
#ifndef NDEBUG
void string_print(string str);
#endif
int string_compare(string a, string b);

/* ********************************** */

typedef struct {
	string str;
	uint32_t ptr;
} string_and_ptr;

typedef struct {
	string_and_ptr *entries;
	unsigned int size;
} string_and_ptr_list;

string_and_ptr_list list_new(unsigned int size);
void list_free(string_and_ptr_list *p_list);

#endif /* STRING_TYPE_H_ */
