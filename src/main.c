#include <stdio.h>
#include <assert.h>

#include <locale.h>
#include <ncurses.h>

#include "widgets.h"

FILE *flog;

/* ****************** */

struct {
	WINDOW *win_keys;
	WINDOW *win_params;
	unsigned int main_width;
	unsigned int main_height;
	unsigned int win_keys_width;
} wins;

void widg_main_update() {
	assert(wins.win_keys_width > 0);
	assert(wins.win_keys_width < wins.main_width);
	
	wresize(
		wins.win_keys,
		wins.main_height - 2,
		wins.win_keys_width - 2
	);
	wnoutrefresh(wins.win_keys);
	
	mvwin(wins.win_params, 1, wins.win_keys_width);
	wresize(
		wins.win_params,
		wins.main_height - 2,
		wins.main_width - wins.win_keys_width - 3
	);
	wnoutrefresh(wins.win_params);
	
	clear();
	box(stdscr, 0, 0);
	move(0, wins.win_keys_width-1);
	addch(ACS_TTEE);
	move(1, wins.win_keys_width-1);
	vline(ACS_VLINE, wins.main_height-2);
	move(wins.main_height-1, wins.win_keys_width-1);
	addch(ACS_BTEE);
	wnoutrefresh(stdscr);
	
	doupdate();
}

void widg_main_init() {
	wins.win_keys = newwin(1, 1, 1, 1);
	wins.win_params = newwin(1, 1, 1, 3);
}

typedef enum {
	WIDG_PANEL_KEYS,
	WIDG_PANEL_PARAMS
} current_widget_type;

current_widget_type current_widget = -1;

/* ****************** */

void widg_params_init();

/**/
char items[100][80];
const char *items_list[100];
/**/
scroll_struct scroll_keys;

void widg_keys_init() {
	unsigned int i;
	for (i=0; i<100; ++i) {
		sprintf(items[i], "item%02u", i);
		items_list[i] = (const char *)items[i];
	}
	
	scroll_struct scroll_keys_ = {
		.outer_box = wins.win_keys,
		.outer_box_width = wins.win_keys_width - 2,
		.outer_box_height = wins.main_height - 2,
		.disp_item_idx_first = 0,
		.disp_item_idx_selected = 0,
		.items_list = items_list,
		.items_count = 100
	};
	scroll_keys = scroll_keys_;
	
	scroll_update(&scroll_keys);
}

void widg_keys_ch(int ch) {
	switch (ch) {
	case '\t':
		widg_params_init();
		current_widget = WIDG_PANEL_PARAMS;
		break;
	case KEY_RESIZE:
		scroll_keys.outer_box_width = wins.win_keys_width - 2;
		scroll_keys.outer_box_height = wins.main_height - 2;
		scroll_update(&scroll_keys);
		break;
	case KEY_ENTER:
		// #####
		break;
	default:
		scroll_ch(&scroll_keys, ch);
		break;
	}
}

/* ****************** */

scroll_struct scroll_params;

void widg_params_init() {
	scroll_struct scroll_params_ = {
		.outer_box = wins.win_params,
		.outer_box_width = wins.main_width - wins.win_keys_width - 3,
		.outer_box_height = wins.main_height - 2,
		.disp_item_idx_first = 0,
		.disp_item_idx_selected = 0,
		.items_list = items_list,
		.items_count = 100
	};
	scroll_params = scroll_params_;
	
	scroll_update(&scroll_params);
}

void widg_params_clear() {
	wclear(scroll_params.outer_box);
	wrefresh(scroll_params.outer_box);
}

void widg_params_ch(int ch) {
	switch (ch) {
	case '\t':
		widg_params_clear();
		current_widget = WIDG_PANEL_KEYS;
		break;
	case KEY_RESIZE:
		scroll_params.outer_box_width = wins.main_width - wins.win_keys_width - 3;
		scroll_params.outer_box_height = wins.main_height - 2;
		scroll_update(&scroll_params);
		break;
	case KEY_ENTER:
		// #####
		break;
	default:
		scroll_ch(&scroll_params, ch);
		break;
	}
}

/* ****************** */

int main() { // int argc, char **argv) {
flog = fopen("/tmp/debug.log", "w");

	setlocale(LC_CTYPE, "");
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	widg_main_init();
	
	getmaxyx(stdscr, wins.main_height, wins.main_width);
	wins.win_keys_width = wins.main_width / 2 + 1;
	widg_main_update();
	
	widg_keys_init();
	current_widget = WIDG_PANEL_KEYS;

	int ch;
	do {
		ch = getch();
		//fprintf(flog, "[%u]\n", ch); fflush(flog);
		switch (ch) {
		case KEY_RESIZE:
			getmaxyx(stdscr, wins.main_height, wins.main_width);
			wins.win_keys_width = wins.main_width / 2 + 1;
			widg_main_update();
		default:
			switch (current_widget) {
			case WIDG_PANEL_KEYS:
				widg_keys_ch(ch);
				break;
			case WIDG_PANEL_PARAMS:
				widg_params_ch(ch);
				break;
			default:
				assert(0);
				break;
			}
			break;
		}
	} while (ch != 'q');
	
	endwin();
	
fclose(flog);
	return 0;
}

