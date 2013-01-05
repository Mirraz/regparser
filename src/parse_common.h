#ifndef PARSE_COMMON_H_
#define PARSE_COMMON_H_

#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#error
#endif

#include <stdio.h>
#include <assert.h>

#define assert_check(condition, errnum) assert(condition)
/* some field of out structure (which was correctly identified) has wrong value */
#define assert_check1(condition) assert_check(condition, 1)
/* ptr was wrong and/or structure is not correctly identified */
#define assert_check2(condition) assert_check(condition, 2)

#define check_struct_size(PREFIX) assert(sizeof(PREFIX##_struct) == PREFIX##_struct_size)
#define check_array_size(array, size) assert(sizeof(array)/sizeof(array[0]) == size)

#define enum_desc_item(item) {item, #item}

#endif /* PARSE_COMMON_H_ */
