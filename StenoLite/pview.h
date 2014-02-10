#include "stdafx.h"
#ifndef MY_PVIEW_H
#define MY_PVIEW_H

#include <list>
#include <vector>
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
	//ULONGLONG starttick;
	//ULONGLONG pausetick;
	int textwidth;
	tstring file;
	HANDLE realtime;
	int lead = 500;
	std::list<singlestroke*> clipboard;
	std::vector<ULONGLONG> filetimes;
	unsigned int currentfile;
};

void PViewNextFocus();
std::list<singlestroke*>::iterator GetItem(int index);
void LaunchProjDlg(HINSTANCE hInst);
void RegisterStroke(unsigned _int8* stroke, int n, const ULONGLONG &thetime);
void RegisterDelete(int n, const ULONGLONG &thetime);
void SetTextSel(unsigned int min, unsigned int max);
ULONGLONG ExposePosition();

extern pdata projectdata;

#endif