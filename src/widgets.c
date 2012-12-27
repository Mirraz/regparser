#include <assert.h>
#include "widgets.h"

inline static void scroll_check_data(scroll_struct *scroll) {
	assert(scroll->outer_box != NULL);
	assert(scroll->items_list != NULL);
	assert(scroll->disp_item_idx_first <= scroll->items_count);
	assert(scroll->disp_item_idx_selected <= scroll->items_count);
	assert(scroll->disp_item_idx_selected >= scroll->disp_item_idx_first);
	assert(scroll->disp_item_idx_selected < scroll->disp_item_idx_first + scroll->outer_box_height);
}

void scroll_update(scroll_struct *scroll) {
	scroll_check_data(scroll);
	wclear(scroll->outer_box);
	unsigned int idx;
	for (
		idx=scroll->disp_item_idx_first;
		idx < scroll->disp_item_idx_first + scroll->outer_box_height;
		++idx
	) {
		if (idx == scroll->disp_item_idx_selected) wattron(scroll->outer_box, A_REVERSE);
		wmove(scroll->outer_box, idx-scroll->disp_item_idx_first, 0);
		wprintw(scroll->outer_box, "%.*s", scroll->outer_box_width, scroll->items_list[idx]);
		if (idx == scroll->disp_item_idx_selected) wattroff(scroll->outer_box, A_REVERSE);
	}
	curs_set(0);
	wrefresh(scroll->outer_box);
}

static void scroll_up(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected == 0) return;
	--scroll->disp_item_idx_selected;
	if (scroll->disp_item_idx_selected < scroll->disp_item_idx_first) {
		--scroll->disp_item_idx_first;
	}
	return scroll_update(scroll);
}

static void scroll_down(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected + 1 == scroll->items_count) return;
	++scroll->disp_item_idx_selected;
	if (scroll->disp_item_idx_selected >= scroll->disp_item_idx_first + scroll->outer_box_height) {
		++scroll->disp_item_idx_first;
	}
	return scroll_update(scroll);
}

static void scroll_pageup(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected == 0) return;
	if (scroll->disp_item_idx_selected < scroll->outer_box_height) {
		scroll->disp_item_idx_selected = 0;
	} else {
		scroll->disp_item_idx_selected -= scroll->outer_box_height;
	}
	if (scroll->disp_item_idx_selected < scroll->disp_item_idx_first) {
		scroll->disp_item_idx_first = scroll->disp_item_idx_selected;
	}
	return scroll_update(scroll);
}

static void scroll_pagedown(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected + 1 == scroll->items_count) return;
	if (scroll->disp_item_idx_selected + 2*scroll->outer_box_height >= scroll->items_count) {
		scroll->disp_item_idx_selected = scroll->items_count-1;
	} else {
		scroll->disp_item_idx_selected += scroll->outer_box_height;
	}
	if (scroll->disp_item_idx_selected >= scroll->disp_item_idx_first + scroll->outer_box_height) {
		scroll->disp_item_idx_first = scroll->disp_item_idx_selected + 1 - scroll->outer_box_height;
	}
	return scroll_update(scroll);
}

void scroll_ch(scroll_struct *scroll, int ch) {
	switch(ch) {
	case KEY_UP:
		scroll_up(scroll);
		break;
	case KEY_DOWN:
		scroll_down(scroll);
		break;
	case KEY_PPAGE:
		scroll_pageup(scroll);
		break;
	case KEY_NPAGE:
		scroll_pagedown(scroll);
		break;
	}
}

/* ****************** */


