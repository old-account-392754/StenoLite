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
	bool paused = true;
	bool reloading = false;
	bool autoplayback = false;
	ULONGLONG starttick;
	ULONGLONG pausetick;
	ULONGLONG exisistingtime = 0;
	int textwidth;
	tstring file;
	HANDLE realtime;
	double speed = 1.0;
	int lead = 1000;
	std::list<singlestroke*> clipboard;
};

void PViewNextFocus();
std::list<singlestroke*>::iterator GetItem(int index);
std::list<singlestroke*>::iterator GetItemByText(unsigned int textindex);
void LaunchProjDlg(HINSTANCE hInst);
void RegisterStroke(unsigned _int8* stroke, int n, const time_t &thetime);
void RegisterDelete(int n, const time_t &thetime);
void SetTextSel(unsigned int min, unsigned int max);

extern pdata projectdata;

#endif