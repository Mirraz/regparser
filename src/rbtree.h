#ifndef RBTREE_H_
#define RBTREE_H_

#include <stdint.h>
#include "string_type.h"
#include "sglib.h"

typedef struct rbtree_ rbtree;
struct rbtree_ {
	string_and_ptr val;
	char color_field;
	rbtree *left;
	rbtree *right;
};

SGLIB_DEFINE_RBTREE_PROTOTYPES(rbtree, left, right, color_field, qweqweqwe);

rbtree *sglib_rbtree_node_new(string_and_ptr val);
void sglib_rbtree_node_free(rbtree *t);
void sglib_rbtree_free(rbtree **p_tree);

#endif /* RBTREE_H_ */
