#include "stdafx.h"

#ifndef MY_GLOBALS_H
#define MY_GLOBALS_H

#include <string>
#include "stenodata.h"
#include <stack>
#include <queue>
#include <list>
#include <tuple>
#include <WinUser.h>
#include "texthelpers.h"

#define WM_NEWITEMDLG WM_USER+4
#define WM_LOAD_PROGRESS WM_USER+5

struct cstruct {
	HWND hTab;
	HWND vone;
	HWND vtwo;
	HWND inputs;
	HWND dicts;
	HWND cktop;
	HWND cktrans;
	HWND ckpref;
	HWND ckmistrans;
	HWND main;
	HWND rdfront;
	HWND rdback;
	HWND rdnone;
	HWND bdict;
	HWND bproj;
	HWND scontainer;
	HWND sscroll;
	HWND mestroke;
	HWND mesuggest;
	int width;
	int lineheight;
	int numlines;
	BOOL inited;
	int currenttab;
} ;

struct sstruct {
	BOOL trans;
	BOOL top;
	BOOL mistrans;
	BOOL prefix;
	int space;
	int height;
	int mode;
	int xpos;
	int ypos;
	tstring dict;
	unsigned __int8 map[256];
} ;

struct istruct {
	unsigned __int8 keys[4];
	unsigned __int8 stroke[4];
	HHOOK handle;
	std::stack<tstring> textstack;
	HWND redirect;
	bool sendasstrokes;
} ;

struct  nstruct {
	HWND dlgwnd;
	bool running;
	int focusedcontrol;
	HWND prevfocus;
};


struct singlestroke;
struct textoutput;

struct sdata  {
	BOOL running;
	BOOL addedtext;
	HANDLE newentry;
	HANDLE newtext;
	HANDLE protectqueue;
	std::queue<__int32> inputs;
	std::list<std::tuple<tstring, dictionary*>> dicts;
	std::list<singlestroke*> strokes;
	dictionary* currentd;
	unsigned __int8 number[4];
	BOOL searching;
	ULONGLONG totalprogress;
	ULONGLONG currentprogress;
	bool sevenorbetter;
};


extern cstruct controls;
extern sstruct settings;
extern istruct inputstate;
extern nstruct newwordwin;
extern sdata sharedData;

extern HWND modelesswnd;


struct singlestroke {
	union {
		unsigned __int8 ival[4];
		unsigned __int32 sval;
	} value;
	textoutput* textout;
	singlestroke();
	singlestroke(__int32 s);
	singlestroke(unsigned __int8* i);
};

#define TF_INOSPACE 0x02
#define TF_IPSPACE 0x01
#define TF_ENOSPACE 0x08
#define TF_EPSPACE 0x04
#define TF_CAPNEXT 0x10
#define TF_LOWNEXT 0x20

struct textoutput {
	unsigned __int8 flags;
	tstring text;
	singlestroke* first;
};


#endif