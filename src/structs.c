#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#error
#endif

#include "codepages.h"
#include "structs.h"

uint8_t *data = NULL;
uint32_t size_data_area;

/* ********************************** */

#define check_struct_size(PREFIX) assert(sizeof(PREFIX##_struct) == PREFIX##_struct_size)

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
}

#undef check_struct_size

/* ********************************** */

#define assert_check(condition, errnum) assert(condition)
/* some field of out structure (which was correctly identified) has wrong value */
#define assert_check1(condition) assert_check(condition, 1)
/* ptr was wrong and/or structure is not correctly identified */
#define assert_check2(condition) assert_check(condition, 2)

#define check_signatire(PREFIX) (s->signature == PREFIX##_signature)
#define check_block_ptr() (ptr_not_null(ptr) && ptr < size_data_area)
#define check_ptr(ptr) (ptr_is_null(ptr) || ptr < size_data_area)
#define check_size(size) (size < size_data_area)
#define check_block_size() check_size(abs(s->size))

/* ********************************** */

regf_struct *regf_init(regf_struct *s) {
	assert(s != NULL);
	assert_check2(check_signatire(regf));

	assert_check1(s->stuff1 == 1);
	assert_check1(s->subversion == 0);
	assert_check1(s->stuff2 == 1);
	assert_check1(s->stuff3 >= 1 && s->stuff3 <= 8);
	// TODO: check checksum

	size_data_area = s->size_data_area;
	assert_check1(check_ptr(s->ptr_root_nk));

	return s;
}

/* ========= */

void set_data(uint8_t *data_) {
	assert(data_ != NULL);
	data = data_;
}

/* ********************************** */

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

FILE *fout = NULL;

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
	unsigned int i;
	for (i=0; i<sk->size_data; ++i) printf("%02X ", sk->data[i]);
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

void parse_childs(uint32_t ptr_chinds_index, void (*cb)(nk_struct *)) {
	assert(ptr_not_null(ptr_chinds_index));
	signature_struct *sig = signature_init(ptr_chinds_index);
	switch (sig->signature) {
	case lf_signature:
	{
		lf_struct *lf = lf_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lf->count_records; ++i) {
			nk_struct *nk = nk_init(lf->records[i].ptr_nk);
			cb(nk);
		}
		break;
	}
	case lh_signature:
	{
		lh_struct *lh = lh_init(ptr_chinds_index);
		unsigned int i;
		for (i=0; i<lh->count_records; ++i) {
			nk_struct *nk = nk_init(lh->records[i].ptr_nk);
			cb(nk);
		}
		break;
	}
	case li_signature:
	{
		//li_struct *li1 = (li_struct *)(data + ptr_chinds_index);
fprintf(stderr, "li\n");
		break;
	}
	case ri_signature:
	{
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
