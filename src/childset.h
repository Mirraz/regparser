#ifndef CHILDSET_H_
#define CHILDSET_H_

#include <stdint.h>
#include "sglib.h"

typedef uint32_t childset_entry;

typedef struct childset_ childset;
struct childset_ {
	childset_entry val;
	char color_field;
	childset *left;
	childset *right;
};

SGLIB_DEFINE_RBTREE_PROTOTYPES(childset, left, right, color_field, qweqweqwe);

childset *sglib_childset_node_new(childset_entry val);
void sglib_childset_node_free(childset *t);
void sglib_childset_free(childset **p_tree);

#endif /* CHILDSET_H_ */
