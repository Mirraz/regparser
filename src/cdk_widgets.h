#ifndef CDK_WIDGETS_H_
#define CDK_WIDGETS_H_

#include <ncurses.h>

const char *cdk_file_select(WINDOW *cursesWin, const char *title);
const char *cdk_entry_form(WINDOW *cursesWin, const char *title);

#endif /* CDK_WIDGETS_H_ */
