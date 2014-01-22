#include "stdafx.h"
#include "DView.h"
#include <Commdlg.h>
#include "globals.h"
#include "resource.h"
#include "stenodata.h"
#include <db.h>
#include <queue>
#include "texthelpers.h"
#include "fileops.h"
#include "setmode.h"

dstruct dviewdata;



HINSTANCE hlocalInst;

dstruct::dstruct() {
	d = NULL;
	dlgwnd = NULL;
	running = false;

	updatingstroke = false;
	lastheight = 0;

	newevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	protectqueue = CreateMutex(NULL, FALSE, NULL);
}

dstruct::~dstruct() {
	CloseHandle(newevent);
	CloseHandle(protectqueue);
}

void ViewShutDown() {
	dviewdata.running = false;
	inputstate.redirect = NULL;
	SetEvent(dviewdata.newevent);
}

void addDVEvent(int e, int repeat) {
	WaitForSingleObject(dviewdata.protectqueue, INFINITE);
	for (int i = 0; i < repeat; i++)
		dviewdata.events.push(e);
	ReleaseMutex(dviewdata.protectqueue);
	SetEvent(dviewdata.newevent);
}

void SetCStrokeText(const TCHAR* text) {
	dviewdata.updatingstroke = true;
	SetWindowText(GetDlgItem(dviewdata.dlgwnd, IDC_CSTROKE), text);
	dviewdata.updatingstroke = false;
}

void SetCTextText(const TCHAR* text) {
	dviewdata.updatingstroke = true;
	SetWindowText(GetDlgItem(dviewdata.dlgwnd, IDC_CTEXT), text);
	dviewdata.updatingstroke = false;
}

void GetListItem(const int &item, unsigned __int8* &strokes, int &numstrokes, std::string &stext, std::string &text) {
	LVITEM itm;
	itm.iItem = item;
	itm.iSubItem = 0;
	TCHAR buffer[260];
	itm.mask = LVIF_TEXT;
	itm.pszText = buffer;
	itm.cchTextMax = 260;

	ListView_GetItem(GetDlgItem(dviewdata.dlgwnd, IDC_VIEW), &itm);

	stext = ttostr(itm.pszText);
	numstrokes = 0;
	strokes = texttomultistroke(stext, numstrokes);
	DB_TXN* trans;
	sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, DB_READ_UNCOMMITTED);
	if (!dviewdata.d->findDItem(strokes, numstrokes * 3, text, trans)) {
		text.clear();
	}
	trans->commit(trans, 0);
}

void Resize(bool bystrokes, unsigned __int8* sdata, int sdatasize, std::string tdata) {
	if (dviewdata.dlgwnd == NULL)
		return;
	if (dviewdata.d == NULL)
		return;

	HWND box = GetDlgItem(dviewdata.dlgwnd, IDC_VIEW);
	dviewdata.displayitems = ListView_GetCountPerPage(box) + 1;
	ListView_DeleteAllItems(box);
	//ListView_SetItemCountEx(box, dviewdata.displayitems, LVSICF_NOSCROLL);
	dviewdata.bystrokes = bystrokes;

	DBT keyin;
	keyin.data = new unsigned __int8[dviewdata.d->longest * 3];
	keyin.size = 0;
	keyin.ulen = dviewdata.d->longest * 3;
	keyin.dlen = 0;
	keyin.doff = 0;
	keyin.flags = DB_DBT_USERMEM;

	DBT strin;
	strin.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strin.size = 0;
	strin.ulen = dviewdata.d->lchars + 1;
	strin.dlen = 0;
	strin.doff = 0;
	strin.flags = DB_DBT_USERMEM;

	DBT strinb;
	strinb.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strinb.size = 0;
	strinb.ulen = dviewdata.d->lchars + 1;
	strinb.dlen = 0;
	strinb.doff = 0;
	strinb.flags = DB_DBT_USERMEM;

	DBC* cursor;
	DB_TXN* trans;
	dviewdata.d->env->txn_begin(dviewdata.d->env, NULL, &trans, DB_READ_UNCOMMITTED);

	int result;

		if (!dviewdata.bystrokes) {
			dviewdata.d->secondary->cursor(sharedData.currentd->secondary, trans, &cursor, 0);
			if (tdata.length() > 0) {
				memset(strin.data, 0, dviewdata.d->lchars + 1);
				memcpy(strin.data, tdata.c_str(), tdata.length() + 1);
				strin.size = tdata.length() + 1;
				result = cursor->pget(cursor, &strin, &keyin, &strinb, DB_SET_RANGE);
			}
			else {
				result = cursor->pget(cursor, &strin, &keyin, &strinb, DB_FIRST);
			}
		}
		else {
			dviewdata.d->contents->cursor(sharedData.currentd->contents, trans, &cursor, 0);
			if (sdatasize > 0) {
				memset(keyin.data, 0, dviewdata.d->longest * 3);
				memcpy(keyin.data, sdata, sdatasize);
				keyin.size = sdatasize;
				result = cursor->get(cursor, &keyin, &strin, DB_SET_RANGE);
			}
			else {
				result = cursor->get(cursor, &keyin, &strin, DB_FIRST);
			}
		}

	TCHAR* text = new TCHAR[dviewdata.d->lchars + 1 + 10];
	TCHAR* strokes = new TCHAR[dviewdata.d->longest * 24 + 1 + 10];

	LVITEM item;
	item.iGroupId = I_GROUPIDNONE;
	item.state = 0;

	unsigned int i = 0;
	while (result == 0 && i < dviewdata.displayitems) {
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_STATE;


		item.iSubItem = 0;
		item.pszText = strokes;

		int sindex = 0;
		for (int n = 0; n * 3 < keyin.size; n++) {
			stroketocsteno(&(((unsigned __int8*)(keyin.data))[n * 3]), &(strokes[sindex]));
			sindex = _tcsnlen(strokes, dviewdata.d->longest * 24 + 1);
			if ((n + 1) * 3 < keyin.size) {
				strokes[sindex] = TEXT('/');
				strokes[sindex + 1] = 0;
				sindex++;
			}

		}

		item.cchTextMax = _tcsnlen(strokes, dviewdata.d->longest * 24 + 1) + 1;

		ListView_InsertItem(box, &item);

		item.mask = LVIF_TEXT;
		item.iSubItem = 1;
		item.pszText = text;
		for (int n = 0; n < strin.size; n++) {
			text[n] = ((char*)(strin.data))[n];
		}
		text[strin.size] = 0;
		item.cchTextMax = _tcsnlen(text, dviewdata.d->lchars + 1 + 1) + 1;

		ListView_SetItem(box, &item);


			if (dviewdata.bystrokes) {
				result = cursor->get(cursor, &keyin, &strin, DB_NEXT);
			}
			else{
				result = cursor->pget(cursor, &strin, &keyin, &strinb, DB_NEXT);
			}

		i++;
	}


	delete text;
	delete strokes;

	cursor->close(cursor);
	trans->commit(trans, 0);

	delete keyin.data;
	delete strin.data;
	delete strinb.data;
}

void PopulateEntries() {
	if (dviewdata.dlgwnd == NULL)
		return;
	if (dviewdata.d == NULL)
		return;

	Resize(true, NULL, 0, "");
}

void Search(bool text) {
	TCHAR* data;
	int len;
	HWND target;
	if (text)
		target = GetDlgItem(dviewdata.dlgwnd, IDC_FTEXT);
	else
		target = GetDlgItem(dviewdata.dlgwnd, IDC_FSTROKE);
	len = GetWindowTextLength(target) + 1;
	data = new TCHAR[len];
	GetWindowText(target, data, len);
	data[len - 1] = 0;

	if (text) {
		Resize(false, NULL, 0, ttostr(data));
	}
	else {
		int numstrokes = 0;
		unsigned __int8* sdata = texttomultistroke(ttostr(data), numstrokes);
		Resize(true, sdata, numstrokes * 3, "");
		delete sdata;
	}

	delete data;
}

/*bool DelCheck(const unsigned __int8* data, const int &size) {
	DBT keyin;
	keyin.data = new unsigned __int8[dviewdata.d->longest * 3];
	keyin.size = 0;
	keyin.ulen = dviewdata.d->longest * 3;
	keyin.dlen = 0;
	keyin.doff = 0;
	keyin.flags = DB_DBT_USERMEM;

	DBT strin;
	strin.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strin.size = 0;
	strin.ulen = dviewdata.d->lchars + 1;
	strin.dlen = 0;
	strin.doff = 0;
	strin.flags = DB_DBT_USERMEM;

	DBT strinb;
	strinb.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strinb.size = 0;
	strinb.ulen = dviewdata.d->lchars + 1;
	strinb.dlen = 0;
	strinb.doff = 0;
	strinb.flags = DB_DBT_USERMEM;

	if (dviewdata.bystrokes) 
		dviewdata.startcursor->get(dviewdata.startcursor, &keyin, &strin, DB_CURRENT);
	else
		dviewdata.startcursor->pget(dviewdata.startcursor, &strin, &keyin, &strinb, DB_CURRENT);
	
	if (keyin.size == size) {
		for (int i = 0; i < size; i++){
			if (((unsigned __int8*)(keyin.data))[i] != data[i])
				return false;
		}
		if (dviewdata.bystrokes)
			dviewdata.startcursor->get(dviewdata.startcursor, &keyin, &strin, DB_NEXT);
		else
			dviewdata.startcursor->pget(dviewdata.startcursor, &strin, &keyin, &strinb, DB_NEXT);
		return true;
	}
	return false;
}*/

void Move(bool up) {
	if (dviewdata.dlgwnd == NULL)
		return;
	if (dviewdata.d == NULL)
		return;

	HWND box = GetDlgItem(dviewdata.dlgwnd, IDC_VIEW);

	DBT keyin;
	keyin.data = new unsigned __int8[dviewdata.d->longest * 3];
	keyin.size = 0;
	keyin.ulen = dviewdata.d->longest * 3;
	keyin.dlen = 0;
	keyin.doff = 0;
	keyin.flags = DB_DBT_USERMEM;

	DBT strin;
	strin.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strin.size = 0;
	strin.ulen = dviewdata.d->lchars + 1;
	strin.dlen = 0;
	strin.doff = 0;
	strin.flags = DB_DBT_USERMEM;

	DBT strinb;
	strinb.data = new unsigned __int8[dviewdata.d->lchars + 1];
	strinb.size = 0;
	strinb.ulen = dviewdata.d->lchars + 1;
	strinb.dlen = 0;
	strinb.doff = 0;
	strinb.flags = DB_DBT_USERMEM;

	DBC* cursor;
	DB_TXN* trans;
	dviewdata.d->env->txn_begin(dviewdata.d->env, NULL, &trans, DB_READ_UNCOMMITTED);

	unsigned __int8* sdata;
	int numstrokes;
	std::string stext;
	std::string text;
	int sz = ListView_GetItemCount(box);
	int direction;
	if (up) {
		GetListItem(0, sdata, numstrokes, stext, text);
		direction = DB_PREV;
	}
	else {
		GetListItem(sz - 1, sdata, numstrokes, stext, text);
		direction = DB_NEXT;
	}

	memset(strin.data, 0, dviewdata.d->lchars + 1);
	memcpy(strin.data, text.c_str(), text.length() + 1);
	strin.size = text.length() + 1;
	memset(keyin.data, 0, dviewdata.d->longest * 3);
	memcpy(keyin.data, sdata, numstrokes * 3);
	keyin.size = numstrokes * 3;
	memset(strinb.data, 0, dviewdata.d->lchars + 1);
	memcpy(strinb.data, text.c_str(), text.length() + 1);
	strinb.size = text.length() + 1;

	int result = -1;
	if (dviewdata.bystrokes) {
		dviewdata.d->contents->cursor(dviewdata.d->contents, trans, &cursor, 0);
		if (cursor->get(cursor, &keyin, &strin, DB_GET_BOTH) == 0) {
			result = cursor->get(cursor, &keyin, &strin, direction);
		}
	}
	else {
		dviewdata.d->secondary->cursor(dviewdata.d->secondary, trans, &cursor, 0);
		if (cursor->pget(cursor, &strin, &keyin, &strinb, DB_GET_BOTH) == 0) {
			result = cursor->pget(cursor, &strin, &keyin, &strinb, direction);
		}
	}
	
	if (result == 0) {
		if (up) {
			if (sz >= dviewdata.displayitems) {
				ListView_DeleteItem(box, sz - 1);
				sz--;
			}
		}
		else {
			ListView_DeleteItem(box, 0);
			sz--;
		}

		TCHAR* text = new TCHAR[dviewdata.d->lchars + 1 + 10];
		TCHAR* strokes = new TCHAR[dviewdata.d->longest * 24 + 1 + 10];

		LVITEM item;
		item.iGroupId = I_GROUPIDNONE;
		item.state = 0;
		if (up)
			item.iItem = 0;
		else
			item.iItem = sz;
		item.iSubItem = 0;
		item.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_STATE;

		item.iSubItem = 0;
		item.pszText = strokes;

		int sindex = 0;
		for (int n = 0; n * 3 < keyin.size; n++) {
			stroketocsteno(&(((unsigned __int8*)(keyin.data))[n * 3]), &(strokes[sindex]));
			sindex = _tcsnlen(strokes, dviewdata.d->longest * 24 + 1);
			if ((n + 1) * 3 < keyin.size) {
				strokes[sindex] = TEXT('/');
				strokes[sindex + 1] = 0;
				sindex++;
			}

		}

		item.cchTextMax = _tcsnlen(strokes, dviewdata.d->longest * 24 + 1) + 1;

		ListView_InsertItem(box, &item);

		item.mask = LVIF_TEXT;
		item.iSubItem = 1;
		item.pszText = text;
		for (int n = 0; n < strin.size; n++) {
			text[n] = ((char*)(strin.data))[n];
		}
		text[strin.size] = 0;
		item.cchTextMax = _tcsnlen(text, dviewdata.d->lchars + 1 + 1) + 1;

		ListView_SetItem(box, &item);

		delete text;
		delete strokes;
	}
	
	
	delete sdata;

	cursor->close(cursor);
	trans->commit(trans, 0);

	delete keyin.data;
	delete strin.data;
	delete strinb.data;
}

void processEvent(unsigned int eventnum) {
	switch (eventnum) {
	case DVE_RESET:
		PopulateEntries();
		break;
	case DVE_UP:
		Move(true);
		break;
	case DVE_DOWN:
		Move(false);
		break;
	case DVE_TEXTSEARCH:
		Search(true);
		break;
	case DVE_STROKESEARCH:
		Search(false);
		break;
	case DVE_RESIZE:
	{
					   unsigned __int8* sdata;
					   int numstrokes;
					   std::string stext;
					   std::string text;
					   GetListItem(0, sdata, numstrokes, stext, text);
					   Resize(dviewdata.bystrokes, sdata, numstrokes * 3, text);
					   delete sdata;
	}
		break;
	default:
		break;
	}
}

DWORD WINAPI processViewEvents(LPVOID lpParam)
{
	while (dviewdata.running) {
		WaitForSingleObject(dviewdata.protectqueue, INFINITE);
		if (!dviewdata.events.empty()) {
			unsigned int val = dviewdata.events.front();
			dviewdata.events.pop();
			ReleaseMutex(dviewdata.protectqueue);
			processEvent(val);
		}
		else {
			ReleaseMutex(dviewdata.protectqueue);
		}

		if (dviewdata.events.empty()) {
			WaitForSingleObject(dviewdata.newevent, INFINITE);
		}
	}
	WaitForSingleObject(dviewdata.protectqueue, INFINITE);
	std::queue<unsigned int>().swap(dviewdata.events);
	ReleaseMutex(dviewdata.protectqueue);
	return 0;
}

void endDVThread() {
	dviewdata.running = false;
	SetEvent(dviewdata.newevent);
}

LRESULT CALLBACK DetectLoseFocus(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_KILLFOCUS:
		inputstate.redirect = NULL;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK KickUpMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_SETFOCUS:
		int rval = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		SetFocus(dviewdata.dlgwnd);
		return rval;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK NewStrokes(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
						  inputstate.sendasstrokes = true;

						  int slen = GetWindowTextLength(GetDlgItem(dviewdata.dlgwnd, IDC_CSTROKE)) + 1;
						  TCHAR* strokes = new TCHAR[slen];
						  GetWindowText(GetDlgItem(dviewdata.dlgwnd, IDC_CSTROKE), strokes, slen);
						  SetDlgItemText(hwndDlg, IDC_NSTROKES, strokes);
						  delete strokes;

						  ShowWindow(hwndDlg, SW_SHOW);
						  SetFocus(GetDlgItem(hwndDlg, IDC_NSTROKES));
						  inputstate.redirect = GetDlgItem(hwndDlg, IDC_NSTROKES);

						  SetDlgItemText(hwndDlg, IDC_NSTAT, TEXT(""));
						  return TRUE;
	}
	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE) {
			if (LOWORD(wParam) == IDC_NSTROKES) {

				tstring stxt = getWinStr(GetDlgItem(hwndDlg, IDC_NSTROKES));
				int numstrokes = 0;
				unsigned __int8* sdata = texttomultistroke(ttostr(stxt), numstrokes);


				std::string result;
				SetDlgItemText(hwndDlg, IDC_NSTAT, TEXT("No existing entry"));
				if (numstrokes >= 1 && dviewdata.d != NULL) {
					DB_TXN* trans;
					sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, DB_READ_UNCOMMITTED);

					if (dviewdata.d->findDItem(sdata, numstrokes * 3, result, trans)) {
						result = std::string("Exists as: ") + result;
						SetDlgItemText(hwndDlg, IDC_NSTAT, strtotstr(result).c_str());
					}

					trans->commit(trans, 0);
				}

				delete sdata;
			}
		}
		else if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam))
			{
			case IDOK:
			{
						 tstring stxt = getWinStr(GetDlgItem(hwndDlg, IDC_NSTROKES));
						 int numstrokes = 0;
						 unsigned __int8* sdata = texttomultistroke(ttostr(stxt), numstrokes);

						 tstring text = getWinStr(GetDlgItem(dviewdata.dlgwnd, IDC_CTEXT));

						 DB_TXN* trans;
						 sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, 0);

						 sharedData.currentd->addNewDItem( sdata, numstrokes * 3, ttostr(text), trans);

						 delete sdata;

						 tstring otxt = getWinStr(GetDlgItem(dviewdata.dlgwnd, IDC_CSTROKE));
						 sdata = texttomultistroke(ttostr(otxt), numstrokes);

						 //DelCheck(sdata, numstrokes * 3);
						 
						 dviewdata.d->deleteDItem(sdata, numstrokes * 3, trans);
						 trans->commit(trans, 0);


						 SetCStrokeText(stxt.c_str());

						 addDVEvent(DVE_RESIZE);

						 delete sdata;

						 inputstate.redirect = NULL;
						 EndDialog(hwndDlg, wParam);
						 return TRUE;
			}
			case IDCANCEL:
				inputstate.redirect = NULL;
				EndDialog(hwndDlg, wParam);
				return TRUE;
			}
		}
	}
	return FALSE;
}

#define MODE_ALL 1
#define MODE_NEW 2
#define MODE_EXJ 3
#define MODE_EXRTF 4

DWORD WINAPI addAll(LPVOID lpParam)
{
	HWND dlg = (HWND)lpParam;
	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = dlg;
	file.lpstrFilter = TEXT("Json Files\0*.json\0RTF Files\0*.rtf\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&file)) {
		//setMode(0);
		if (file.lpstrFile[file.nFileExtension] == TEXT('J') || file.lpstrFile[file.nFileExtension] == TEXT('j')) {
			LoadJson(dviewdata.d, TCHARtostr(file.lpstrFile, MAX_PATH), GetDlgItem(dlg, IDC_PROGRESS), true);
		}
		else {
			LoadRTF(dviewdata.d, TCHARtostr(file.lpstrFile, MAX_PATH), GetDlgItem(dlg, IDC_PROGRESS), true);
		}
		//setMode(settings.mode);
		addDVEvent(DVE_RESET);
	}
	EndDialog(dlg, 0);
	return 0;
}

DWORD WINAPI addNew(LPVOID lpParam)
{
	HWND dlg = (HWND)lpParam;
	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = dlg;
	file.lpstrFilter = TEXT("Json Files\0*.json\0RTF Files\0*.rtf\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&file)) {
		//setMode(0);
		if (file.lpstrFile[file.nFileExtension] == TEXT('J') || file.lpstrFile[file.nFileExtension] == TEXT('j')) {
			LoadJson(dviewdata.d, TCHARtostr(file.lpstrFile, MAX_PATH), GetDlgItem(dlg, IDC_PROGRESS), false);
		}
		else {
			LoadRTF(dviewdata.d, TCHARtostr(file.lpstrFile, MAX_PATH), GetDlgItem(dlg, IDC_PROGRESS), false);
		}
		//setMode(settings.mode);
		addDVEvent(DVE_RESET);
	}
	EndDialog(dlg, 0);
	return 0;
}

DWORD WINAPI exJ(LPVOID lpParam)
{
	HWND dlg = (HWND)lpParam;
	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = dlg;
	file.lpstrFilter = TEXT("Json Files\0*.json\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&file)) {
		std::string filename = TCHARtostr(buffer, MAX_PATH);
		if (filename.find(".json") == std::string::npos) {
			filename += ".json";
		}
		SaveJson(dviewdata.d, filename, GetDlgItem(dlg, IDC_PROGRESS));
	}
	EndDialog(dlg, 0);
	return 0;
}

DWORD WINAPI exRTF(LPVOID lpParam)
{
	HWND dlg = (HWND)lpParam;
	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = dlg;
	file.lpstrFilter = TEXT("RTF Files\0*.rtf\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&file)) {
		std::string filename = TCHARtostr(buffer, MAX_PATH);
		if (filename.find(".rtf") == std::string::npos) {
			filename += ".rtf";
		}
		SaveRTF(dviewdata.d, filename, GetDlgItem(dlg, IDC_PROGRESS));
	}
	EndDialog(dlg, 0);
	return 0;
}

INT_PTR CALLBACK PdlgProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		ShowWindow(hwndDlg, SW_SHOW);
		switch (lParam) {
		case MODE_ALL:
			CreateThread(NULL, 0, addAll, (LPVOID)hwndDlg, 0, NULL);
			break;
		case MODE_NEW:
			CreateThread(NULL, 0, addNew, (LPVOID)hwndDlg, 0, NULL);
			break;
		case MODE_EXJ:
			CreateThread(NULL, 0, exJ, (LPVOID)hwndDlg, 0, NULL);
			break;
		case MODE_EXRTF:
			CreateThread(NULL, 0, exRTF, (LPVOID)hwndDlg, 0, NULL);
			break;
		}
		
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK ViewProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg) {
	
	case WM_PAINT:
		hdc = BeginPaint(hwndDlg, &ps);
		EndPaint(hwndDlg, &ps);
		return TRUE;
	case WM_QUIT:
		endDVThread();
		inputstate.redirect = NULL;
		dviewdata.dlgwnd = NULL;
		return TRUE;
	case WM_CLOSE:
		endDVThread();
		inputstate.redirect = NULL;
		DestroyWindow(dviewdata.dlgwnd);
		dviewdata.dlgwnd = NULL;
		return FALSE;
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_CSTROKE)) {
			return (BOOL)(GetStockObject(WHITE_BRUSH));
		}
		return FALSE;
	case WM_SIZE:
	{
					RECT rt;
					GetClientRect(hwndDlg, &rt);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_FSTROKE), NULL, 10, 15, 170, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_FTEXT), NULL, 10 + 170 + 10, 15, 170, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_UP), NULL, 10, 45, rt.right - rt.left - 20, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_VIEW), NULL,  10, 65, rt.right - rt.left - 20, rt.bottom - rt.top - 120, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_DOWN), NULL, 10, 65 + rt.bottom - rt.top - 120, rt.right - rt.left - 20, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_CSTROKE), NULL,  10, 90 + rt.bottom - rt.top - 120, 170, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_CTEXT), NULL, 10 + 170 + 10, 90 + rt.bottom - rt.top - 120, 170, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_DELETE), NULL, 10+170+10+170+10, 90 + rt.bottom - rt.top - 120, 50, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_NEW), NULL, 10 + 170 + 10 + 170 + 10+50+10, 90 + rt.bottom - rt.top - 120, 50, 20, SWP_NOZORDER);

					if ((rt.bottom - rt.top) - dviewdata.lastheight > 20 || (rt.bottom - rt.top) - dviewdata.lastheight < -20) {
						dviewdata.lastheight = rt.bottom - rt.top;
						addDVEvent(DVE_RESIZE);
					}
	}
		return TRUE;
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hwndDlg, IDC_UP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hlocalInst, MAKEINTRESOURCE(IDI_UP)));
		SendMessage(GetDlgItem(hwndDlg, IDC_DOWN), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hlocalInst, MAKEINTRESOURCE(IDI_DOWN)));
		SetWindowSubclass(GetDlgItem(hwndDlg, IDC_FSTROKE), &DetectLoseFocus, 1239, NULL);
		SetWindowSubclass(GetDlgItem(hwndDlg, IDC_VIEW), &KickUpMouse, 1240, NULL);
		
		dviewdata.running = true;
		addDVEvent(DVE_RESET);
		ViewProc(hwndDlg, WM_SIZE, 0, 0);
		SetFocus(hwndDlg);
		return FALSE;
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDC_VIEW) {
			if (dviewdata.d == NULL)
				return FALSE;
			LPNMHDR hdr = (LPNMHDR)lParam;
			if (hdr->code != NM_CLICK)
				return FALSE;
			LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
			if (lpnmitem->iItem == -1 || lpnmitem->iItem > ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_VIEW))) {
				SetCStrokeText(TEXT(""));
				SetCTextText(TEXT(""));
				return TRUE;
			}

			std::string text;
			std::string stext;
			unsigned __int8* sdata;
			int numstrokes;
			GetListItem(lpnmitem->iItem, sdata, numstrokes, stext, text);
			SetCStrokeText(strtotstr(stext).c_str());
			SetCTextText(strtotstr(text).c_str());

			delete sdata;
			return TRUE;
		}
		return FALSE;
	case WM_MOUSEWHEEL:
	{
						  short distance = 3*(GET_WHEEL_DELTA_WPARAM(wParam));
						  if (distance > 0) {
							  addDVEvent(DVE_UP, distance / WHEEL_DELTA);
						  }
						  else {
							  addDVEvent(DVE_DOWN, (distance*-1) / WHEEL_DELTA);
						  }
	}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_A_N:
			addDVEvent(DVE_DOWN, dviewdata.displayitems - 2);
			return TRUE;
		case ID_A_P:
			addDVEvent(DVE_UP, dviewdata.displayitems - 2);
			return TRUE;
		case IDM_IALL:
			DialogBoxParam(hlocalInst, MAKEINTRESOURCE(IDD_PROCESS), hwndDlg, (DLGPROC)PdlgProc, MODE_ALL);
			return TRUE;
		case IDM_INEW:
			DialogBoxParam(hlocalInst, MAKEINTRESOURCE(IDD_PROCESS), hwndDlg, (DLGPROC)PdlgProc, MODE_NEW);
			return TRUE;
		case IDM_EJSON:
			DialogBoxParam(hlocalInst, MAKEINTRESOURCE(IDD_PROCESS), hwndDlg, (DLGPROC)PdlgProc, MODE_EXJ);
			return TRUE;
		case IDM_ERTF:
			DialogBoxParam(hlocalInst, MAKEINTRESOURCE(IDD_PROCESS), hwndDlg, (DLGPROC)PdlgProc, MODE_EXRTF);
			return TRUE;
		case IDC_UP:
			if (HIWORD(wParam) == BN_CLICKED) {
				addDVEvent(DVE_UP);
				return TRUE;
			}
		case IDC_DOWN:
			if (HIWORD(wParam) == BN_CLICKED) {
				addDVEvent(DVE_DOWN);
				return TRUE;
			}
		case IDC_DELETE:
			if (HIWORD(wParam) == BN_CLICKED) {
				tstring stxt = getWinStr(GetDlgItem(hwndDlg, IDC_CSTROKE));
				int numstrokes = 0;
				unsigned __int8* sdata = texttomultistroke(ttostr(stxt), numstrokes);

				//DelCheck(sdata, numstrokes * 3);
				DB_TXN* trans;
				sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, 0);
				dviewdata.d->deleteDItem( sdata, numstrokes * 3, trans);
				trans->commit(trans, 0);

				delete sdata;

				SetCStrokeText(TEXT(""));
				SetCTextText(TEXT(""));

				addDVEvent(DVE_RESIZE);
				return TRUE;
			}
		case IDC_NEW:
			if (HIWORD(wParam) == BN_CLICKED) {
				PostMessage(controls.main, WM_NEWITEMDLG, 0, 0);
				return TRUE;
			}
		case IDC_FTEXT:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				//
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FTEXT)) >= 1) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_FSTROKE), TEXT(""));
					SetCStrokeText(TEXT(""));
					SetCTextText(TEXT(""));
					addDVEvent(DVE_TEXTSEARCH);
				}
			}
			return FALSE;
		case IDC_FSTROKE:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				inputstate.sendasstrokes = true;
				inputstate.redirect = GetDlgItem(hwndDlg, IDC_FSTROKE);
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FSTROKE)) >= 1) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_FTEXT), TEXT(""));
					SetCStrokeText(TEXT(""));
					SetCTextText(TEXT(""));
					addDVEvent(DVE_STROKESEARCH);
				}
			}
			return FALSE;
		case IDC_CSTROKE:
			if (HIWORD(wParam) == EN_SETFOCUS && !dviewdata.modallaunched) {
				if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CSTROKE)) > 0) {
					dviewdata.modallaunched = true;
					SetFocus(GetDlgItem(hwndDlg, IDC_CTEXT));
					DialogBox(hlocalInst, MAKEINTRESOURCE(IDD_NEWSTROKE), hwndDlg, (DLGPROC)NewStrokes);
					//inputstate.sendasstrokes = false;
					inputstate.redirect = NULL;
					dviewdata.modallaunched = false;
				}
			}
		case IDC_CTEXT:
			if (HIWORD(wParam) == EN_CHANGE && !dviewdata.updatingstroke) {	
				tstring stxt = getWinStr(GetDlgItem(hwndDlg, IDC_CSTROKE));
				int numstrokes = 0;
				unsigned __int8* sdata = texttomultistroke(ttostr(stxt), numstrokes);

				//bool del = DelCheck(sdata, numstrokes*3);

				tstring txt = getWinStr(GetDlgItem(hwndDlg, IDC_CTEXT));

				DB_TXN* trans;
				sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, 0);
				dviewdata.d->addDItem(sdata, numstrokes * 3, ttostr(txt), trans);
				trans->commit(trans, 0);
					
				delete sdata;
				//if (del)
				//	addDVEvent(DVE_UP);
					
				addDVEvent(DVE_RESIZE);
			}
			return FALSE;
		default:
			return FALSE;
		}
	default:
		break;
	}
	return FALSE; // message not processed
}


void LaunchViewDlg(HINSTANCE hInst, dictionary *d) {
	hlocalInst = hInst;
	if (d == NULL) {
		MessageBox(NULL, TEXT("No dictionary selected"), TEXT(""), MB_OK);
		return;
	}
	if (dviewdata.dlgwnd == NULL) {
		dviewdata.dlgwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DICTVIEW), NULL, ViewProc);
		ShowWindow(dviewdata.dlgwnd, SW_SHOW);
		dviewdata.running = true;
		dviewdata.d = d;

		LVCOLUMN clm;
		ListView_SetExtendedListViewStyle(GetDlgItem(dviewdata.dlgwnd, IDC_VIEW), LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		clm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
		clm.fmt = LVCFMT_LEFT;
		clm.cx = 200;
		clm.pszText = TEXT("Strokes");
		clm.cchTextMax = 8;
		clm.iSubItem = 1;
		clm.iOrder = 0;
		SendMessage(GetDlgItem(dviewdata.dlgwnd, IDC_VIEW), LVM_INSERTCOLUMN, 0, (LPARAM)&clm);
		clm.pszText = TEXT("Entry");
		clm.cchTextMax = 6;
		clm.iSubItem = 2;
		clm.iOrder = 1;
		SendMessage(GetDlgItem(dviewdata.dlgwnd, IDC_VIEW), LVM_INSERTCOLUMN, 1, (LPARAM)&clm);
		
		CreateThread(NULL, 0, processViewEvents, NULL, 0, NULL);
	}
	else if (!newwordwin.running) {
		SetCStrokeText(TEXT(""));
		SetCTextText(TEXT(""));
		SetDlgItemText(dviewdata.dlgwnd, IDC_FSTROKE, TEXT(""));
		SetDlgItemText(dviewdata.dlgwnd, IDC_FTEXT, TEXT(""));
		ShowWindow(dviewdata.dlgwnd, SW_SHOW);
		dviewdata.running = true;
		if (dviewdata.d != d){
			dviewdata.d = d;
			CreateThread(NULL, 0, processViewEvents, NULL, 0, NULL);
		}
	}
}