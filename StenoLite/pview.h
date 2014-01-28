#include "stdafx.h"
#ifndef MY_PVIEW_H
#define MY_PVIEW_H

#include <list>
#include "globals.h"

struct pdata {
	bool open;
	bool addingnew;
	int focusedcontrol;
	int cursorpos;
	std::list<singlestroke*> strokes;
	HWND dlg;
	int selectionmin;
	int selectionmax;
	dictionary *d;
};

struct indexedtext : public textoutput {
	unsigned int startingindex;
};

void PViewNextFocus();
std::list<singlestroke*>::iterator GetItem(int index);
void AdjustTextStart(std::list<singlestroke*>::iterator last, int adjustment);
std::list<singlestroke*>::iterator GetItemByText(unsigned int textindex);

extern pdata projectdata;

#endif