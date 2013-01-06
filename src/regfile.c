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

#if 0

#include "codepages.h"

void nk_print_name(nk_struct *s) {
	assert(s != NULL);
	if (s->flag & 0x20) {
		/* ansi */
		unsigned int i;
		for (i=0; i<s->size_key_name; ++i)
			fprintf(fout, "%c", s->key_name[i]);
	} else {
		/* unicode */
		fprintStringUnicode(fout, s->key_name, (s->size_key_name)>>1);
	}
}

void nk_print_class(nk_struct *s) {
	assert(s != NULL);
	assert(ptr_not_null(s->ptr_class_name));
	unsigned int size_key_class = s->size_key_class;
	value_struct *class_value =
			value_init(s->ptr_class_name, size_key_class);
	fprintStringUnicode(fout, class_value->value, size_key_class>>1);
}

void nk_print_sk(nk_struct *s) {
	assert(s != NULL);
	assert(ptr_not_null(s->ptr_sk));
	sk_struct *sk = sk_init(s->ptr_sk);
	sec_data_parse(sk->data, sk->size);
}

void vk_print_name(vk_struct *s) {
	assert(s != NULL);
	if (s->size_param_name == 0) {
		fprintf(fout, "@");
	} else {
		if (s->flag & 1) {
			/* ansi */
			unsigned int i;
			for (i=0; i<s->size_param_name; ++i)
				fprintf(fout, "%c", s->param_name[i]);
		} else {
			/* unicode */
			fprintStringUnicode(fout, s->param_name, (s->size_param_name)>>1);
		}
	}
}

void nk_ls_params(nk_struct *s) {
	assert(s != NULL);
	assert(ptr_not_null(s->ptr_params_index));
	unsigned int count_index_records = s->count_params;
	index_struct *index_params =
			index_init(s->ptr_params_index, count_index_records);
	unsigned int i;
	for (i=0; i<count_index_records; ++i) {
		vk_struct *vk = vk_init(index_params->ptr_blocks[i]);
		vk_print_name(vk); fprintf(fout, "\n");
	}
}

void value_type_print(uint8_t *value_data, uint32_t value_size, uint32_t vk_type) {
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		unsigned int bytes_counter = 0;
		unsigned int i;
		for (i=0; i<value_size; ++i) {
			if (bytes_counter == 16) {
				fprintf(fout, "\n");
				bytes_counter = 0;
			}
			fprintf(fout, "%02X ", value_data[i]);
			++bytes_counter;
		}
		fprintf(fout, "\n");
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		/* in "default" regfile may by "?? ?? .. .. ?? ?? 00 00 ??" */
		/*assert(!(value_size & 1));*/

		fprintf(fout, "\"");
		fprintStringUnicode(fout, value_data, value_size>>1);
		fprintf(fout, "\"");
		fprintf(fout, "\n");
		break;
	}
	case REG_DWORD: {
		fprintf(fout, "%08X", *((uint32_t *)value_data));
		fprintf(fout, "\n");
		break;
	}
	case REG_DWORD_BIG_ENDIAN: {
		fprintf(fout, "%08X", be32toh(*((uint32_t *)value_data)));
		fprintf(fout, "\n");
		break;
	}
	case REG_QWORD: {
		fprintf(fout, "%016llX", (long long unsigned int)(*((uint64_t *)value_data)));
		fprintf(fout, "\n");
		break;
	}
	case REG_MULTI_SZ: {
		uint16_t *utf16_data = (uint16_t *)value_data;
		assert(!(value_size & 1));
		uint32_t utf16_rest = value_size >> 1;
		unsigned int i = 0;
		/* sometimes loop stops by condition "i < utf16_rest" */
		while (i < utf16_rest && utf16_data[i] != 0) {
			utf16_data += i;
			utf16_rest -= i;
			i = 0;
			while (i < utf16_rest && utf16_data[i] != 0) ++i;
			fprintStringUnicode(fout, (uint8_t *)utf16_data, i);
			fprintf(fout, "\n");
			if (i == utf16_rest) break;
			++i;
		}
		break;
	}
	}
}

void vk_print_value(vk_struct *s) {
	assert(s != NULL);
//fprintf(fout, "size = %d\n", s->size_param_value & ~0x80000000);
	if (!(s->size_param_value & 0x80000000)) {
		value_struct *block = (value_struct *)(data + s->ptr_param_value);
		/* this condition doesn't work */
		/*if (s->size_param_value <= PARAM_PART_MAX)*/
		if (abs(block->size) >= s->size_param_value) {
			value_struct *param_value =
					value_init(s->ptr_param_value, s->size_param_value);
			value_type_print(param_value->value, s->size_param_value, s->param_type);
		} else {
			/* db */
			if (
					s->param_type == REG_NONE ||
					s->param_type == REG_BINARY ||
					s->param_type == REG_RESOURCE_LIST ||
					s->param_type == REG_FULL_RESOURCE_DESCRIPTOR ||
					s->param_type == REG_RESOURCE_REQUIREMENTS_LIST ||
					s->param_type > param_types_count) {
				
				db_struct *db = db_init(s->ptr_param_value);
				index_struct *part_index = index_init(db->ptr_value_parts_index, db->count_records);
				uint32_t rest = s->size_param_value;
				unsigned int bytes_counter = 0;
				unsigned int i;
				for (i=0; i<db->count_records; ++i) {
					uint32_t part_size = (rest > PARAM_PART_MAX ? PARAM_PART_MAX : rest);
					rest -= part_size;
					value_struct *part = value_init(part_index->ptr_blocks[i], part_size);
					unsigned int j;
					for (j=0; j<part_size; ++j) {
						if (bytes_counter == 16) {
							fprintf(fout, "\n");
							bytes_counter = 0;
						}
						fprintf(fout, "%02X ", part->value[j]);
						++bytes_counter;
					}
				}
				fprintf(fout, "\n");
				
			} else {
				fprintf(stderr, "db unsupported type %s\n", param_type_desc[s->param_type].name);
			}
		}
	} else {
		value_type_print((uint8_t *)&(s->ptr_param_value), s->size_param_value & ~0x80000000, s->param_type);
	}
}

void parse_childs(uint32_t ptr_chinds_index, void (*cb)(nk_struct *)) {
	assert(ptr_not_null(ptr_chinds_index));
	signature_struct *sig = signature_init(ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature: {
		lf_struct *lf = lf_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lf->count_records; ++i) {
			nk_struct *nk = nk_init(lf->records[i].ptr_nk);
			cb(nk);
		}
		break;
	}
	case lh_signature: {
		lh_struct *lh = lh_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lh->count_records; ++i) {
			nk_struct *nk = nk_init(lh->records[i].ptr_nk);
			cb(nk);
		}
		break;
	}
	case li_signature: {
		//li_struct *li1 = (li_struct *)(data + ptr_chinds_index);
fprintf(stderr, "li\n");
		break;
	}
	case ri_signature: {
		ri_struct *ri = ri_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<ri->count_records; ++i)
			parse_childs(ri->ptr_indexes[i], cb);
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
}

void nk_ls_childs_cb(nk_struct *s) {
	assert(s != NULL);
	nk_print_name(s); fprintf(fout, "\n");
}

void nk_ls_childs(nk_struct *s) {
	assert(s != NULL);
	assert(ptr_not_null(s->ptr_chinds_index));
	parse_childs(s->ptr_chinds_index, &nk_ls_childs_cb);
}

void nk_print_pwd(nk_struct *s) {
	if (s != (nk_struct *)(data + header->ptr_root_nk)) {
		nk_print_pwd(nk_init(s->ptr_parent));
		fprintf(fout, "/");
		nk_print_name(s);
	}
}

void nk_print_verbose(nk_struct *s) {
	assert(s != NULL);

	fprintf(fout, "[path]\n");
	nk_print_pwd(s);
	fprintf(fout, "\n---------------\n");

	fprintf(fout, "[key name]\n");
	nk_print_name(s);
	fprintf(fout, "\n---------------\n");

	if (ptr_not_null(s->ptr_class_name)) {
		fprintf(fout, "[key class]\n");
		nk_print_class(s);
		fprintf(fout, "\n---------------\n");
	}

	if (ptr_not_null(s->ptr_sk)) {
		fprintf(fout, "[sk]\n");
		nk_print_sk(s);
		fprintf(fout, "---------------\n");
	}

	if (s->count_params != 0) {
		fprintf(fout, "[params]\n");
		/*nk_ls_params(s);*/
		
		assert(ptr_not_null(s->ptr_params_index));
		unsigned int count_index_records = s->count_params;
		index_struct *index_params =
				index_init(s->ptr_params_index, count_index_records);
		unsigned int i;
		for (i=0; i<count_index_records; ++i) {
			vk_struct *vk = vk_init(index_params->ptr_blocks[i]);
			vk_print_name(vk); fprintf(fout, "\n");
			unsigned int j;
			for (j=0; j<param_types_count && param_type_desc[j].type != vk->param_type; ++j);
			fprintf(fout, "%s", param_type_desc[j].name);
			if (j == param_types_count) fprintf(fout, "=%d", vk->param_type);
			fprintf(fout, " (%d)\n", vk->size_param_value & ~0x80000000);
			vk_print_value(vk);
			if (i+1 != count_index_records) fprintf(fout, "-----\n");
		}
		fprintf(fout, "---------------\n");
	}

	if (ptr_not_null(s->ptr_chinds_index)) {
		fprintf(fout, "[childs]\n");
		nk_ls_childs(s);
		fprintf(fout, "---------------\n");
	}
}

void nk_recur(nk_struct *s) {
	assert(s != NULL);

	nk_print_verbose(s);

	fprintf(fout, "============================\n");

	if (ptr_not_null(s->ptr_chinds_index)) {
		parse_childs(s->ptr_chinds_index, &nk_recur);
	}
}

int nk_check_name(nk_struct *s, const char* name, uint32_t size_name) {
	if (s->size_key_name != size_name) return 1;
	return strncmp((const char *)&(s->key_name), name, size_name);
}

typedef struct {
	int res;
	nk_struct *nk;
} nk_cd_parse_childs_res;

nk_cd_parse_childs_res nk_cd_parse_childs(uint32_t ptr_chinds_index, const char* name, uint32_t size_name) {
	assert(ptr_not_null(ptr_chinds_index));
	signature_struct *sig = signature_init(ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature: {
		lf_struct *lf = lf_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lf->count_records; ++i) {
			nk_struct *nk = nk_init(lf->records[i].ptr_nk);
			if (!nk_check_name(nk, name, size_name)) {
				nk_cd_parse_childs_res res = {.res = 0, .nk = nk};
				return res;
			}
		}
		break;
	}
	case lh_signature: {
		lh_struct *lh = lh_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lh->count_records; ++i) {
			nk_struct *nk = nk_init(lh->records[i].ptr_nk);
			if (!nk_check_name(nk, name, size_name)) {
				nk_cd_parse_childs_res res = {.res = 0, .nk = nk};
				return res;
			}
		}
		break;
	}
	case li_signature: {
		//li_struct *li1 = (li_struct *)(data + ptr_chinds_index);
fprintf(stderr, "li\n");
		break;
	}
	case ri_signature: {
		ri_struct *ri = ri_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<ri->count_records; ++i) {
			nk_cd_parse_childs_res res = nk_cd_parse_childs(ri->ptr_indexes[i], name, size_name);
			if (!res.res) return res;
		}
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
	nk_cd_parse_childs_res res = {.res = 1, .nk = NULL};
	return res;
}

nk_struct *nk_cd(nk_struct *s, const char *path) {
	const char *end_token;
	do {
		end_token = strchrnul(path, '/');
		assert(end_token - path >= 0);
		size_t length = end_token - path;
		if (length > 0) {
			
			if (ptr_is_null(s->ptr_chinds_index)) return NULL;
			nk_cd_parse_childs_res res = nk_cd_parse_childs(s->ptr_chinds_index, path, length);
			if (res.res) return NULL;
			
			s = res.nk;
		}
		path = end_token+1;
	} while (*end_token != '\0');
	return s;
}

#endif

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
