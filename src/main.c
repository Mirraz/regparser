#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <locale.h>
#include <ncurses.h>

#include "widgets.h"
#include "regfile.h"
#include "parse_common.h"

FILE *fout;
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

scroll_struct widg_childs;
scroll_struct widg_params;

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
			//widg_childs_ch(ch);
			break;
		case WIDG_PARAMS:
			//widg_params_ch(ch);
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

int main() {   // int argc, char **argv) {
flog = fopen("/tmp/debug.log", "w");

	setlocale(LC_CTYPE, "");

	initscr();
	cbreak();
	noecho();

	widg_main_init();

	widg_main_ch(KEY_RESIZE);

	int ch;
	do {
		ch = getch();
	} while (!widg_main_ch(ch));

	endwin();
	
fclose(flog);
	return 0;
}

