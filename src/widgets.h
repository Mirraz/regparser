#ifndef WIDGETS_H_
#define WIDGETS_H_

#include <ncurses.h>
#include "string_type.h"

typedef struct {
	string *entries;
	unsigned int size;
} string_list_type;

typedef struct {
	WINDOW *outer_box;
	unsigned int outer_box_width;
	unsigned int outer_box_height;
	unsigned int disp_item_idx_first;
	unsigned int disp_item_idx_selected;
	string_list_type list;
} scroll_struct;

void scroll_update(scroll_struct *scroll);
void scroll_ch(scroll_struct *scroll, int ch);

#endif /* WIDGETS_H_ */
