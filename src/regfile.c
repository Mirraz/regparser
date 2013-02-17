#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"
#include "debug.h"
#include "string_type.h"
#include "rbtree.h"
#include "childmap.h"
#include "parse_common.h"
#include "regfile_declare.h"
#include "regfile.h"

/* ********************************** */

#define check_signatire(PREFIX) (s->signature == PREFIX##_signature)
#define check_block_ptr() (ptr_not_null(ptr) && ptr < header_data.size_data_area)
#define check_ptr(ptr) (ptr_is_null(ptr) || ptr < header_data.size_data_area)
#define check_size(size) (size < header_data.size_data_area)
#define check_block_size() (abs(s->size)+ptr <= header_data.size_data_area)

static const param_type_desc_struct param_type_desc[] = param_type_desc_value;

int fd = -1;
static regf_struct header_data;
regf_struct *header = &header_data;
uint8_t *data = NULL;

unsigned int vk_value_brief_max_len = 128;

DELKEY_MODE delkey_mode = DELKEY_MODE_DISABLE;
childmap *delkeytree = NULL;

/* ********************************** */

void structs_check_size();
regf_struct *regf_init(regf_struct *s);

uint32_t regfile_init(const char *regfile_path) {
	structs_check_size();

	fd = open(regfile_path, O_RDONLY);
	if (fd < 0) {perror("open"); return ptr_null;}

	ssize_t red = read(fd, header, regf_struct_size);
	if (red < 0) {perror("read"); return ptr_null;}
	assert(red == regf_struct_size);
	regf_init(header);

	data = mmap(NULL, header->size_data_area,
			PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, regf_header_size);
	if (data == MAP_FAILED) {perror("mmap"); return ptr_null;}

	return header->ptr_root_nk;
}

int regfile_uninit() {
	int res1 = munmap(data, header->size_data_area);
	int res2 = close(fd);
	return res1 || res2;
}

void structs_check_size() {
	check_struct_size(regf);
	check_struct_size(hbin);

	check_struct_size(nk);
	check_struct_size(vk);
	check_struct_size(sk);
	check_struct_size(lf);
	check_struct_size(lh);
	check_struct_size(li);
	check_struct_size(ri);
	check_struct_size(db);

	check_struct_size(value);
	check_struct_size(index);

	check_struct_size(signature);
	
	check_array_size(param_type_desc, param_types_count+1);
	
	//secstructs_check_size();
}


/* ********************************** */

regf_struct *regf_init(regf_struct *s) {
	assert(s != NULL);
	assert_check2(check_signatire(regf));

	assert_check1(s->stuff1 == 1);
	assert_check1(s->subversion == 0);
	assert_check1(s->stuff2 == 1);
	assert_check1(s->stuff3 >= 1 && s->stuff3 <= 8);
	// TODO: check checksum

	assert_check1(check_ptr(s->ptr_root_nk));

	return s;
}

/* ========= */

hbin_struct *hbin_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	hbin_struct *s = (hbin_struct *)(data + ptr);
	assert_check2(check_signatire(hbin));

	assert_check1(ptr == s->ptr_self);
	assert_check1(check_size(s->size_segment));
	assert_check1(s->stuff1 == 0);
	assert_check1(s->stuff2 == 0);

	return s;
}

/* ========= */

nk_struct *nk_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	nk_struct *s = (nk_struct *)(data + ptr);
	assert_check2(check_signatire(nk));
	assert_check1(check_block_size());

	assert_check1(nk_struct_size + s->size_key_name <= abs(s->size));
	// if key_name is in unicode, then size_key_name must be even
	assert_check1( (s->flag & 0x20) || (s->size_key_name & 1) == 0);
	assert_check1(check_size(s->size_key_class));

	assert_check1(check_ptr(s->ptr_parent));
	assert_check1(check_ptr(s->ptr_childs_index));
	assert_check1(check_ptr(s->ptr_params_index));
	assert_check1(check_ptr(s->ptr_sk));
	assert_check1(check_ptr(s->ptr_class_name));

	return s;
}

nk_struct *nk_init_check(uint32_t ptr) {
	assert_check2(check_block_ptr());
	nk_struct *s = (nk_struct *)(data + ptr);
	if (s->size < 0) return nk_init(ptr);

	if (!check_signatire(nk)) return  NULL;
	if (!check_block_size()) return NULL;

	if (!( nk_struct_size + s->size_key_name <= abs(s->size) )) return NULL;
	// if key_name is in unicode, then size_key_name must be even
	if (!( (s->flag & 0x20) || (s->size_key_name & 1) == 0 )) return NULL;
	if (!check_size(s->size_key_class)) return NULL;

	if (!check_ptr(s->ptr_parent)) return NULL;
	if (!check_ptr(s->ptr_childs_index)) return NULL;
	if (!check_ptr(s->ptr_params_index)) return NULL;
	if (!check_ptr(s->ptr_sk)) return NULL;
	if (!check_ptr(s->ptr_class_name)) return NULL;

	return s;
}

vk_struct *vk_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	vk_struct *s = (vk_struct *)(data + ptr);
	assert_check2(check_signatire(vk));
	assert_check1(check_block_size());

	assert_check1(vk_struct_size + s->size_param_name <= abs(s->size));
	if (!(s->size_param_value & 0x80000000)) {
		assert_check1(check_size(s->size_param_value));
		assert_check1(check_ptr(s->ptr_param_value));
	} else {
		assert_check1((s->size_param_value & ~0x80000000) <= 4);
	}
	/* in SAM regfile in type may be sid user number */
	/*assert_check1(s->param_type < param_types_count);*/

	return s;
}

sk_struct *sk_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	sk_struct *s = (sk_struct *)(data + ptr);
	assert_check2(check_signatire(sk));
	assert_check1(check_block_size());

	assert_check1(check_ptr(s->ptr_prev_sk));
	assert_check1(check_ptr(s->ptr_next_sk));
	assert_check1(sk_struct_size + s->size_data <= abs(s->size));

	return s;
}

lf_struct *lf_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	lf_struct *s = (lf_struct *)(data + ptr);
	assert_check2(check_signatire(lf));
	assert_check1(check_block_size());

	assert_check1(lf_struct_size + s->count_records * sizeof(s->records[0]) <=
			abs(s->size));
	unsigned int i;
	for (i=0; i<s->count_records; ++i)
		assert_check1(check_ptr(s->records[i].ptr_nk));

	return s;
}

lh_struct *lh_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	lh_struct *s = (lh_struct *)(data + ptr);
	assert_check2(check_signatire(lh));
	assert_check1(check_block_size());

	assert_check1(lh_struct_size + s->count_records * sizeof(s->records[0]) <=
			abs(s->size));
	unsigned int i;
	for (i=0; i<s->count_records; ++i)
		assert_check1(check_ptr(s->records[i].ptr_nk));

	return s;
}

ri_struct *ri_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	ri_struct *s = (ri_struct *)(data + ptr);
	assert_check2(check_signatire(ri));
	assert_check1(check_block_size());

	assert_check1(ri_struct_size + s->count_records * sizeof(s->ptr_indexes[0]) <=
			abs(s->size));
	unsigned int i;
	for (i=0; i<s->count_records; ++i)
		assert_check1(check_ptr(s->ptr_indexes[i]));

	return s;
}

db_struct *db_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	db_struct *s = (db_struct *)(data + ptr);
	assert_check2(check_signatire(db));
	assert_check1(check_block_size());
	
	assert_check1(check_ptr(s->ptr_value_parts_index));
	
	return s;
}

/* ========= */

value_struct *value_init(uint32_t ptr, unsigned int size_value) {
	assert_check2(check_block_ptr());
	value_struct *s = (value_struct *)(data + ptr);
	assert_check2(check_block_size());

	assert_check2(size_value <= abs(s->size));
	assert_check2((s->size & 1) == 0);

	return s;
}

index_struct *index_init(uint32_t ptr, unsigned int count_records) {
	assert_check2(check_block_ptr());
	index_struct *s = (index_struct *)(data + ptr);
	assert_check2(check_block_size());

	assert_check2(count_records * 4 + index_struct_size <= abs(s->size));

	unsigned int i;
	for (i=0; i<count_records; ++i) {
		assert_check1(check_ptr(s->ptr_blocks[i]));
	}

	return s;
}

signature_struct *signature_init(uint32_t ptr) {
	assert_check2(check_block_ptr());
	signature_struct *s = (signature_struct *)(data + ptr);
	assert_check2(check_block_size());

	return s;
}

/* ********************************** */

string nk_get_name(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	string name;
	if (s->flag & 0x20) {
		// ansi
		name = string_new_from_ansi(s->key_name, s->size_key_name);
	} else {
		// unicode
		name = string_new_from_unicode((uint16_t *)s->key_name, s->size_key_name >> 1);
	}
	return name;
}

string nk_get_class(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	if (s->ptr_class_name != ptr_null) {
		value_struct *class_val = value_init(s->ptr_class_name, s->size_key_class);
		string res = string_new_from_unicode((uint16_t *)class_val->value, s->size_key_class >> 1);
		return res;
	} else {
		string res = {.str = NULL, .len = 0};
		return res;
	}
}

int nk_childs_index_process(uint32_t ptr_chinds_index,
			int (*callback)(uint32_t, void *), void *callback_data) {
	assert(ptr_not_null(ptr_chinds_index));
	signature_struct *sig = signature_init(ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature: {
		lf_struct *lf = lf_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lf->count_records; ++i) {
			if (callback(lf->records[i].ptr_nk, callback_data)) return 1;
		}
		break;
	}
	case lh_signature: {
		lh_struct *lh = lh_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lh->count_records; ++i) {
			if (callback(lh->records[i].ptr_nk, callback_data)) return 1;
		}
		break;
	}
	case li_signature: {
		// TODO
		//li_struct *li1 = (li_struct *)(data + ptr_chinds_index);
		break;
	}
	case ri_signature: {
		ri_struct *ri = ri_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<ri->count_records; ++i) {
			if (nk_childs_index_process(ri->ptr_indexes[i], callback, callback_data)) return 1;
		}
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
	return 0;
}

int delkey_nk_childs_process(uint32_t ptr,
			int (*callback)(uint32_t, void *), void *callback_data);

int nk_childs_process(uint32_t ptr,
		int (*callback)(uint32_t, void *), void *callback_data) {
	nk_struct *nk = nk_init(ptr);
	if (ptr_is_null(nk->ptr_childs_index)) return 1;
	if (delkey_mode != DELKEY_MODE_ONLY_DEL) {
		if (nk_childs_index_process(nk->ptr_childs_index, callback, callback_data)) return 1;
	}
	if (delkey_mode != DELKEY_MODE_DISABLE) {
		if (delkey_nk_childs_process(ptr, callback, callback_data)) return 1;
	}
	return 0;
}

int child_print_name(uint32_t ptr, void *data) {
	(void)data;
	string name = nk_get_name(ptr);
	string_print(name);
	string_free(&name);
	return 0;
}

int child_set_add(uint32_t ptr, void *data) {
	rbtree **p_the_tree = (rbtree **)data;
	string_and_ptr val = {.str = nk_get_name(ptr), .ptr = ptr};
	rbtree *node = sglib_rbtree_node_new(val);
	rbtree *member = NULL;
	int res = sglib_rbtree_add_if_not_member(p_the_tree, node, &member);
	assert(res != 0 && member == NULL);		// isn't childs with duplicate names
	return 0;
}

string_and_ptr_list string_and_ptr_list_new(unsigned int size) {
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

void string_and_ptr_list_free(string_and_ptr_list *p_list) {
	assert(p_list != NULL);
	assert((p_list->size == 0) == (p_list->entries == NULL));
	unsigned int i;
	for (i=0; i<p_list->size; ++i) {
		string_free(&p_list->entries[i].str);
	}
	free(p_list->entries);
	p_list->entries = NULL;
	p_list->size = 0;
}

string_and_ptr_list nk_get_childs_list(uint32_t ptr) {
	rbtree *the_tree = NULL;
	nk_childs_process(ptr, child_set_add, &the_tree);

	string_and_ptr_list res = string_and_ptr_list_new(sglib_rbtree_len(the_tree));

	unsigned int idx;
	struct sglib_rbtree_iterator it;
	rbtree *te;
	for(
			te=sglib_rbtree_it_init_inorder(&it,the_tree), idx = 0;
			te!=NULL;
			te=sglib_rbtree_it_next(&it), ++idx
	) {
		res.entries[idx] = te->val;
	}
	assert(idx == res.size);

	sglib_rbtree_free(&the_tree);

	nk_struct *nk = nk_init(ptr);
	assert(res.size == nk->count_childs);

	return res;
}

int childs_find(uint32_t ptr, void *data) {
	string_and_ptr *find_child = (string_and_ptr *)data;
	string cur_name = nk_get_name(ptr);
	int res = string_compare(cur_name, find_child->str);
	if (res == 0) {
		find_child->ptr = ptr;
	}
	string_free(&cur_name);
	return !res;
}

uint32_t nk_find_child(uint32_t ptr, string name) {
	string_and_ptr find = {.str = name, .ptr = ptr_null};
	nk_childs_process(ptr, childs_find, &find);
	return find.ptr;
}

uint32_t nk_get_parent(uint32_t ptr) {
	if (header->ptr_root_nk == ptr) return ptr_null;
	nk_struct *nk = nk_init(ptr);
	return nk->ptr_parent;
}

string vk_get_name(uint32_t ptr) {
	vk_struct *s = vk_init(ptr);
	if (s->size_param_name == 0) {
		string name = {.str = NULL, .len = 0};
		return name;
	} else {
		string name;
		if (s->flag & 1) {
			// ansi
			name = string_new_from_ansi(s->param_name, s->size_param_name);
		} else {
			// unicode
			name = string_new_from_unicode((uint16_t *)s->param_name, s->size_param_name >> 1);
		}
		return name;
	}
}

string vk_get_type_str(uint32_t ptr) {
	vk_struct *vk = vk_init(ptr);
	unsigned int j;
	for (j=0; j<param_types_count && param_type_desc[j].type != vk->param_type; ++j);
	string res;
	res.len = strlen(param_type_desc[j].name);
	if (j < param_types_count) {
		res.str = strndup(param_type_desc[j].name, res.len);
	} else {
		char str_unknown[] = " (0x00000000)";
		const unsigned int str_unknown_size = 13;
		res.str = malloc(res.len + str_unknown_size);
		strncpy((char *)res.str, param_type_desc[j].name, res.len);
		snprintf(str_unknown+4, 9, "%08X", vk->param_type);
		str_unknown[str_unknown_size-1] = ')';
		strncpy((char *)res.str+res.len, str_unknown, str_unknown_size);
		res.len += str_unknown_size;
	}
	return res;
}

/* ****************** */

string param_get_value_brief(uint8_t *value_data, uint32_t value_size, uint32_t vk_type) {
	string res = {.len = 0, .str = NULL};
	if (value_size == 0) return res;
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		// +1 for '\0' in snprintf()
		char *brief = malloc(vk_value_brief_max_len + 1);
		assert(brief != NULL);
		char *brief_cur = brief;
		unsigned int brief_rest = vk_value_brief_max_len;
		unsigned int i;
		for (i=0; i<value_size; ++i) {
			if (brief_rest < 2) break;
			// +1 for '\0'
			snprintf(brief_cur, 2+1, "%02X", value_data[i]);
			brief_cur += 2;
			brief_rest -= 2;
			if (i != value_size-1) {
				if (brief_rest < 3) break;
				*brief_cur = ' ';
				brief_cur += 1;
				brief_rest -= 1;
			}
		}
		assert(brief_cur - brief == vk_value_brief_max_len - brief_rest);
		res.len = brief_cur - brief;
		if (res.len != 0) {
			res.str = malloc(res.len);
			assert(res.str != NULL);
			memcpy((char *)res.str, brief, res.len);
		} else {
			res.str = NULL;
		}
		free(brief);
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		// in "default" regfile may be "?? ?? .. .. ?? ?? 00 00 ??"
		//assert(!(value_size & 1));
		uint16_t *utf16_data = (uint16_t *)value_data;
		unsigned int utf16_data_size = value_size >> 1;
		// in "system", "software" regfiles may not be
		if (utf16_data_size > 0 && utf16_data[utf16_data_size-1] == 0) --utf16_data_size;
		res = string_new_from_unicode(utf16_data,
				MIN(utf16_data_size, vk_value_brief_max_len >> 1));
		break;
	}
	case REG_DWORD: {
		assert(value_size == 4);
		assert(vk_value_brief_max_len >= 2+8);
		// +1 for '\0'
		char brief[2+8+1] = "0x00000000";
		// +1 for '\0'
		snprintf(brief+2, 8+1, "%08X", *((uint32_t *)value_data));
		res.len = 2+8;
		res.str = malloc(res.len);
		assert(res.str != NULL);
		memcpy((char *)res.str, brief, res.len);
		break;
	}
	case REG_DWORD_BIG_ENDIAN: {
		assert(value_size == 4);
		assert(vk_value_brief_max_len >= 2+8);
		// +1 for '\0'
		char brief[2+8+1] = "0x00000000";
		// +1 for '\0'
		snprintf(brief+2, 8+1, "%08X", be32toh(*((uint32_t *)value_data)));
		res.len = 2+8;
		res.str = malloc(res.len);
		assert(res.str != NULL);
		memcpy((char *)res.str, brief, res.len);
		break;
	}
	case REG_QWORD: {
		assert(value_size == 8);
		assert(vk_value_brief_max_len >= 2+16);
		// +1 for '\0'
		char brief[2+16+1] = "0x0000000000000000";
		// +1 for '\0'
		snprintf(brief+2, 16+1, "%016llX", (long long unsigned int)(*((uint64_t *)value_data)));
		res.len = 2+16;
		res.str = malloc(res.len);
		assert(res.str != NULL);
		memcpy((char *)res.str, brief, res.len);
		break;
	}
	case REG_MULTI_SZ: {
		uint16_t *utf16_data = (uint16_t *)value_data;
		assert(!(value_size & 1));
		unsigned int i = 0;
		while (
				i < value_size >> 1 &&
				i < vk_value_brief_max_len &&
				utf16_data[i] != 0
		) ++i;
		if (!(i < value_size >> 1 && i < vk_value_brief_max_len)) --i;
		res = string_new_from_unicode((uint16_t *)value_data, i);
		break;
	}
	}
	return res;
}

typedef struct str_list_entry_ str_list_entry;
struct str_list_entry_ {
	string str;
	str_list_entry *next;
};

param_value param_block_get_value(uint8_t *value_data, uint32_t value_size, uint32_t vk_type) {
	param_value res;
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		if (value_size == 0) {
			res.hex.data = NULL;
			res.hex.size = 0;
			break;
		}
		res.hex.size = value_size;
		res.hex.data = malloc(res.hex.size);
		assert(res.hex.data != NULL);
		memcpy(res.hex.data, value_data, res.hex.size);
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		if (value_size == 0) {
			res.str.str = NULL;
			res.str.len = 0;
			break;
		}
		// in "default" regfile may by "?? ?? .. .. ?? ?? 00 00 ??"
		//assert(!(value_size & 1));
		uint16_t *utf16_data = (uint16_t *)value_data;
		unsigned int utf16_data_size = value_size >> 1;
		// in "system", "software" regfiles may not be
		if (utf16_data_size > 0 && utf16_data[utf16_data_size-1] == 0) --utf16_data_size;
		res.str = string_new_from_unicode(utf16_data, utf16_data_size);
		break;
	}
	case REG_DWORD: {
		if (value_size == 0) {
			res.dword = 0;
			break;
		}
		assert(value_size == 4);
		res.dword = *((uint32_t *)value_data);
		break;
	}
	case REG_DWORD_BIG_ENDIAN: {
		if (value_size == 0) {
			res.dword = 0;
			break;
		}
		assert(value_size == 4);
		res.dword = be32toh(*((uint32_t *)value_data));
		break;
	}
	case REG_QWORD: {
		if (value_size == 0) {
			res.qword = 0;
			break;
		}
		assert(value_size == 8);
		res.qword = *((uint64_t *)value_data);
		break;
	}
	case REG_MULTI_SZ: {
		if (value_size == 0) {
			res.multi_str.entries = NULL;
			res.multi_str.size = 0;
			break;
		}

		str_list_entry *list_first = NULL;
		str_list_entry *list_last = NULL;

		uint16_t *utf16_data = (uint16_t *)value_data;
		uint32_t utf16_rest = value_size >> 1;
		unsigned int entries_count = 0;
		unsigned int i = 0;
		// sometimes loop stops by condition "i < utf16_rest"
		while (i < utf16_rest && utf16_data[i] != 0) {
			utf16_data += i;
			utf16_rest -= i;
			i = 0;
			while (i < utf16_rest && utf16_data[i] != 0) ++i;

			str_list_entry *list_entry = malloc(sizeof(str_list_entry));
			assert(list_entry != NULL);
			list_entry->str = string_new_from_unicode(utf16_data, i);
			list_entry->next = NULL;
			if (list_last == NULL) {
				list_last = list_entry;
				assert(list_first == NULL);
				list_first = list_last;
			} else {
				list_last->next = list_entry;
				list_last = list_entry;
			}

			++entries_count;
			if (i == utf16_rest) break;
			++i;
		}

		res.multi_str.size = entries_count;
		if (res.multi_str.size != 0) {
			res.multi_str.entries = malloc(res.multi_str.size * sizeof(res.multi_str.entries[0]));
			assert(res.multi_str.entries != NULL);

			str_list_entry *list_entry = list_first;
			for (i=0; i<entries_count; ++i) {
				assert(list_entry != NULL);
				res.multi_str.entries[i] = list_entry->str;
				list_entry = list_entry->next;
			}

		} else {
			res.multi_str.entries = NULL;
		}

		// free str_list
		while (list_first != NULL) {
			str_list_entry *list_entry = list_first;
			list_first = list_first->next;
			free(list_entry);
		}

		break;
	}
	}
	return res;
}

typedef struct {
	param_value *entries;
	unsigned int size;
} param_value_list;

param_value param_values_list_merge_and_free(param_value_list list, uint32_t vk_type) {
	assert((list.size != 0) && (list.entries != NULL));
	param_value res;
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		unsigned int i;
		for (i=0; i<list.size; ++i)
			res.hex.size += list.entries[i].hex.size;
		assert(res.hex.size > 0);
		res.hex.data = malloc(res.hex.size);
		assert(res.hex.data != NULL);
		uint8_t *dst = res.hex.data;
		for (i=0; i<list.size; ++i) {
			memcpy(dst, list.entries[i].hex.data, list.entries[i].hex.size);
			dst += list.entries[i].hex.size;
			assert(list.entries[i].hex.size > 0 || list.entries[i].hex.data == NULL);
			free(list.entries[i].hex.data);
		}
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		unsigned int i;
		for (i=0; i<list.size; ++i)
			res.str.len += list.entries[i].str.len;
		assert(res.str.len > 0);
		res.str.str = malloc(res.str.len);
		assert(res.str.str != NULL);
		char *dst = (char *)res.str.str;
		for (i=0; i<list.size; ++i) {
			memcpy(dst, list.entries[i].str.str, list.entries[i].str.len);
			dst += list.entries[i].str.len;
			string_free(&list.entries[i].str);
		}
		break;
	}
	case REG_DWORD:
	case REG_DWORD_BIG_ENDIAN:
	case REG_QWORD:
		assert(0);
		break;
	case REG_MULTI_SZ: {
		unsigned int i;
		for (i=0; i<list.size; ++i)
			res.multi_str.size += list.entries[i].multi_str.size;
		assert(res.multi_str.size > 0);
		res.multi_str.entries = malloc(res.multi_str.size);
		assert(res.multi_str.entries != NULL);
		unsigned int j, idx = 0;
		for (i=0; i<list.size; ++i) {
			for (j=0; j<list.entries[i].multi_str.size; ++j) {
				// not deep coping => not free
				res.multi_str.entries[idx++] = list.entries[i].multi_str.entries[j];
			}
		}
		break;
	}
	}
	free(list.entries);
	return res;
}

void param_value_free(param_value *p_value, uint32_t vk_type) {
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		assert((p_value->hex.size == 0) == (p_value->hex.data == NULL));
		free(p_value->hex.data);
		p_value->hex.data = NULL;
		p_value->hex.size = 0;
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		string_free(&p_value->str);
		break;
	}
	case REG_DWORD:
	case REG_DWORD_BIG_ENDIAN:
	case REG_QWORD:
		break;
	case REG_MULTI_SZ: {
		assert((p_value->multi_str.size == 0) == (p_value->multi_str.entries == NULL));
		unsigned int i;
		for (i=0; i<p_value->multi_str.size; ++i)
			string_free(&p_value->multi_str.entries[i]);
		free(p_value->multi_str.entries);
		p_value->multi_str.entries = NULL;
		p_value->multi_str.size = 0;
		break;
	}
	}
}

string vk_get_value_brief(uint32_t ptr) {
	vk_struct *s = vk_init(ptr);
	//fprintf(fout, "size = %d\n", s->size_param_value & ~0x80000000);
	if (!(s->size_param_value & 0x80000000)) {
		value_struct *block = (value_struct *)(data + s->ptr_param_value);
		// this condition doesn't work
		//if (s->size_param_value <= PARAM_PART_MAX)
		if (abs(block->size) >= s->size_param_value) {
			value_struct *param_value =
					value_init(s->ptr_param_value, s->size_param_value);
			return param_get_value_brief(param_value->value, s->size_param_value, s->param_type);
		} else {
			// db (in "system" regfile)
			db_struct *db = db_init(s->ptr_param_value);
			index_struct *part_index = index_init(db->ptr_value_parts_index, db->count_records);
			// only first block for brief
			uint32_t part_size = MIN(s->size_param_value, PARAM_PART_MAX);
			value_struct *part = value_init(part_index->ptr_blocks[0], part_size);
			return param_get_value_brief(part->value, part_size, s->param_type);
		}
	} else {
		return param_get_value_brief(
				(uint8_t *)&(s->ptr_param_value),
				s->size_param_value & ~0x80000000,
				s->param_type
		);
	}
}

param_value vk_get_value(uint32_t ptr) {
	vk_struct *s = vk_init(ptr);
	//fprintf(fout, "size = %d\n", s->size_param_value & ~0x80000000);
	if (!(s->size_param_value & 0x80000000)) {
		value_struct *block = (value_struct *)(data + s->ptr_param_value);
		// this condition doesn't work
		//if (s->size_param_value <= PARAM_PART_MAX)
		if (abs(block->size) >= s->size_param_value) {
			value_struct *param_value =
					value_init(s->ptr_param_value, s->size_param_value);
			return param_block_get_value(param_value->value, s->size_param_value, s->param_type);
		} else {
			// db (in "system" regfile)
			db_struct *db = db_init(s->ptr_param_value);
			index_struct *part_index = index_init(db->ptr_value_parts_index, db->count_records);

			param_value_list list = {.size = db->count_records};
			assert(list.size != 0);
			list.entries = malloc(list.size * sizeof(list.entries[0]));
			assert(list.entries != NULL);

			uint32_t rest = s->size_param_value;
			unsigned int i;
			for (i=0; i<db->count_records; ++i) {
				uint32_t part_size = (rest > PARAM_PART_MAX ? PARAM_PART_MAX : rest);
				rest -= part_size;
				value_struct *part = value_init(part_index->ptr_blocks[i], part_size);
				list.entries[i] = param_block_get_value(part->value, part_size, s->param_type);
			}

			return param_values_list_merge_and_free(list, s->param_type);
		}
	} else {
		return param_block_get_value((uint8_t *)&(s->ptr_param_value), s->size_param_value & ~0x80000000, s->param_type);
	}
}

void param_parsed_full_free(param_parsed_full *p_param) {
	assert(p_param != NULL);
	param_parsed_full param = *p_param;
	string_free(&p_param->name);
	string_free(&p_param->type_str);
	param_value_free(&p_param->value, param.type);
	p_param->type = param_types_count;
}

param_parsed_full vk_get_parsed(uint32_t ptr) {
	param_parsed_full res;
	vk_struct *vk = vk_init(ptr);
	res.name = vk_get_name(ptr);
	res.type = vk->param_type;
	res.type_str = vk_get_type_str(ptr);
	res.size_value = vk->size_param_value & ~0x80000000;
	res.value = vk_get_value(ptr);
	return res;
}

/* ****************** */

string_and_ptr_list nk_get_params_names_list(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	if (ptr_is_null(s->ptr_params_index)) {
		return string_and_ptr_list_new(0);
	}

	unsigned int count_index_records = s->count_params;
	index_struct *index_params =
			index_init(s->ptr_params_index, count_index_records);
	string_and_ptr_list list = string_and_ptr_list_new(count_index_records);

	unsigned int i;
	for (i=0; i<count_index_records; ++i) {
		uint32_t ptr_vk = index_params->ptr_blocks[i];
		string_and_ptr val = {.str = vk_get_name(ptr_vk), .ptr = ptr_vk};
		list.entries[i] = val;
	}

#define string_and_ptr_cmp(a, b) string_compare(a.str, b.str)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
	SGLIB_ARRAY_SINGLE_HEAP_SORT(string_and_ptr, list.entries, list.size, string_and_ptr_cmp)
#pragma GCC diagnostic pop
#undef string_and_ptr_cmp

	return list;
}

void params_parsed_list_free(params_parsed_list *p_list) {
	assert(p_list != NULL);
	assert((p_list->size == 0) == (p_list->entries == NULL));
	unsigned int i;
	for (i=0; i<p_list->size; ++i) {
		string_free(&p_list->entries[i].name);
		string_free(&p_list->entries[i].type);
		string_free(&p_list->entries[i].value_brief);
	}
	free(p_list->entries);
	p_list->entries = NULL;
	p_list->size = 0;
}

params_parsed_list nk_get_params_parsed_list(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	if (ptr_is_null(s->ptr_params_index)) {
		params_parsed_list list = {.entries = NULL, .size = 0};
		return list;
	}
	if (delkey_mode == DELKEY_MODE_ONLY_DEL && s->size < 0) {
		//don't show params for USED keys
		params_parsed_list list = {.entries = NULL, .size = 0};
		return list;
	}

	unsigned int count_index_records = s->count_params;
	index_struct *index_params =
			index_init(s->ptr_params_index, count_index_records);
	params_parsed_list list = {.size = count_index_records};
	assert (list.size > 0);
	list.entries = malloc(list.size * sizeof(list.entries[0]));
	assert(list.entries != NULL);

	unsigned int i;
	for (i=0; i<count_index_records; ++i) {
		uint32_t ptr_vk = index_params->ptr_blocks[i];
		param_parsed val;
		val.name = vk_get_name(ptr_vk);
		val.type = vk_get_type_str(ptr_vk);
		vk_struct *vk = vk_init(ptr_vk);
		val.size_value = vk->size_param_value & ~0x80000000;
		val.value_brief = vk_get_value_brief(ptr_vk);
		val.ptr = ptr_vk;
		list.entries[i] = val;
	}

#define param_parsed_cmp(a, b) string_compare(a.name, b.name)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
	SGLIB_ARRAY_SINGLE_HEAP_SORT(param_parsed, list.entries, list.size, param_parsed_cmp)
#pragma GCC diagnostic pop
#undef param_parsed_cmp

	return list;
}

/* ****************** */

void string_list_free(string_list *p_list) {
	assert(p_list != NULL);
	assert((p_list->size == 0) == (p_list->entries == NULL));
	unsigned int i;
	for (i=0; i<p_list->size; ++i) {
		string_free(&p_list->entries[i]);
	}
	free(p_list->entries);
	p_list->entries = NULL;
	p_list->size = 0;
}

typedef struct ptr_list_entry_ ptr_list_entry;
struct ptr_list_entry_ {
	uint32_t ptr;
	ptr_list_entry *next;
};

string_list nk_get_path_list(uint32_t ptr) {
	ptr_list_entry *ptr_list = NULL;
	unsigned int entries_count = 0;
	do {
		ptr_list_entry *entry = malloc(sizeof(ptr_list_entry));
		entry->ptr = ptr;
		entry->next = ptr_list;
		ptr_list = entry;
		++entries_count;
		if (header->ptr_root_nk == ptr) break;
		nk_struct *s = nk_init(ptr);
		ptr = s->ptr_parent;
	} while (1);

	string_list list;
	list.size = entries_count;
	if (list.size != 0) {
		list.entries = malloc(list.size * sizeof(list.entries[0]));
		assert(list.entries != NULL);

		ptr_list_entry *entry = ptr_list;
		unsigned int i;
		for (i=0; i<entries_count; ++i) {
			list.entries[i] = nk_get_name(entry->ptr);
			entry = entry->next;
		}
	} else {
		list.entries = NULL;
	}

	// free ptr_list
	while (ptr_list != NULL) {
		ptr_list_entry *entry = ptr_list;
		ptr_list = entry->next;
		free(entry);
	}

	return list;
}

/* ****************** */

#define WIN_TIME_OFFSET ((int64_t)(369 * 365 + 89) * 24 * 3600 * 10000000)
inline time_t win_time_to_utc(int64_t win_time) {
	if (win_time == 0) return 0;
	return (win_time - (WIN_TIME_OFFSET)) / 10000000;
}

string win_time_to_string(uint64_t win_time) {
	time_t time = win_time_to_utc(win_time);
	struct tm *tm = gmtime(&time);
	assert(tm != NULL);
#define str_size 256
	char str[str_size];
	size_t res_len = strftime(str, str_size-1, "%d.%m.%Y %H:%M:%S", tm);
#undef str_size
	assert(res_len != 0);
	string res = string_new_from_ansi((uint8_t *)str, res_len);
	return res;
}

void nk_stats_free(nk_stats *p_stats) {
	p_stats->count_childs = 0;
	p_stats->count_params = 0;
	string_free(&p_stats->class_name);
	string_free(&p_stats->time_creation);
	p_stats->ptr_self = ptr_null;
}

nk_stats nk_get_stats(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	nk_stats res = {
			.count_childs = s->count_childs,
			.count_params = s->count_params,
			.ptr_self = ptr
	};
	res.class_name = nk_get_class(ptr);
	res.time_creation = win_time_to_string(s->time_creation);
	return res;
}

/* ********************************** */

void delkeytree_add(uint32_t ptr_parent, uint32_t ptr_child) {
	childmap *childmap_node = sglib_childmap_node_new(ptr_parent);
	childmap *childmap_member = NULL;
	if (! sglib_childmap_add_if_not_member(&delkeytree, childmap_node, &childmap_member)) {
		sglib_childmap_node_free(childmap_node);
		assert(childmap_member != NULL);
		childmap_node = childmap_member;
	}

	childset *childset_node = sglib_childset_node_new(ptr_child);
	childset *childset_member = NULL;
	int res = sglib_childset_add_if_not_member(
			&(childmap_node->val.childs), childset_node, &childset_member);
	// no duplicates
	assert(delkey_mode != DELKEY_MODE_MIX || (res != 0 && childset_member == NULL));
}

void delkeytree_print() {
	childmap *tree = delkeytree;
	childmap *te;
	struct sglib_childmap_iterator it;
	for(te=sglib_childmap_it_init(&it,tree); te!=NULL; te=sglib_childmap_it_next(&it)) {
		printf("parent = %08X\n", te->val.parent);
		childset *tree_ = te->val.childs;
		childset *te_;
		struct sglib_childset_iterator it_;
		for(te_=sglib_childset_it_init(&it_,tree_); te_!=NULL; te_=sglib_childset_it_next(&it_)) {
			printf("child = %08X\n", te_->val);
		}
	}
}

int delkey_nk_childs_process(uint32_t ptr,
			int (*callback)(uint32_t, void *), void *callback_data) {
	childmap entry = {.val = {.parent = ptr}};
	childmap *childmap_member = sglib_childmap_find_member(delkeytree, &entry);
	if (childmap_member == NULL) return 1;

	childset *tree = childmap_member->val.childs;
	childset *te;
	struct sglib_childset_iterator it;
	for(te=sglib_childset_it_init(&it,tree); te!=NULL; te=sglib_childset_it_next(&it)) {
		if (callback(te->val, callback_data)) return 1;
	}
	return 0;
}

void delkey_nk_add(uint32_t ptr) {
	do {
		nk_struct *s = nk_init_check(ptr);
		if (s == NULL) break;
		// don't add USED keys (in mix mode)
		if (s->size < 0 && delkey_mode == DELKEY_MODE_MIX) break;

		delkeytree_add(s->ptr_parent, ptr);

		if (header->ptr_root_nk == ptr) break;
		ptr = s->ptr_parent;
	} while (1);
}

void delkey_scan_blocks() {
	uint32_t ptr_segm = 0;
	do {
		hbin_struct *hbin = hbin_init(ptr_segm);
		uint32_t ptr_block = ptr_segm + hbin_struct_size;
		do {
			value_struct *block = (value_struct *)(data + ptr_block);
			uint32_t block_size = abs(block->size);
			if (block->size > 0) {	// FREE
				if (block_size >= 6) {
					signature_struct *sig_block = (signature_struct *)block;
					switch (sig_block->signature) {
					case nk_signature:
						delkey_nk_add(ptr_block);
						break;
					case vk_signature:
					case sk_signature:
					case lf_signature:
					case lh_signature:
					case li_signature:
					case ri_signature:
					case db_signature:
						break;
					default:
						// not sig
						break;
					}
				} else {
					// too small for sig
				}
			}
			ptr_block += block_size;
		} while (ptr_block < ptr_segm + hbin->size_segment);
		assert(ptr_block == ptr_segm + hbin->size_segment);
		ptr_segm += hbin->size_segment;
	} while(ptr_segm < header->size_data_area);
	assert(ptr_segm == header->size_data_area);
}

void delkey_init(DELKEY_MODE mode) {
	delkey_mode = mode;
	if (mode != DELKEY_MODE_DISABLE) delkey_scan_blocks();
}
