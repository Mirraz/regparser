#include <assert.h>
#include <stdlib.h>
#include "childset.h"
#include "sglib.h"

#define childset_node_cmp(a,b) (a->val - b->val)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
SGLIB_DEFINE_RBTREE_FUNCTIONS(childset, left, right, color_field, childset_node_cmp);
#pragma GCC diagnostic pop

childset *sglib_childset_node_new(childset_entry val) {
	childset *t = malloc(sizeof(childset));
	t->val = val;
	return t;
}

void sglib_childset_node_free(childset *t) {
	free(t);
}

void sglib_childset_free(childset **p_tree) {
	childset *tree = *p_tree;
	struct sglib_childset_iterator it;
	childset *te;
	for(te=sglib_childset_it_init(&it,tree); te!=NULL; te=sglib_childset_it_next(&it)) {
		sglib_childset_node_free(te);
	}
	*p_tree = NULL;
}
