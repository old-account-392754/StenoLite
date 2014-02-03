#include "stdafx.h"

#include "pview.h"
#include "globals.h"
#include "texthelpers.h"
#include "resource.h"
#include <Richedit.h>
#include <list>
#include <db.h>
#include <Windowsx.h>
#include <Commdlg.h>
#include "fileops.h"
#include <vector>
#include "pstroke.h"

pdata projectdata;
#define STROKES  "#####STROKES#####"

void saveProject(const tstring &file) {
	static tstring sbuffer;
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		writeBOM(hfile);

		DB_TXN* trans;
		projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, DB_READ_UNCOMMITTED);

		DBC* startcursor;

		DBT keyin;
		keyin.data = new unsigned __int8[projectdata.d->longest * 3];
		keyin.size = 0;
		keyin.ulen = projectdata.d->longest * 3;
		keyin.dlen = 0;
		keyin.doff = 0;
		keyin.flags = DB_DBT_USERMEM;

		DBT strin;
		strin.data = new unsigned __int8[projectdata.d->lchars + 1];
		strin.size = 0;
		strin.ulen = projectdata.d->lchars + 1;
		strin.dlen = 0;
		strin.doff = 0;
		strin.flags = DB_DBT_USERMEM;

		projectdata.d->contents->cursor(projectdata.d->contents, trans, &startcursor, 0);
		int result = startcursor->get(startcursor, &keyin, &strin, DB_FIRST);

		while (result == 0) {
			tstring acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc, sharedData.currentd->format);
			for (unsigned int i = 1; i * 3 < keyin.size; i++) {
				acc += TEXT("/");
				tstring tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i * 3]), tmp, sharedData.currentd->format);
				acc += tmp;
			}
			writestr(hfile, ttostr(acc));
			writestr(hfile, " 0 ");
			writestr(hfile, (char*)(strin.data));
			writestr(hfile, "\r\n");

			result = startcursor->get(startcursor, &keyin, &strin, DB_NEXT);
		}

		writestr(hfile, STROKES);
		writestr(hfile, "\r\n");

		startcursor->close(startcursor);
		trans->commit(trans, 0);


		for (auto it = projectdata.strokes.cbegin(); it != projectdata.strokes.cend(); it++) {
			if ((*it)->textout->first == (*it)) {
				sbuffer.clear();
				stroketocsteno((*it)->value.ival, sbuffer, sharedData.currentd->format);
				writestr(hfile, ttostr(sbuffer));
				writestr(hfile, " ");
				writestr(hfile, std::to_string((*it)->textout->flags));
				writestr(hfile, " ");
				writestr(hfile, ttostr(escapestr((*it)->textout->text)));
				writestr(hfile, "\r\n");
			}
			else {
				sbuffer.clear();
				stroketocsteno((*it)->value.ival, sbuffer, sharedData.currentd->format);
				writestr(hfile, ttostr(sbuffer));
				writestr(hfile, "\r\n");
			}
		}

		CloseHandle(hfile);
	}
}


DWORD CALLBACK StreamOutCallback(_In_  DWORD_PTR dwCookie, _In_  LPBYTE pbBuff, _In_  LONG cb, _In_  LONG *pcb)
{
	HANDLE* pFile = (HANDLE*)dwCookie;
	DWORD dwW;
	WriteFile(*pFile, pbBuff, cb, &dwW, NULL);
	*pcb = cb;
	return 0;
}

void SaveText(HWND hWnd)
{

	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = NULL;
	file.lpstrFilter = TEXT("Text Files\0*.txt\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&file)) {
		tstring filename = buffer;
		if (filename.find(TEXT(".txt")) == std::string::npos) {
			filename += TEXT(".txt");
		}

		HANDLE hfile = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hfile != INVALID_HANDLE_VALUE) {
			EDITSTREAM es;
			es.dwError = 0;
			es.dwCookie = (DWORD)&hfile;
			es.pfnCallback = StreamOutCallback;

			SendMessage(hWnd, EM_STREAMOUT, (WPARAM)SF_TEXT, (LPARAM)&es);

			CloseHandle(hfile);
		}
	}
	

}

void RegisterDelete(int n) {
	if (n < 0)
		return;

	if (projectdata.realtime != NULL) {
		writestr(projectdata.realtime, "\r\nD ");
		writestr(projectdata.realtime, std::to_string(n));
	}
}

void RegisterDef(tstring stroke, tstring val) {
	if (projectdata.realtime != NULL) {
		writestr(projectdata.realtime, "\r\nX ");
		writestr(projectdata.realtime, ttostr(stroke));
		writestr(projectdata.realtime, " ");
		writestr(projectdata.realtime, ttostr(val));
	}
}

void RegisterStroke(unsigned _int8* stroke, int n) {
	if (n < 0)
		return;

	if (projectdata.realtime != NULL) {
		tstring buffer;
		stroketocsteno(stroke, buffer, sharedData.currentd->format);
		writestr(projectdata.realtime, "\r\n");
		writestr(projectdata.realtime, std::to_string(n));
		writestr(projectdata.realtime, " ");
		writestr(projectdata.realtime, ttostr(buffer));
	}
}

struct stroke {
	unsigned __int8 sval[4];
};

void delat(std::vector<stroke> &slist, int n) {
	if (n < 0 || n > slist.size() - 1)
		return;

	auto it = slist.begin();
	it += n;
	slist.erase(it);
}

void addat(std::vector<stroke> &slist, int n, unsigned __int8* str) {
	if (n < 0 || n > slist.size())
		return;

	stroke toadd;
	toadd.sval[0] = str[0];
	toadd.sval[1] = str[1];
	toadd.sval[2] = str[2];

	auto it = slist.begin();
	it += n;
	slist.insert(it, toadd);
}

void reRealtime() {
	WaitForSingleObject(sharedData.lockprocessing, INFINITE);

	CloseHandle(projectdata.realtime);

	tstring realtime = projectdata.file;
	realtime = realtime.replace(realtime.find(TEXT(".prj")), 4, TEXT(".srf"));

	projectdata.realtime = CreateFile(realtime.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	if (projectdata.realtime != INVALID_HANDLE_VALUE) {

		DB_TXN* trans;
		projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, DB_READ_UNCOMMITTED);

		DBC* startcursor;

		DBT keyin;
		keyin.data = new unsigned __int8[projectdata.d->longest * 3];
		keyin.size = 0;
		keyin.ulen = projectdata.d->longest * 3;
		keyin.dlen = 0;
		keyin.doff = 0;
		keyin.flags = DB_DBT_USERMEM;

		DBT strin;
		strin.data = new unsigned __int8[projectdata.d->lchars + 1];
		strin.size = 0;
		strin.ulen = projectdata.d->lchars + 1;
		strin.dlen = 0;
		strin.doff = 0;
		strin.flags = DB_DBT_USERMEM;

		projectdata.d->contents->cursor(projectdata.d->contents, trans, &startcursor, 0);
		int result = startcursor->get(startcursor, &keyin, &strin, DB_FIRST);

		while (result == 0) {
			tstring acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc, sharedData.currentd->format);
			for (unsigned int i = 1; i * 3 < keyin.size; i++) {
				acc += TEXT("/");
				tstring tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i * 3]), tmp, sharedData.currentd->format);
				acc += tmp;
			}
			writestr(projectdata.realtime, "\r\nX ");
			writestr(projectdata.realtime, ttostr(acc));
			writestr(projectdata.realtime, " ");
			writestr(projectdata.realtime, (char*)(strin.data));

			result = startcursor->get(startcursor, &keyin, &strin, DB_NEXT);
		}

		startcursor->close(startcursor);
		trans->commit(trans, 0);

		for (auto i = projectdata.strokes.cbegin(); i != projectdata.strokes.cend(); i++) {
			RegisterStroke((*i)->value.ival, 0);
		}
	}

	ReleaseMutex(sharedData.lockprocessing);
}

void LoadRealtime() {
	const static std::regex parsefile("^(\\S+?)\\s(.*)$");
	const static std::regex parseX("^(\\S+?)\\s(.*)$");
	std::cmatch m;
	std::cmatch n;
	
	WaitForSingleObject(sharedData.lockprocessing, INFINITE);

	std::vector<stroke> slist;

	char c;
	DWORD bytes;
	std::string cline("");

	int totalb = 0;
	ReadFile(projectdata.realtime, &c, 1, &bytes, NULL);
	while (bytes > 0) {
		totalb++;
		if (c != '\r')
			cline += c;
		std::string::size_type r = cline.find("\n");
		if (r != std::string::npos) {
			cline.erase(r, 1);
			if (std::regex_match(cline.c_str(), m, parsefile)) {
				if (m[1].str().compare("X") == 0) {
					std::string temp = m[2].str();
					if (std::regex_match(temp.c_str(), n, parseX)) {
						int numstrokes = 0;
						unsigned __int8* sdata = texttomultistroke(strtotstr(n[1].str()), numstrokes, sharedData.currentd->format);

						DB_TXN* trans;
						projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, 0);
						projectdata.d->addDItem(sdata, numstrokes * 3, n[2].str(), trans);
						trans->commit(trans, 0);

						MessageBoxA(NULL, n[2].str().c_str(), n[1].str().c_str(), MB_OK);

						delete sdata;
					}
				}
				else if (m[1].str().compare("D") == 0) {
					delat(slist, atoi(m[2].str().c_str()));
				}
				else {
					unsigned __int8 sbuf[4];
					textToStroke(strtotstr(m[2].str()), sbuf, sharedData.currentd->format);
					addat(slist, atoi(m[1].str().c_str()), sbuf);

					//MessageBox(NULL, TEXT("ADD"), TEXT("ADD"), MB_OK);
				}
			}
			
			cline.clear();

		}
		ReadFile(projectdata.realtime, &c, 1, &bytes, NULL);
	}

	if (std::regex_match(cline.c_str(), m, parsefile)) {
		if (m[1].str().compare("X") == 0) {
			std::string temp = m[2].str();
			if (std::regex_match(temp.c_str(), m, parseX)) {
				int numstrokes = 0;
				unsigned __int8* sdata = texttomultistroke(strtotstr(m[1].str()), numstrokes, sharedData.currentd->format);

				DB_TXN* trans;
				projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, 0);
				projectdata.d->addDItem(sdata, numstrokes * 3, m[2].str(), trans);
				trans->commit(trans, 0);

				delete sdata;
			}
		}
		else if (m[1].str().compare("D") == 0) {
			delat(slist, atoi(m[2].str().c_str()));
		}
		else {
			unsigned __int8 sbuf[4];
			textToStroke(strtotstr(m[2].str()), sbuf, sharedData.currentd->format);
			addat(slist, atoi(m[1].str().c_str()), sbuf);
		}
	}
	HANDLE rtback = projectdata.realtime;
	projectdata.realtime = NULL;
	for (auto i = slist.cbegin(); i != slist.cend(); i++) {
		unsigned __int8 temp[3];
		temp[0] = (*i).sval[0];
		temp[1] = (*i).sval[1];
		temp[2] = (*i).sval[2];
		processSingleStroke(&(temp[0]));
	}
	projectdata.realtime = rtback;

	ReleaseMutex(sharedData.lockprocessing);
}

void ProcessItem(std::string &cline, textoutput* &last, bool &matchdict, DB_TXN* trans) {
	const static std::regex parsefile("^(\\S+?)\\s(\\S+?)\\s(.*)$");
	std::cmatch m;

	if (std::regex_match(cline.c_str(), m, parsefile)) {
		if (matchdict) {
			int len = 0;
			unsigned __int8* strokes = texttomultistroke(strtotstr(m[1].str()), len, sharedData.currentd->format);
			projectdata.d->addDItem(strokes, len*3, m[3].str(), trans);
			delete strokes;
		}
		else {
			unsigned __int8 stroke[4];
			textToStroke(strtotstr(m[1].str()), stroke, sharedData.currentd->format);
			singlestroke* s = new singlestroke(stroke);
			s->textout = new textoutput();
			s->textout->flags = atoi(m[2].str().c_str());
			s->textout->text = unescapestr(strtotstr(m[3].str()));
			s->textout->first = s;
			projectdata.strokes.push_back(s);
			last = s->textout;

			TCHAR buffer[32] = TEXT("\r\n");
			stroketosteno(s->value.ival, &buffer[2], sharedData.currentd->format);
			
			CHARRANGE crngb;
			crngb.cpMin = crngb.cpMax = 23;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crngb);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)buffer);
			
			
			crngb.cpMin = crngb.cpMax = 0;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crngb);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)(s->textout->text.c_str()));
		}
	}
	else {
		if (cline.compare(STROKES) == 0) {
			matchdict = false;
		}
		else if (cline.length() > 0) {
			unsigned __int8 stroke[4];
			textToStroke(strtotstr(cline), stroke, sharedData.currentd->format);
			singlestroke* s = new singlestroke(stroke);
			s->textout = last;
			projectdata.strokes.push_back(s);

			TCHAR buffer[32] = TEXT("\r\n");
			stroketosteno(s->value.ival, &buffer[2], sharedData.currentd->format);

			CHARRANGE crngb;
			crngb.cpMin = crngb.cpMax = 23;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crngb);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)buffer);
		}
	}

}

void exDict()
{
	OPENFILENAME file;
	TCHAR buffer[MAX_PATH] = TEXT("\0");
	memset(&file, 0, sizeof(OPENFILENAME));
	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = projectdata.dlg;
	file.lpstrFilter = TEXT("Json Files\0*.json\0\0");
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&file)) {
		tstring filename = buffer;
		if (filename.find(TEXT(".json")) == std::string::npos) {
			filename += TEXT(".json");
		}
		SaveJson(projectdata.d, filename, NULL);
	}
}

bool loadProject(const tstring &file) {
	WaitForSingleObject(sharedData.lockprocessing, INFINITE);
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	
	bool matchdict = true;

	if (hfile != INVALID_HANDLE_VALUE) {

		char c;
		DWORD bytes;
		std::string cline("");

		//detect and erase BOM
		unsigned __int8 bom[3] = "\0\0";
		ReadFile(hfile, bom, 3, &bytes, NULL);
		if (bom[0] != 239 || bom[1] != 187 || bom[2] != 191) {
			cline += bom[0];
			cline += bom[1];
			cline += bom[2];
			std::string::iterator end_pos = std::remove_if(cline.begin(), cline.end(), isReturn);
			cline.erase(end_pos, cline.end());

		}
		DB_TXN* trans = NULL;
		projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, DB_TXN_BULK);
		textoutput* last = NULL;

		int totalb = 0;
		ReadFile(hfile, &c, 1, &bytes, NULL);
		while (bytes > 0) {
			totalb++;
			if (c != '\r')
				cline += c;
			std::string::size_type r = cline.find("\n");
			if (r != std::string::npos) {
				cline.erase(r, 1);
				ProcessItem(cline, last, matchdict, trans);
				cline.clear();

			}
			ReadFile(hfile, &c, 1, &bytes, NULL);
		}

		ProcessItem(cline, last, matchdict, trans);

		trans->commit(trans, 0);
		CloseHandle(hfile);

		CHARRANGE crngb;
		crngb.cpMin = crngb.cpMax = GetWindowTextLength(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST));
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crngb);
		crngb.cpMin = crngb.cpMax = GetWindowTextLength(GetDlgItem(projectdata.dlg, IDC_MAINTEXT));
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crngb);

		ReleaseMutex(sharedData.lockprocessing);
		return true;
	}
	ReleaseMutex(sharedData.lockprocessing);
	return false;
}

void PViewNextFocus() {
	if (projectdata.addingnew) {
		switch (projectdata.focusedcontrol) {
		case 0:  projectdata.focusedcontrol = 1; inputstate.sendasstrokes = false; inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PENTRY);
			SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		case 1:  projectdata.focusedcontrol = 2; SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(projectdata.dlg, IDC_POK), TRUE); break;
		case 2:  projectdata.focusedcontrol = 3; SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(projectdata.dlg, IDC_PCANCEL), TRUE); break;
		default: projectdata.focusedcontrol = 0; inputstate.sendasstrokes = true;  inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PSTROKE);
			SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		}
	}
}

std::list<singlestroke*>::iterator GetItem(int index) {
	int iindex = 0;

	auto it = (projectdata.strokes.end());
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (iindex == index) {
			return it;
		}
		iindex++;
	}

	return it;
}

int GetTextFromItem(const std::list<singlestroke*>::const_iterator &item) {

	int pos = 0;

	if (item == projectdata.strokes.cend())
		return 0;

	if (projectdata.strokes.size() > 0){
		auto it = (--projectdata.strokes.cend());
		for (; it != projectdata.strokes.cbegin(); it--) {
			if ((*it)->textout->first == *it)
				pos += (*it)->textout->text.length();
			if (it == item)
				return pos;
		}
	}
	pos += (*projectdata.strokes.cbegin())->textout->text.length();
	return pos;
}

void SetTextSel(unsigned int min, unsigned int max) {
	CHARRANGE crnew;
	unsigned int index = 1;
	projectdata.selectionmax = max;
	projectdata.selectionmin = min;


	if (max == 0) {
		crnew.cpMax = 0;
	}
	if (min == 0) {
		crnew.cpMin = 0;
	}

	if (projectdata.strokes.size() > 0){
	
		auto it = (--projectdata.strokes.cend());
		int pos = 0;
		for (; it != projectdata.strokes.cbegin(); it--) {
			if (index == max) {
				crnew.cpMax = pos + (*it)->textout->text.length();
			}
			if (index == min) {
				crnew.cpMin = pos + (*it)->textout->text.length();
			}
			if ((*it)->textout->first == *it)
				pos += (*it)->textout->text.length();
			index++;
		}
		if (index <= max) {
			crnew.cpMax = pos + (*it)->textout->text.length();
		}
		if (index <= min) {
			crnew.cpMin = pos + (*it)->textout->text.length();
		}
	}

	SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, 0, (LPARAM)&crnew);
}

std::list<singlestroke*>::iterator GetItemByText(unsigned int textindex) {
	int iindex = projectdata.strokes.size();
	if (iindex == 0) {
		return projectdata.strokes.end();
	}


	auto it = (--projectdata.strokes.end());
	int pos = 0;
	for (; it != projectdata.strokes.cbegin(); it--) {
		
			if (textindex < pos + ((*it)->textout->text.length()+1) / 2) {
				it++;
				return it;
			}

		if ((*it)->textout->first == *it) {
			pos += (*it)->textout->text.length();
		}
		iindex--;
	}

	if (it == projectdata.strokes.cbegin()) {
		if (textindex < pos + ((*it)->textout->text.length()+1) / 2) {
			it++;
			return it;
		}
		else {
			return it;
		}
	}

	return it;
}

int StrokeFromTextIndx(unsigned int txtindex) {
	unsigned int index = 1;
	if (projectdata.strokes.size() == 0) {
		return 0;
	}

	auto it = (--projectdata.strokes.cend());
	int pos = 0;
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (txtindex < pos + ((*it)->textout->text.length()+1) / 2) {
			return index - 1;
		}

		if ((*it)->textout->first == *it) {
			pos += (*it)->textout->text.length();
		}
		index++;
	}
	if (it == projectdata.strokes.cbegin()) {
		if (txtindex < pos + ((*it)->textout->text.length()+1) / 2) {
			return index - 1;
		}
		else {
			return index;
		}
	}

	return index;
}



LRESULT CALLBACK StrokeList(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int initialchr = 0;
	static bool capturing = false;

	switch (uMsg) {
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		return 0;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		POINTL p;
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		initialchr = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_CHARFROMPOS, 0, (LPARAM)(&p));
		capturing = true;
		//return 0;
	case WM_MOUSEMOVE:
		if (!capturing)
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	{
			POINTL p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			int cchar = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_CHARFROMPOS, 0, (LPARAM)(&p));
			 
			int lesser;
			int greater;
			if (initialchr > cchar) {
				lesser = cchar;
				greater = initialchr;
			}
			else {
				lesser = initialchr;
				greater = cchar;
			}
						 projectdata.settingsel = true;
						 int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, lesser);
						 int lineb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, greater);

						 //protect against selecting in the middle of a word
						 auto it = GetItem(line);
						 if (it != projectdata.strokes.cend()) {
							 while ((*it)->textout->first != (*it)) {
								 it--;
								 line++;
							 }
						 }

						 if (line > lineb)
							 lineb = line;

						 if (lineb != line) {
							 it = GetItem(lineb);
							 if (it != projectdata.strokes.cend()) {
								 while ((*it)->textout->first != (*it)) {
									 it--;
									 lineb++;
								 }
							 }
						 }

						 int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
						 int lineindexb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
						 CHARRANGE crnew;
						 crnew.cpMin = lineindex + 23;
						 crnew.cpMax = lineindexb + 23;
						 SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
						 SetTextSel(line, lineb);
						 projectdata.settingsel = false;
	}
		return 0;
	case WM_LBUTTONUP:
		capturing = false;
		ReleaseCapture();
		return 0;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK MainText(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int initialchr = 0;
	static bool capturing = false;

	switch (uMsg) {
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		return 0;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		POINTL p;
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		initialchr = SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_CHARFROMPOS, 0, (LPARAM)(&p));
		capturing = true;
		//return 0;
	case WM_MOUSEMOVE:
		if (!capturing)
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		{
			POINTL p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			int cchar = SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_CHARFROMPOS, 0, (LPARAM)(&p));

			int lesser;
			int greater;
			if (initialchr > cchar) {
				lesser = cchar;
				greater = initialchr;
			}
			else {
				lesser = initialchr;
				greater = cchar;
			}
			projectdata.settingsel = true;
			int line = StrokeFromTextIndx(lesser);
			int lineb = StrokeFromTextIndx(greater);
			int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
			int lineindexb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
			CHARRANGE crnew;
			crnew.cpMin = lineindex + 23;
			crnew.cpMax = lineindexb + 23;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
			SetTextSel(line, lineb);

			projectdata.settingsel = false;
		}
		return 0;
	case WM_LBUTTONUP:
		capturing = false;
		ReleaseCapture();
		return 0;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void ShowBatch(int show) {
	ShowWindow(GetDlgItem(projectdata.dlg, IDC_PENTRY), show);
	ShowWindow(GetDlgItem(projectdata.dlg, IDC_PSTROKE), show);
	ShowWindow(GetDlgItem(projectdata.dlg, IDC_POK), show);
	ShowWindow(GetDlgItem(projectdata.dlg, IDC_PCANCEL), show);
	ShowWindow(GetDlgItem(projectdata.dlg, IDC_PNEW), show);
}

INT_PTR CALLBACK PViewProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	NMHDR* hdr;

	switch (uMsg) {
	case WM_ACTIVATE:
		if (wParam == 0) {
			modelesswnd = NULL;
			projectdata.open = false;
		}
		else {
			modelesswnd = hwndDlg;
			projectdata.open = true;
		}
		return FALSE;
	case WM_SIZE:
	{
					RECT rt;
					GetClientRect(hwndDlg, &rt);
					if (projectdata.addingnew) {
						SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKELIST), NULL, 0, 0, projectdata.textwidth, rt.bottom - rt.top-30, SWP_NOZORDER);
						SetWindowPos(GetDlgItem(hwndDlg, IDC_MAINTEXT), NULL, projectdata.textwidth, 0, rt.right - rt.left - projectdata.textwidth, rt.bottom - rt.top-30, SWP_NOZORDER);
					}
					else {
						SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKELIST), NULL, 0, 0, projectdata.textwidth, rt.bottom - rt.top, SWP_NOZORDER);
						SetWindowPos(GetDlgItem(hwndDlg, IDC_MAINTEXT), NULL, projectdata.textwidth, 0, rt.right - rt.left - projectdata.textwidth, rt.bottom - rt.top, SWP_NOZORDER);
					}
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PNEW), NULL, 0, rt.bottom-rt.top - 30, rt.right - rt.left, 30, SWP_NOZORDER);

					SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKE), GetDlgItem(hwndDlg, IDC_PNEW), 5, rt.bottom - rt.top - 25, 160, 20, 0);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PENTRY), GetDlgItem(hwndDlg, IDC_PNEW), 170, rt.bottom - rt.top - 25, 160, 20, 0);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_POK), GetDlgItem(hwndDlg, IDC_PNEW), 335, rt.bottom - rt.top - 26, 30, 22, 0);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PCANCEL), GetDlgItem(hwndDlg, IDC_PNEW), 370, rt.bottom - rt.top - 26, 45, 22, 0);
	}
		return TRUE;
	case WM_QUIT:
		projectdata.open = false;
		projectdata.dlg = NULL;
		inputstate.redirect = NULL;
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(projectdata.dlg);
		projectdata.open = false;
		projectdata.dlg = NULL;
		inputstate.redirect = NULL;
		saveProject(projectdata.file);
		projectdata.d->close();
		CloseHandle(projectdata.realtime);
		return FALSE;
	case WM_INITDIALOG:
	{
						  
						  projectdata.dlg = hwndDlg;
						  projectdata.d = new dictionary("");
						  tstring form;
						  if (sharedData.currentd != NULL)
							  form = sharedData.currentd->format;
						  else
							  form = TEXT("#STKPWHRAO*EUFRPBLGTSDZ");

						  if (!projectdata.d->opentransient(form))
							  MessageBox(NULL, TEXT("Unable to open project's dictionary"), TEXT("Error"), MB_OK);

						 ShowBatch(SW_HIDE);
						 
						  PARAFORMAT2 pf;
						  memset(&pf, 0, sizeof(pf));
						  pf.cbSize = sizeof(PARAFORMAT2);
						  pf.dwMask = PFM_OFFSETINDENT | PFM_SPACEAFTER | PFM_OFFSET;
						  pf.dxStartIndent = (24 * 1440) / 72;
						  pf.dySpaceAfter = (5 * 1440) / 72;
						  pf.dxOffset = -pf.dxStartIndent;
						  SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETPARAFORMAT, 0, (LPARAM)&pf);

						  CHARFORMAT2 cf;
						  memset(&cf, 0, sizeof(cf));
						  cf.cbSize = sizeof(CHARFORMAT2);
						  cf.dwMask = CFM_FACE | CFM_SIZE;
						  cf.yHeight = (10 * 1440) / 72;
						  _tcscpy_s(cf.szFaceName, LF_FACESIZE, TEXT("Consolas"));

						  if (SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf) == 0) {
							  _tcscpy_s(cf.szFaceName, LF_FACESIZE, TEXT("Courier New"));
							  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						  }

						  memset(&cf, 0, sizeof(cf));
						  cf.cbSize = sizeof(CHARFORMAT2);
						  cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
						  cf.yHeight = (settings.fsize * 1440) / 72;
						  cf.wWeight = settings.fweight;
						  _tcscpy_s(cf.szFaceName, LF_FACESIZE, settings.fname.c_str());

						  SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

						  SetWindowText(GetDlgItem(hwndDlg, IDC_PSTROKELIST), TEXT("                       "));
						  CHARRANGE crnew;
						  crnew.cpMax = 23;
						  crnew.cpMin = 23;
						  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
						  //SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETEVENTMASK, 0, ENM_SELCHANGE);
						  //SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETEVENTMASK, 0, ENM_SELCHANGE);

						  SetWindowSubclass(GetDlgItem(hwndDlg, IDC_PSTROKELIST), &StrokeList, 1245, NULL);
						  SetWindowSubclass(GetDlgItem(hwndDlg, IDC_MAINTEXT), &MainText, 1246, NULL);

						  projectdata.strokes.clear();


						  projectdata.addingnew = false;
						  projectdata.open = true;
						  projectdata.selectionmax = 0;
						  projectdata.selectionmin = 0;
						  projectdata.cursorpos = 0;

						  if (projectdata.file.find(TEXT(".srf")) != std::string::npos) {
							  projectdata.realtime = CreateFile(projectdata.file.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
							  
							  LoadRealtime();

							  projectdata.file = projectdata.file.replace(projectdata.file.find(TEXT(".srf")), 4, TEXT(".prj"));
						  }
						  else {
							  loadProject(projectdata.file);
							  //open realtime file

							  tstring realtime = projectdata.file;
							  realtime = realtime.replace(realtime.find(TEXT(".prj")), 4, TEXT(".srf"));

							  projectdata.realtime = CreateFile(realtime.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
							  if (projectdata.realtime != INVALID_HANDLE_VALUE) {
									SetFilePointer(projectdata.realtime, 0, NULL, FILE_END);
							  }
						  }

						  POINTL pt;
						  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_POSFROMCHAR, (WPARAM)&pt, 1);
						  projectdata.textwidth = pt.x * 23;

						  PViewProc(hwndDlg, WM_SIZE, 0, 0);

						  CHARRANGE crngb;
						  crngb.cpMin = crngb.cpMax = GetWindowTextLength(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST));
						  SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crngb);
						  crngb.cpMin = crngb.cpMax = GetWindowTextLength(GetDlgItem(projectdata.dlg, IDC_MAINTEXT));
						  SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crngb);

						  SetFocus(GetDlgItem(projectdata.dlg, IDC_MAINTEXT));
	}
		return FALSE;
	case WM_NOTIFY:
		hdr = (NMHDR*)lParam;
		
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PSTROKE:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				projectdata.focusedcontrol = 0;
				inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PSTROKE);
				inputstate.sendasstrokes = true;
			}
			break;
		case IDC_PENTRY:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				projectdata.focusedcontrol = 1;
				inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PENTRY);
				inputstate.sendasstrokes = false;
			}
			break;
		case IDM_RECREATE:
			reRealtime();
			break;
		case IDM_EXDICT:
			exDict();
			break;
		case IDM_PSTROKEEX:
			SaveText(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST));
			break;
		case IDM_PTEXTEX:
			SaveText(GetDlgItem(projectdata.dlg, IDC_MAINTEXT));
			break;
		case IDC_POK:
		{
						SetFocus(GetDlgItem(hwndDlg, IDC_MAINTEXT));
						ShowBatch(SW_HIDE);
						projectdata.addingnew = false;
						inputstate.redirect = NULL;

						tstring txt = getWinStr(GetDlgItem(hwndDlg, IDC_PENTRY));
						tstring strke = getWinStr(GetDlgItem(hwndDlg, IDC_PSTROKE));
						int numstrokes = 0;

						unsigned __int8* sdata = texttomultistroke(strke, numstrokes, sharedData.currentd->format);
						std::string trmd = trimstr(ttostr(txt), " ");

						RegisterDef(strke, strtotstr(trmd));

						DB_TXN* trans;
						projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, 0);
						projectdata.d->addDItem(sdata, numstrokes * 3, trmd, trans);
						trans->commit(trans, 0);

						delete sdata;
						PViewProc(hwndDlg, WM_SIZE, 0, 0);

						union {
							unsigned __int8 sval[4];
							unsigned __int32 ival;
						} tstroke;
						memcpy(tstroke.sval, sharedData.currentd->sdelete, 3);
						addStroke(tstroke.ival);
		}
			break;
		case IDC_PCANCEL:
			SetFocus(GetDlgItem(hwndDlg, IDC_MAINTEXT));
			ShowBatch(SW_HIDE);
			projectdata.addingnew = false;
			inputstate.redirect = NULL;
			PViewProc(hwndDlg, WM_SIZE, 0, 0);

			union {
				unsigned __int8 sval[4];
				unsigned __int32 ival;
			} tstroke;
			memcpy(tstroke.sval, sharedData.currentd->sdelete, 3);
			addStroke(tstroke.ival);

			break;
		case IDM_PFONT:
			CHOOSEFONT cf;
			memset(&cf, 0, sizeof(cf));
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = hwndDlg;
			cf.Flags =  CF_NOSCRIPTSEL | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
			cf.rgbColors = RGB(0, 0, 0);

			LOGFONT lf;
			memset(&lf, 0, sizeof(lf));
			HDC screen = GetDC(NULL);
			lf.lfHeight = -MulDiv(settings.fsize, GetDeviceCaps(screen, LOGPIXELSY), 72);
			ReleaseDC(NULL, screen);
			lf.lfWeight = settings.fweight;

			_tcscpy_s(lf.lfFaceName, LF_FACESIZE, settings.fname.c_str());
			cf.lpLogFont = &lf;

			if (ChooseFont(&cf)) {
				settings.fsize = cf.iPointSize / 10;
				settings.fweight = lf.lfWeight;
				settings.fname = lf.lfFaceName;

				CHARFORMAT2 cf;
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
				cf.yHeight = (settings.fsize * 1440) / 72;
				cf.wWeight = settings.fweight;
				_tcscpy_s(cf.szFaceName, LF_FACESIZE, settings.fname.c_str());

				SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
			}
			break;
		}

		return TRUE;
	case WM_NEWITEMDLG:
		SetWindowText(GetDlgItem(projectdata.dlg, IDC_PSTROKE), TEXT(""));
		SetWindowText(GetDlgItem(projectdata.dlg, IDC_PENTRY), TEXT(""));
		ShowBatch(SW_SHOW);

		projectdata.focusedcontrol = 0;
		inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PSTROKE);
		inputstate.sendasstrokes = true;
		//SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE);
		SetFocus(inputstate.redirect);
		projectdata.addingnew = true;
		PViewProc(hwndDlg, WM_SIZE, 0, 0);
		return TRUE;
	}
	
	return FALSE;
	}


void LaunchProjDlg(HINSTANCE hInst) {
	if (sharedData.currentd == NULL) {
		MessageBox(NULL, TEXT("No dictionary selected"), TEXT(""), MB_OK);
		return;
	}
	if (projectdata.dlg == NULL) {

		OPENFILENAME file;
		TCHAR buffer[MAX_PATH] = TEXT("\0");
		memset(&file, 0, sizeof(OPENFILENAME));
		file.lStructSize = sizeof(OPENFILENAME);
		file.hwndOwner = NULL;
		file.lpstrFilter = TEXT("Project Files\0*.prj\0Stroke Record Files\0*.srf\0\0");
		file.nFilterIndex = 1;
		file.lpstrFile = buffer;
		file.nMaxFile = MAX_PATH;
		file.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
		if (GetOpenFileName(&file)) {
			tstring filename = buffer;
			if (filename.find(TEXT(".srf")) != std::string::npos) {
				//filename += TEXT(".rtf");
			}
			else if (filename.find(TEXT(".prj")) != std::string::npos) {

			}
			else {
				filename += TEXT(".prj");
			}
			projectdata.file = filename;

			projectdata.dlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PROJECT), NULL, PViewProc);
		
			ShowWindow(projectdata.dlg, SW_SHOW);
			SetForegroundWindow(projectdata.dlg);
		}
	}
}