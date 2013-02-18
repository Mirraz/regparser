#include <assert.h>
#include <string.h>
#include <cdk.h>

#include "cdk_widgets.h"

static int fselectCB(EObjectType cdktype GCC_UNUSED, void *object,
		void *clientData GCC_UNUSED, chtype key) {
	(void)cdktype;
	(void)clientData;
	(void)key;
	CDKFSELECT *fselect = (CDKFSELECT *)object;
	(void)injectCDKFselect(fselect, KEY_BACKSPACE);
	return TRUE;
}


const char *cdk_file_select(WINDOW *cursesWin, const char *title) {
	CDKSCREEN *cdkscreen = initCDKScreen(cursesWin);
	//initCDKColor();

	CDKFSELECT *fSelect = newCDKFselect(
		cdkscreen,
		CENTER, CENTER,
		20, 65,
		title, "File: ",
		A_NORMAL, ' ', A_REVERSE,
		"</5>", "</48>", "</N>", "</N>",
		TRUE, FALSE
	);
	assert(fSelect != NULL);

	bindCDKObject(vFSELECT, fSelect, 0x7F, fselectCB, NULL);

	const char *fselect = activateCDKFselect(fSelect, NULL);

	const char *res;
	if (fselect != NULL) {
		res = strdup(fselect);
	} else {
		res = NULL;
	}

	if (fSelect->exitType == vESCAPE_HIT) {
		destroyCDKFselect(fSelect);
		destroyCDKScreen(cdkscreen);

		return NULL;
	}

	destroyCDKFselect(fSelect);
	destroyCDKScreen(cdkscreen);

	return res;
}

static int entryCB(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED,
		void *clientData, chtype key) {
	(void)cdktype;
	(void)object;
	(void)clientData;
	CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData;
	(void)injectCDKButtonbox(buttonbox, key);
	return TRUE;
}

static int entryCB2(EObjectType cdktype GCC_UNUSED, void *object,
		void *clientData GCC_UNUSED, chtype key) {
	(void)cdktype;
	(void)clientData;
	(void)key;
	CDKENTRY *entry = (CDKENTRY *)object;
	(void)injectCDKEntry(entry, KEY_BACKSPACE);
	return TRUE;
}

const char *cdk_entry_form(WINDOW *cursesWin, const char *title) {
	CDKSCREEN *cdkscreen = initCDKScreen(cursesWin);
	//initCDKColor();

	CDKENTRY *entry = newCDKEntry(
		cdkscreen,
		CENTER, CENTER,
		title, "Name ",
		A_NORMAL, ' ', vMIXED,
		40, 0, 1024,
		TRUE, FALSE
	);
	assert(entry != NULL);

	const char *buttons[] = {" OK ", " Cancel "};
	CDKBUTTONBOX *buttonWidget = newCDKButtonbox(
			cdkscreen,
			getbegx(entry->win),
			getbegy(entry->win) + entry->boxHeight - 1,
			1, entry->boxWidth - 1,
			0, 1, 2,
			(CDK_CSTRING2)buttons, 2, A_REVERSE,
			TRUE, FALSE
	);
	assert(buttonWidget != NULL);

	setCDKEntryLLChar(entry, ACS_LTEE);
	setCDKEntryLRChar(entry, ACS_RTEE);
	setCDKButtonboxULChar(buttonWidget, ACS_LTEE);
	setCDKButtonboxURChar(buttonWidget, ACS_RTEE);

	bindCDKObject(vENTRY, entry, KEY_TAB, entryCB, buttonWidget);
	bindCDKObject(vENTRY, entry, 0x7F, entryCB2, NULL);

	drawCDKButtonbox(buttonWidget, TRUE);
	const char *info = copyChar(activateCDKEntry(entry, NULL));
	int selection = buttonWidget->currentButton;

	destroyCDKButtonbox(buttonWidget);
	destroyCDKEntry(entry);
	destroyCDKScreen(cdkscreen);

	if (selection != 0) {
		freeChar((char *)info);
		info = NULL;
	}
	return info;
}
