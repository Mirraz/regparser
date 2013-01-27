#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "string_type.h"
#include "rbtree.h"
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

/* ********************************** */

void structs_check_size();
regf_struct *regf_init(regf_struct *s);

uint32_t regfile_init(const char *regfile_path) {
	structs_check_size();

	fd = open(regfile_path, O_RDONLY);

	ssize_t red = read(fd, header, regf_struct_size);
	assert(red == regf_struct_size);
	regf_init(header);

	data = mmap(NULL, header->size_data_area,
			PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, regf_header_size);
	assert(data != MAP_FAILED);

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
	/* if key_name is in unicode, then size_key_name must be even */
	assert_check1( (s->flag & 0x20) || (s->size_key_name & 1) == 0);
	assert_check1(check_size(s->size_key_class));

	assert_check1(check_ptr(s->ptr_parent));
	assert_check1(check_ptr(s->ptr_chinds_index));
	assert_check1(check_ptr(s->ptr_params_index));
	assert_check1(check_ptr(s->ptr_sk));
	assert_check1(check_ptr(s->ptr_class_name));

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
		name = string_new_from_unicode(s->key_name, s->size_key_name);
	}
	return name;
}

void nk_childs_index_process(uint32_t ptr_chinds_index,
			int (*callback)(uint32_t, void *), void *callback_data) {
	assert(ptr_not_null(ptr_chinds_index));
	signature_struct *sig = signature_init(ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature: {
		lf_struct *lf = lf_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lf->count_records; ++i) {
			if (callback(lf->records[i].ptr_nk, callback_data)) return;
		}
		break;
	}
	case lh_signature: {
		lh_struct *lh = lh_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lh->count_records; ++i) {
			if (callback(lh->records[i].ptr_nk, callback_data)) return;
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
			nk_childs_index_process(ri->ptr_indexes[i], callback, callback_data);
		}
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
}

void nk_childs_process(uint32_t ptr,
		int (*callback)(uint32_t, void *), void *callback_data) {
	nk_struct *nk = nk_init(ptr);
	if (ptr_is_null(nk->ptr_chinds_index)) return;
	return nk_childs_index_process(nk->ptr_chinds_index, callback, callback_data);
}

int child_print_name(uint32_t ptr, void *data) {
	(void)data;
	string name = nk_get_name(ptr);
	string_print(name);
	string_free(name);
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
	string_and_ptr_list list = *p_list;
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_free(list.entries[i].str);
	}
	free(list.entries);
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

	return res;
}

int childs_find(uint32_t ptr, void *data) {
	string_and_ptr *find_child = (string_and_ptr *)data;
	string cur_name = nk_get_name(ptr);
	int res = string_compare(cur_name, find_child->str);
	if (res == 0) {
		find_child->ptr = ptr;
	}
	string_free(cur_name);
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
			name = string_new_from_unicode(s->param_name, s->size_param_name);
		}
		return name;
	}
}

string vk_get_type(uint32_t ptr) {
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
	params_parsed_list list = *p_list;
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_free(list.entries[i].name);
		string_free(list.entries[i].type);
		string_free(list.entries[i].value_brief);
	}
	free(list.entries);
	p_list->entries = NULL;
	p_list->size = 0;
}

params_parsed_list nk_get_params_parsed_list(uint32_t ptr) {
	nk_struct *s = nk_init(ptr);
	if (ptr_is_null(s->ptr_params_index)) {
		params_parsed_list list = {.entries = NULL, .size = 0};
		return list;
	}

	unsigned int count_index_records = s->count_params;
	index_struct *index_params =
			index_init(s->ptr_params_index, count_index_records);
	params_parsed_list list = {.size = count_index_records};
	list.entries = malloc(list.size * sizeof(list.entries[0]));
	assert(list.entries != NULL);

	unsigned int i;
	for (i=0; i<count_index_records; ++i) {
		uint32_t ptr_vk = index_params->ptr_blocks[i];
		param_parsed val;
		val.name = vk_get_name(ptr_vk);
		val.type = vk_get_type(ptr_vk);
		vk_struct *vk = vk_init(ptr_vk);
		val.size_value = vk->size_param_value & ~0x80000000;
val.value_brief.len = 0;
val.value_brief.str = NULL;
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
	string_list list = *p_list;
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		string_free(list.entries[i]);
	}
	free(list.entries);
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
	list.entries = malloc(list.size * sizeof(list.entries[0]));
	assert(list.entries != NULL);

	ptr_list_entry *entry = ptr_list;
	unsigned int i;
	for (i=0; i<entries_count; ++i) {
		list.entries[i] = nk_get_name(entry->ptr);
		entry = entry->next;
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
