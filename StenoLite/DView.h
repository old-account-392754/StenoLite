#include "stdafx.h"

#ifndef MY_DVIEW_H
#define MY_DVIEW_H

#include "stenodata.h"
#include <db.h>
#include <queue>

#define DVE_RESET 0
#define DVE_UP 1
#define DVE_DOWN 2
#define DVE_TEXTSEARCH 3
#define DVE_STROKESEARCH 4
#define DVE_RESIZE 5

struct dstruct {
	HWND dlgwnd;
	bool running;
	dictionary *d;
	unsigned int displayitems;
	bool bystrokes;
	bool modallaunched;

	dstruct();
	~dstruct();

	HANDLE newevent;
	HANDLE protectqueue;
	std::queue<unsigned int> events;
	bool updatingstroke;
	int lastheight;
};

extern dstruct dviewdata;
void LaunchViewDlg(HINSTANCE hInst, dictionary *d);
void addDVEvent(int e, int repeat = 1);

#endif