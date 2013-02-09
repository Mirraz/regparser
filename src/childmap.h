#ifndef CHILDMAP_H_
#define CHILDMAP_H_

#include <stdint.h>
#include "sglib.h"
#include "childset.h"

typedef struct {
	uint32_t parent;
	childset *childs;
} childmap_entry;

typedef struct childmap_ childmap;
struct childmap_ {
	childmap_entry val;
	char color_field;
	childmap *left;
	childmap *right;
};

SGLIB_DEFINE_RBTREE_PROTOTYPES(childmap, left, right, color_field, qweqweqwe);

childmap *sglib_childmap_node_new(uint32_t parent);
void sglib_childmap_node_free(childmap *t);
void sglib_childmap_free(childmap **p_tree);

#endif /* CHILDMAP_H_ */
