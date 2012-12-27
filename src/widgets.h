#ifndef WIDGETS_H_
#define WIDGETS_H_

#include <ncurses.h>

typedef struct {
	WINDOW *outer_box;
	unsigned int outer_box_width;
	unsigned int outer_box_height;
	unsigned int disp_item_idx_first;
	unsigned int disp_item_idx_selected;
	const char **items_list;
	unsigned int items_count;
} scroll_struct;

void scroll_update(scroll_struct *scroll);
void scroll_ch(scroll_struct *scroll, int ch);

#endif /* WIDGETS_H_ */
