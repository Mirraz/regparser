#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <locale.h>
#include <ncurses.h>

#include "widgets.h"
#include "regfile.h"
#include "parse_common.h"
#include "string_type.h"
#include "debug.h"

FILE *flog;

/* ****************** */

struct {
	struct {
		WINDOW *pwd;
		WINDOW *childs;
		WINDOW *key_stats;
		WINDOW *params;
	} wins;
	struct {
		unsigned int main_width;
		unsigned int main_height;
	} geom;
} widg_main;

typedef struct {
	uint32_t *entries;
	unsigned int size;
} ptr_list_type;

typedef struct {
	scroll_struct scroll;
	ptr_list_type ptr_list;
} widg_list_type;

#define widg_list_init_value { \
	.scroll.list = { \
		.entries = NULL, \
		.size = 0 \
	}, \
	.ptr_list = { \
		.entries = NULL, \
		.size = 0 \
	} \
}

widg_list_type widg_childs = widg_list_init_value;
widg_list_type widg_params = widg_list_init_value;

typedef enum {
	WIDG_INVALID,
	WIDG_CHILDS,
	WIDG_PARAMS
} widget_type;

struct {
	widget_type current_widget;
	uint32_t ptr_nk_current;
} state = {
		.current_widget = WIDG_INVALID,
		.ptr_nk_current = (uint32_t)(-1)
};

const unsigned int widg_stats_height = 5;

/* ****************** */

void widg_main_init() {
	widg_main.wins.pwd = newwin(1, 2, 1, 1);
	widg_main.wins.childs = newwin(widg_stats_height + 2, 1, 3, 1);
	widg_main.wins.key_stats = newwin(widg_stats_height, 1, 3, 3);
	widg_main.wins.params = newwin(1, 1, widg_stats_height + 4, 3);
}

void widg_main_update() {
	if (widg_main.geom.main_width < 5) widg_main.geom.main_width = 5;
	if (widg_main.geom.main_height < 7) widg_main.geom.main_height = 7;
	unsigned int widg_childs_width = widg_main.geom.main_width / 3;   // main_width/2-1;
	unsigned int widg_params_width = widg_main.geom.main_width - widg_childs_width - 3;

	wresize(
		widg_main.wins.pwd,
		1,
		widg_main.geom.main_width - 2
	);
	wnoutrefresh(widg_main.wins.pwd);

	wresize(
		widg_main.wins.childs,
		widg_main.geom.main_height - 4,
		widg_childs_width
	);
	wnoutrefresh(widg_main.wins.childs);

	mvwin(widg_main.wins.key_stats, 3, widg_childs_width + 2);
	wresize(
		widg_main.wins.key_stats,
		widg_stats_height,
		widg_params_width
	);
	wnoutrefresh(widg_main.wins.key_stats);

	mvwin(widg_main.wins.params, widg_stats_height + 4, widg_childs_width + 2);
	wresize(
		widg_main.wins.params,
		widg_main.geom.main_height - widg_stats_height - 5,
		widg_params_width
	);
	wnoutrefresh(widg_main.wins.params);

	clear();
	box(stdscr, 0, 0);

	mvaddch(2, 0, ACS_LTEE);
	mvhline(2, 1, ACS_HLINE, widg_main.geom.main_width - 2);
	mvaddch(2, widg_main.geom.main_width - 1, ACS_RTEE);

	mvaddch(2, widg_childs_width + 1, ACS_TTEE);
	mvvline(3, widg_childs_width + 1, ACS_VLINE, widg_main.geom.main_height - 4);
	mvaddch(widg_main.geom.main_height - 1, widg_childs_width + 1, ACS_BTEE);

	mvaddch(widg_stats_height + 3, widg_childs_width + 1, ACS_LTEE);
	mvhline(widg_stats_height + 3, widg_childs_width + 2, ACS_HLINE, widg_params_width);
	mvaddch(widg_stats_height + 3, widg_main.geom.main_width - 1, ACS_RTEE);

	curs_set(0);

	wnoutrefresh(stdscr);

	doupdate();

}
void widg_childs_ch(int ch);
void widg_params_ch(int ch);

int widg_main_ch(int ch) {
	//fprintf(flog, "[%u]\n", ch); fflush(flog);
	switch (ch) {
	case 'q':
		return 1;
		break;
	case KEY_RESIZE:
		getmaxyx(stdscr, widg_main.geom.main_height, widg_main.geom.main_width);
		widg_main_update();
	default:
		switch (state.current_widget) {
		case WIDG_CHILDS:
			widg_childs_ch(ch);
			break;
		case WIDG_PARAMS:
			widg_params_ch(ch);
			break;
		default:
			//assert(0);
			break;
		}
		break;
	}
	return 0;
}

/* ****************** */

void widg_list_free(widg_list_type *widg) {
	unsigned int i;
	if (widg->scroll.list.entries != NULL) {
		for (i=0; i<widg->scroll.list.size; ++i)
			string_free(widg->scroll.list.entries[i]);
		free(widg->scroll.list.entries);
		widg->scroll.list.entries = NULL;
		widg->scroll.list.size = 0;
	}
	if (widg->ptr_list.entries != NULL) {
		free(widg->ptr_list.entries);
		widg->ptr_list.entries = NULL;
		widg->ptr_list.size = 0;
	}
}

void widg_childs_init() {
	widg_list_free(&widg_childs);

	int height, width;
	getmaxyx(widg_main.wins.childs, height, width);
	scroll_struct scroll = {
		.outer_box = widg_main.wins.childs,
		.outer_box_width = width,
		.outer_box_height = height,
		.disp_item_idx_first = 0,
		.disp_item_idx_selected = 0
	};

	string_and_ptr_list list = nk_get_childs_list(state.ptr_nk_current);
	uint32_t ptr_parent = nk_get_parent(state.ptr_nk_current);
	unsigned int items_count = list.size;
	if (ptr_not_null(ptr_parent)) ++items_count; // for ".."
	scroll.list.entries = malloc(items_count * sizeof(string));
	scroll.list.size = items_count;
	widg_childs.ptr_list.entries = malloc(items_count * sizeof(uint32_t));
	widg_childs.ptr_list.size = items_count;

	unsigned int add_if_parent = 0;
	if (ptr_not_null(ptr_parent)) {
		string parent = {.str = strndup("..", 2), .len = 2};
		scroll.list.entries[0] = parent;
		widg_childs.ptr_list.entries[0] = ptr_parent;
		++add_if_parent;
	}

	unsigned int i;
	for (i=0; i<list.size; ++i) {
		scroll.list.entries[i+add_if_parent] = list.entries[i].str;
		widg_childs.ptr_list.entries[i+add_if_parent] = list.entries[i].ptr;
	}
	free(list.entries);

	widg_childs.scroll = scroll;
	scroll_update(&widg_childs.scroll);

}

void widg_childs_ch(int ch) {
	switch(ch) {
	case '\n':
		;
		unsigned int idx = widg_childs.scroll.disp_item_idx_selected;
		assert(idx < widg_childs.ptr_list.size);
		state.ptr_nk_current = widg_childs.ptr_list.entries[idx];
		widg_childs_init();
		break;
	default:
		scroll_ch(&widg_childs.scroll, ch);
		break;
	}
}

void widg_params_ch(int ch) {
	(void) ch;
}

/* ****************** */

int main() {   // int argc, char **argv) {
flog = fopen("/tmp/debug.log", "w");

	setlocale(LC_CTYPE, "");

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	widg_main_init();
	widg_main_ch(KEY_RESIZE);

	state.ptr_nk_current = regfile_init("NTUSER.DAT");
	state.current_widget = WIDG_CHILDS;

	widg_childs_init();

	int ch;
	do {
		ch = getch();
	} while (!widg_main_ch(ch));

	regfile_uninit();

	endwin();
	
fclose(flog);
	return 0;
}

