#include "stdafx.h"
#ifndef MY_PVIEW_H
#define MY_PVIEW_H

#include <list>

struct pdata {
	bool open;
	int cursorpos;
	std::list<singlestroke*> strokes;
	HWND dlg;
	int selectionmin;
	int selectionmax;
};

struct indexedtext : public textoutput {
	int startingindex;
};

extern pdata projectdata;

#endif