#include "stdafx.h"
#ifndef MY_PVIEW_H
#define MY_PVIEW_H

#include <list>
#include "globals.h"
#include "texthelpers.h"

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
	bool settingsel = false;
	int textwidth;
	tstring file;
	HANDLE realtime;
	std::list<singlestroke*> clipboard;
};

void PViewNextFocus();
std::list<singlestroke*>::iterator GetItem(int index);
std::list<singlestroke*>::iterator GetItemByText(unsigned int textindex);
void LaunchProjDlg(HINSTANCE hInst);
void RegisterStroke(unsigned _int8* stroke, int n);
void RegisterDelete(int n);
void SetTextSel(unsigned int min, unsigned int max);

extern pdata projectdata;

#endif