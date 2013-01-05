#include <assert.h>
#include <stdlib.h>
#include "string_type.h"
#include "rbtree.h"
#include "sglib.h"

#define rbtree_node_cmp(a,b) string_compare(a->val.str, b->val.str)

SGLIB_DEFINE_RBTREE_FUNCTIONS(rbtree, left, right, color_field, rbtree_node_cmp);

rbtree *sglib_rbtree_node_new(string_and_ptr val) {
	rbtree *t = malloc(sizeof(rbtree));
	t->val = val;
	return t;
}

void sglib_rbtree_node_free(rbtree *t) {
	free(t);
}

void sglib_rbtree_free(rbtree **p_tree) {
	rbtree *tree = *p_tree;
	struct sglib_rbtree_iterator it;
	rbtree *te;
	for(te=sglib_rbtree_it_init(&it,tree); te!=NULL; te=sglib_rbtree_it_next(&it)) {
		sglib_rbtree_node_free(te);
	}
	*p_tree = NULL;
}
