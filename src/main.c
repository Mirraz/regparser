#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <locale.h>
#include <ncurses.h>

#include "widgets.h"
#include "regfile.h"
#include "parse_common.h"
#include "string_type.h"
#include "common.h"
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
struct {
	unsigned int total_max;
	unsigned int name;
	unsigned int type;
	unsigned int size_value;
} widg_params_widths = {
	.total_max = 128,
	.name = 30,
	.type = 13,
	.size_value = 10
};

#define KEY_NK_CHANGE (KEY_MAX+1)

typedef enum {
	MODE_INVALID,
	MODE_MAIN,
	MODE_VK
} mode_type;

typedef enum {
	WIDG_INVALID,
	WIDG_CHILDS,
	WIDG_PARAMS
} widget_type;

struct {
	mode_type current_mode;
	widget_type current_widget;
	uint32_t ptr_nk_current;
} state = {
		.current_mode = MODE_MAIN,
		.current_widget = WIDG_INVALID,
		.ptr_nk_current = (uint32_t)(-1)
};

const unsigned int widg_stats_height = 5;

/* ****************** */

int widg_main_ch(int ch);
int mode_vk_ch(int ch);

int modes_ch(int ch) {
	switch(state.current_mode) {
	case MODE_MAIN:
		return widg_main_ch(ch);
		break;
	case MODE_VK:
		return mode_vk_ch(ch);
		break;
	default:
		assert(0);
		break;
	}
	return 1;
}

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
void widg_pwd_ch(int ch);

int widg_main_ch(int ch) {
	//fprintf(flog, "[%u]\n", ch); fflush(flog);
	switch (ch) {
	case 'q':
		return 1;
		break;
	case KEY_RESIZE:
		getmaxyx(stdscr, widg_main.geom.main_height, widg_main.geom.main_width);
		widg_main_update();
		widg_childs_ch(ch);
		widg_params_ch(ch);
		widg_pwd_ch(ch);
		break;
	case KEY_NK_CHANGE:
		widg_childs_ch(ch);
		widg_params_ch(ch);
		widg_pwd_ch(ch);
		break;
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
			string_free(&widg->scroll.list.entries[i]);
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
		.disp_item_idx_selected = 0,
		.disp_show_select = (state.current_widget == WIDG_CHILDS)
	};

	string_and_ptr_list list = nk_get_childs_list(state.ptr_nk_current);
	uint32_t ptr_parent = nk_get_parent(state.ptr_nk_current);
	unsigned int items_count = list.size;
	if (ptr_not_null(ptr_parent)) ++items_count; // for ".."
	scroll.list.entries = malloc(items_count * sizeof(string));
	assert(scroll.list.entries != NULL);
	scroll.list.size = items_count;
	widg_childs.ptr_list.entries = malloc(items_count * sizeof(uint32_t));
	assert(widg_childs.ptr_list.entries != NULL);
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

void widg_childs_select() {
	unsigned int idx = widg_childs.scroll.disp_item_idx_selected;
	assert(idx < widg_childs.ptr_list.size);
	state.ptr_nk_current = widg_childs.ptr_list.entries[idx];
}

void widg_childs_ch(int ch) {
	switch(ch) {
	case '\n':
		widg_childs_select();
		widg_main_ch(KEY_NK_CHANGE);
		break;
	case KEY_NK_CHANGE:
		widg_childs_init();
		break;
	case '\t':
		state.current_widget = WIDG_PARAMS;
		widg_childs.scroll.disp_show_select = 0;
		widg_params.scroll.disp_show_select = 1;
		scroll_update(&widg_childs.scroll);
		scroll_update(&widg_params.scroll);
		break;
	default:
		scroll_ch(&widg_childs.scroll, ch);
		break;
	}
}


/* ****************** */

string widg_param_parsed_to_table_row(param_parsed val) {
	unsigned int width_first =
			widg_params_widths.name + 1 +
			widg_params_widths.type + 1 +
			widg_params_widths.size_value + 1;
	assert(widg_params_widths.total_max >= width_first);

	char *res_str = malloc(widg_params_widths.total_max);
	assert(res_str != NULL);
	char *cur = res_str;
	unsigned int cpy_count;

	cpy_count = MIN(val.name.len, widg_params_widths.name);
	memcpy(cur, val.name.str, cpy_count);
	if (cpy_count < widg_params_widths.name)
		memset(cur+cpy_count, ' ', widg_params_widths.name - cpy_count);
	cur += widg_params_widths.name;
	*cur++ = '|';

	cpy_count = MIN(val.type.len, widg_params_widths.type);
	memcpy(cur, val.type.str, cpy_count);
	if (cpy_count < widg_params_widths.type)
		memset(cur+cpy_count, ' ', widg_params_widths.type - cpy_count);
	cur += widg_params_widths.type;
	*cur++ = '|';

#define uint32_t_max_width 10
	char buf[uint32_t_max_width+1];
	sprintf(buf, "%*u", uint32_t_max_width, val.size_value);
	memcpy(cur, buf+uint32_t_max_width-widg_params_widths.size_value,
			widg_params_widths.size_value);
	cur += widg_params_widths.size_value;
	*cur++ = '|';
#undef uint32_t_max_width

	//assert(cur - res_str == width_first);
	unsigned int rest = widg_params_widths.total_max - width_first;

	cpy_count = MIN(val.value_brief.len, rest);
	memcpy(cur, val.value_brief.str, cpy_count);

	string res;
	res.len = width_first + cpy_count;
	res.str = malloc(res.len);
	assert(res.str != NULL);
	memcpy((char *)res.str, res_str, res.len);
	free(res_str);
	return res;
}

/*void widg_param_parsed_to_table_row_test() {
	param_parsed val = {
		.name = {.str = "name", .len = 4},
		.type = {.str = "type", .len = 4},
		.size_value = 16,
		.value_brief = {.str = "value", .len = 5},
		.ptr = -1
	};
	string res = widg_param_parsed_to_table_row(val);
	string_print(res);
}
*/

void widg_params_init() {
	widg_list_free(&widg_params);

	int height, width;
	getmaxyx(widg_main.wins.params, height, width);
	scroll_struct scroll = {
		.outer_box = widg_main.wins.params,
		.outer_box_width = width,
		.outer_box_height = height,
		.disp_item_idx_first = 0,
		.disp_item_idx_selected = 0,
		.disp_show_select = (state.current_widget == WIDG_PARAMS)
	};

	params_parsed_list list = nk_get_params_parsed_list(state.ptr_nk_current);

	scroll.list.entries = malloc(list.size * sizeof(string));
	assert(scroll.list.entries != NULL);
	scroll.list.size = list.size;
	widg_params.ptr_list.entries = malloc(list.size * sizeof(uint32_t));
	assert(widg_params.ptr_list.entries != NULL);
	widg_params.ptr_list.size = list.size;

	unsigned int i;
	for (i=0; i<list.size; ++i) {
		scroll.list.entries[i] = widg_param_parsed_to_table_row(list.entries[i]);
		widg_params.ptr_list.entries[i] = list.entries[i].ptr;
	}
	params_parsed_list_free(&list);

	widg_params.scroll = scroll;
	scroll_update(&widg_params.scroll);

}

void mode_vk_init();

void widg_params_ch(int ch) {
	switch(ch) {
	case '\n':
		state.current_mode = MODE_VK;
		mode_vk_init();
		break;
	case KEY_NK_CHANGE:
		widg_params_init();
		break;
	case '\t':
		state.current_widget = WIDG_CHILDS;
		widg_childs.scroll.disp_show_select = 1;
		widg_params.scroll.disp_show_select = 0;
		scroll_update(&widg_childs.scroll);
		scroll_update(&widg_params.scroll);
		break;
	default:
		scroll_ch(&widg_params.scroll, ch);
		break;
	}
}

/* ****************** */

void widg_pwd_init() {
	wclear(widg_main.wins.pwd);
	wmove(widg_main.wins.pwd, 0, 0);
	string_list list = nk_get_path_list(state.ptr_nk_current);
	unsigned int i;
	for (i=0; i<list.size; ++i) {
		wprintw(widg_main.wins.pwd, "%.*s/", list.entries[i].len, list.entries[i].str);
		//fprintf(flog, "%.*s/", (unsigned int)list.entries[i].len, list.entries[i].str);
	}
	//fprintf(flog, "\n");
	string_list_free(&list);
	wrefresh(widg_main.wins.pwd);
}

void widg_pwd_ch(int ch) {
	switch(ch) {
	case KEY_NK_CHANGE:
	case KEY_RESIZE:
		widg_pwd_init();
		break;
	default:
		assert(0);
		break;
	}
}

/* ****************** */

void param_print(param_value value, uint32_t vk_type) {
	switch (vk_type) {
	case REG_NONE:
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	default: {
		unsigned int row_size = 32;
		unsigned int i;
		for (i=0; i<value.hex.size; ++i) {
			printw("%02X ", value.hex.data[i]);
			if (!((i+1)%row_size)) printw("\n");
		}
		if (i%row_size) printw("\n");
		break;
	}
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK: {
		printw("%.*s\n", value.str.len, value.str.str);
		break;
	}
	case REG_DWORD:
	case REG_DWORD_BIG_ENDIAN: {
		printw("0x%08X\n", value.dword);
		break;
	}
	case REG_QWORD: {
		printw("0x%016llX\n", (long long unsigned int)value.qword);
		break;
	}
	case REG_MULTI_SZ: {
		unsigned int i;
		for (i=0; i<value.multi_str.size; ++i)
			printw("%.*s\n", value.multi_str.entries[i].len, value.multi_str.entries[i].str);
		break;
	}
	}
}

void mode_vk_init() {
	clear();
	move(0, 0);

	uint32_t ptr = widg_params.ptr_list.entries[widg_params.scroll.disp_item_idx_selected];
	param_parsed_full param = vk_get_parsed(ptr);

	printw("Name: %.*s\n", param.name.len, param.name.str);
	printw("Type: %.*s\n", param.type_str.len, param.type_str.str);
	printw("Size: %u\n", param.size_value);
	printw("Value:\n");
	param_print(param.value, param.type);

	param_parsed_full_free(&param);

	refresh();
}

int mode_vk_ch(int ch) {
	switch (ch) {
	case 'q':
		return 1;
		break;
	case '\n':
		state.current_mode = MODE_MAIN;
		widg_main_ch(KEY_RESIZE);
		break;
	case KEY_RESIZE:
	default:
		break;
	}
	return 0;
}

/* ****************** */

int main(int argc, char **argv) {
flog = fopen("/tmp/debug.log", "w");

	if (argc != 2) {fprintf(stderr, "Usage: %s <regfile path>\n", argv[0]); return 1;}
	uint32_t ptr_root_nk = regfile_init(argv[1]);
	if (ptr_root_nk == ptr_null) return 1;

	setlocale(LC_CTYPE, "");

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	widg_main_init();

	state.ptr_nk_current = ptr_root_nk;
	state.current_widget = WIDG_CHILDS;
	widg_main_ch(KEY_NK_CHANGE);
	widg_main_ch(KEY_RESIZE);

	int ch;
	do {
		ch = getch();
	} while (!modes_ch(ch));

	regfile_uninit();

	endwin();
	
fclose(flog);
	return 0;
}
