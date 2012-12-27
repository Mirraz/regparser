#include <stdio.h>
#include <assert.h>

#include <locale.h>
#include <ncurses.h>

#define KEY_ESC 27

FILE *flog;

/* ****************** */

typedef struct {
	WINDOW *outer_box;
	unsigned int outer_box_width;
	unsigned int outer_box_height;
	unsigned int disp_item_idx_first;
	unsigned int disp_item_idx_selected;
	const char **items_list;
	unsigned int items_count;
} scroll_struct;

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

void scroll_up(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected == 0) return;
	--scroll->disp_item_idx_selected;
	if (scroll->disp_item_idx_selected < scroll->disp_item_idx_first) {
		--scroll->disp_item_idx_first;
	}
	return scroll_update(scroll);
}

void scroll_down(scroll_struct *scroll) {
	scroll_check_data(scroll);
	if (scroll->disp_item_idx_selected + 1 == scroll->items_count) return;
	++scroll->disp_item_idx_selected;
	if (scroll->disp_item_idx_selected >= scroll->disp_item_idx_first + scroll->outer_box_height) {
		++scroll->disp_item_idx_first;
	}
	return scroll_update(scroll);
}

void scroll_pageup(scroll_struct *scroll) {
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

void scroll_pagedown(scroll_struct *scroll) {
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

int main() { // int argc, char **argv) {
flog = fopen("debug.log", "w");

	int res;

	setlocale(LC_CTYPE, "");
	
	WINDOW *win_main = initscr();	// == stdscr
	cbreak();
	noecho();
	keypad(win_main, TRUE);
	
	WINDOW *panel_border_keys = derwin(win_main, 25, 40, 0, 0);
	res = wborder(
		panel_border_keys,
		ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
		ACS_ULCORNER, ACS_TTEE, ACS_LLCORNER, ACS_BTEE
	);
	WINDOW *panel_keys = derwin(panel_border_keys, 23, 38, 1, 1);
	
	WINDOW *panel_border_params = derwin(win_main, 25, 40, 0, 39);
	res = wborder(
		panel_border_params,
		ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
		ACS_TTEE, ACS_URCORNER, ACS_BTEE, ACS_LRCORNER
	);
	WINDOW *panel_params = derwin(panel_border_params, 23, 38, 1, 1);	
	
	
	char items[100][80];
	const char *items_list[100];
	unsigned int i;
	for (i=0; i<100; ++i) {
		sprintf(items[i], "item%02u", i);
		items_list[i] = (const char *)items[i];
	}
	scroll_struct scroll_keys = {
		.outer_box = panel_keys,
		.outer_box_width = 38,
		.outer_box_height = 23,
		.disp_item_idx_first = 0,
		.disp_item_idx_selected = 3,
		.items_list = items_list,
		.items_count = 100
	};
	scroll_update(&scroll_keys);
	//refresh();
	
	int ch;
	do {
		ch = getch();
		//fprintf(flog, "[%u]\n", ch); fflush(flog);
		scroll_ch(&scroll_keys, ch);
	} while (ch != 'q');
	
	endwin();
	
fclose(flog);
	return 0;
}

