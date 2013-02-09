#include <assert.h>
#include <stdlib.h>
#include "childmap.h"
#include "childset.h"
#include "sglib.h"

#define childmap_node_cmp(a,b) (a->val.parent - b->val.parent)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
SGLIB_DEFINE_RBTREE_FUNCTIONS(childmap, left, right, color_field, childmap_node_cmp);
#pragma GCC diagnostic pop

childmap *sglib_childmap_node_new(uint32_t parent) {
	childmap *t = malloc(sizeof(childmap));
	t->val.parent = parent;
	t->val.childs = NULL;
	return t;
}

void sglib_childmap_node_free(childmap *t) {
	sglib_childset_node_free(t->val.childs);
	free(t);
}

void sglib_childmap_free(childmap **p_tree) {
	childmap *tree = *p_tree;
	struct sglib_childmap_iterator it;
	childmap *te;
	for(te=sglib_childmap_it_init(&it,tree); te!=NULL; te=sglib_childmap_it_next(&it)) {
		sglib_childmap_node_free(te);
	}
	*p_tree = NULL;
}
