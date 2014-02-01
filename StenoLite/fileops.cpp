#include "stdafx.h"
#include "fileops.h"

#include "globals.h"
#include "stenodata.h"
#include <tuple>
#include <iostream>
#include <regex>
#include <list>
#include <string>
#include "texthelpers.h"
#include "addendings.h"
#include <utility>
#include "setmode.h"
#include <algorithm>

TCHAR pathbuffer[MAX_PATH];

bool isReturn(char value) {
	return value == '\r' || value =='\n';
}

void writestr(HANDLE hfile, const std::string& data) {
	DWORD bytes;
	WriteFile(hfile, data.c_str(), data.length(), &bytes, NULL);
}

void writeBOM(HANDLE hfile) {
	static const unsigned __int8 bom[3] = { 239, 187, 191 };
	DWORD bytes;
	WriteFile(hfile, bom, 3, &bytes, NULL);
}


tstring GetDictDir() {
	DWORD len = MAX_PATH;
	const static tregex rx(TEXT("\\\\[^\\\\]*$"));

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\STENOLITE\\DICTDIR"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)pathbuffer, &len) == ERROR_SUCCESS) {
			return pathbuffer;
		}
		RegCloseKey(hkey);
	}

	GetModuleFileName(NULL, pathbuffer, MAX_PATH);
	return std::regex_replace(pathbuffer, rx, TEXT("\\"));
}

void saveSettings() {

	int trans = 0;
	int top = 0;
	int mtrans = 0;
	int prefix = 0;

	if (settings.trans == TRUE)
		trans = 1;
	if (settings.top == TRUE)
		top = 1;
	if (settings.mistrans == TRUE)
		mtrans = 1;
	if (settings.prefix == TRUE)
		prefix = 1;


	
	DWORD len = MAX_PATH;
	tstring res = GetDictDir();

	
	tstring file = res + TEXT("settings");

	HANDLE hfile = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		writeBOM(hfile);
		writestr(hfile, "HEIGHT = ");
		writestr(hfile, std::to_string(settings.height));
		writestr(hfile, "\r\n");

		if (settings.trans)
			writestr(hfile, "TRANS = 1\r\n");
		else
			writestr(hfile, "TRANS = 0\r\n");

		if (settings.top)
			writestr(hfile, "TOP = 1\r\n");
		else
			writestr(hfile, "TOP = 0\r\n");

		if (settings.prefix)
			writestr(hfile, "PREFIX = 1\r\n");
		else
			writestr(hfile, "PREFIX = 0\r\n");

		if (settings.mistrans)
			writestr(hfile, "MTRANS = 1\r\n");
		else
			writestr(hfile, "MTRANS = 0\r\n");

		writestr(hfile, "MODE = ");
		writestr(hfile, std::to_string(settings.mode));
		writestr(hfile, "\r\n");

		writestr(hfile, "SPACE = ");
		writestr(hfile, std::to_string(settings.space));
		writestr(hfile, "\r\n");

		writestr(hfile, "DICT = ");
		writestr(hfile, ttostr(settings.dict));
		writestr(hfile, "\r\n");

		writestr(hfile, "X = ");
		writestr(hfile, std::to_string(settings.xpos));
		writestr(hfile, "\r\n");

		writestr(hfile, "Y = ");
		writestr(hfile, std::to_string(settings.ypos));
		writestr(hfile, "\r\n");

		writestr(hfile, "FSIZE = ");
		writestr(hfile, std::to_string(settings.fsize));
		writestr(hfile, "\r\n");

		writestr(hfile, "FWEIGHT = ");
		writestr(hfile, std::to_string(settings.fweight));
		writestr(hfile, "\r\n");

		writestr(hfile, "FNAME = ");
		writestr(hfile, ttostr(settings.fname));
		writestr(hfile, "\r\n");

		HKL locale = GetKeyboardLayout(GetCurrentThreadId());
		for (int i = 0; i < 256; i++){
			if (settings.map[i] != 0) {
				if (i == LOBYTE(VK_F1)) {
					writestr(hfile, "MAP = f1->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F2)) {
					writestr(hfile, "MAP = f2->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F3)) {
					writestr(hfile, "MAP = f3->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F4)) {
					writestr(hfile, "MAP = f4->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F5)) {
					writestr(hfile, "MAP = f5->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F6)) {
					writestr(hfile, "MAP = f6->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F7)) {
					writestr(hfile, "MAP = f7->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F8)) {
					writestr(hfile, "MAP = f8->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F9)) {
					writestr(hfile, "MAP = f9->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F10)) {
					writestr(hfile, "MAP = f10->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F11)) {
					writestr(hfile, "MAP = f11->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else if (i == LOBYTE(VK_F12)) {
					writestr(hfile, "MAP = f12->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
				else {
					TCHAR c = MapVirtualKeyEx(i, MAPVK_VK_TO_CHAR, locale);
					if (c == TEXT(' ')) {
						c = TEXT('_');
					}
					TCHAR tbuf[2];
					tbuf[0] = c;
					tbuf[1] = 0;
					writestr(hfile, "MAP = ");
					writestr(hfile, ttostr(tbuf));
					writestr(hfile, "->");
					writestr(hfile, std::to_string(settings.map[i]));
					writestr(hfile, "\r\n");
				}
			}
		}

		CloseHandle(hfile);
	}
}

void saveDictSettings(dictionary* d) {
	static tstring sbuffer;
	HANDLE hfile = CreateFile(d->settingslocation.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		writeBOM(hfile);
		writestr(hfile, "FORMAT = ");
		writestr(hfile, ttostr(d->format));
		writestr(hfile, "\r\n");

		writestr(hfile, "LONGEST = ");
		writestr(hfile, std::to_string(d->longest));
		writestr(hfile, "\r\n");

		writestr(hfile, "LONGTEXT = ");
		writestr(hfile, std::to_string(d->lchars));
		writestr(hfile, "\r\n");

		if (d->extras) {
			writestr(hfile, "EXTRAS = 1\r\n");
		}
		else {
			writestr(hfile, "EXTRAS = 0\r\n");
		}

		writestr(hfile, "DELETE = ");
		sbuffer.clear();
		stroketocsteno(d->sdelete, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "TAB = ");
		sbuffer.clear();
		stroketocsteno(d->stab, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "RETURN = ");
		sbuffer.clear();
		stroketocsteno(d->sreturn, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "NUMBER = ");
		sbuffer.clear();
		stroketocsteno(d->number, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "CUT = ");
		sbuffer.clear();
		stroketocsteno(d->scut, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "COPY = ");
		sbuffer.clear();
		stroketocsteno(d->scopy, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "PASTE = ");
		sbuffer.clear();
		stroketocsteno(d->spaste, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "RPROCESS = ");
		sbuffer.clear();
		stroketocsteno(d->sreprocess, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "LEFT = ");
		sbuffer.clear();
		stroketocsteno(d->sleft, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "RIGHT = ");
		sbuffer.clear();
		stroketocsteno(d->sright, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "SLEFT = ");
		sbuffer.clear();
		stroketocsteno(d->sshleft, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "SRIGHT = ");
		sbuffer.clear();
		stroketocsteno(d->sshright, sbuffer, d->format);
		writestr(hfile, ttostr(sbuffer));
		writestr(hfile, "\r\n");

		writestr(hfile, "ITEMS = ");
		writestr(hfile, std::to_string(d->items));
		writestr(hfile, "\r\n");

		for (auto it = d->suffix.cbegin(); it != d->suffix.cend(); it++) {
			writestr(hfile, "SUFFIX = ");
			union {
				unsigned __int8 sval[4];
				unsigned __int32 ival;
			} tstroke;
			tstroke.ival = (*it).first;
			sbuffer.clear();
			stroketocsteno(tstroke.sval, sbuffer, d->format);
			writestr(hfile, ttostr(sbuffer));
			writestr(hfile, "->");
			writestr(hfile, suftostr((*it).second));
			writestr(hfile, "\r\n");
		}

		CloseHandle(hfile);
	}
}



void strtodsetting(dictionary* d, const std::string& setting, const std::string& value) {
	const static std::regex map("\\s*(\\S+)->(\\S+)\\s*");
	if (setting.compare("LONGEST")==0) {
		d->longest = std::atoi(value.c_str());
	}
	else if(setting.compare("LONGTEXT")==0) {
		d->lchars = std::atoi(value.c_str());
	}
	else if (setting.compare("ITEMS") == 0) {
		d->items = std::atoi(value.c_str());
	}
	else if (setting.compare("DELETE") == 0) {
		textToStroke(strtotstr(value), d->sdelete, d->format);
	}
	else if (setting.compare("TAB") == 0) {
		textToStroke(strtotstr(value), d->stab, d->format);
	}
	else if (setting.compare("FORMAT") == 0) {
		d->format = strtotstr(value);
		while (d->format.length() < 23) {
			d->format += 'X';
		}
	}
	else if (setting.compare("RETURN") == 0) {
		textToStroke(strtotstr(value), d->sreturn, d->format);
	}
	else if (setting.compare("NUMBER") == 0) {
		textToStroke(strtotstr(value), d->number, d->format);
	}
	else if (setting.compare("CUT") == 0) {
		textToStroke(strtotstr(value), d->scut, d->format);
	}
	else if (setting.compare("COPY") == 0) {
		textToStroke(strtotstr(value), d->scopy, d->format);
	}
	else if (setting.compare("PASTE") == 0) {
		textToStroke(strtotstr(value), d->spaste, d->format);
	}
	else if (setting.compare("RPROCESS") == 0) {
		textToStroke(strtotstr(value), d->sreprocess, d->format);
	}
	else if (setting.compare("RIGHT") == 0) {
		textToStroke(strtotstr(value), d->sright, d->format);
	}
	else if (setting.compare("LEFT") == 0) {
		textToStroke(strtotstr(value), d->sleft, d->format);
	}
	else if (setting.compare("SRIGHT") == 0) {
		textToStroke(strtotstr(value), d->sshright, d->format);
	}
	else if (setting.compare("SLEFT") == 0) {
		textToStroke(strtotstr(value), d->sshleft, d->format);
	}
	else if (setting.compare("EXTRAS") == 0) {
		if (std::atoi(value.c_str()) == 1) {
			d->extras = TRUE;
		}
	}
	else if (setting.compare("SUFFIX") == 0) {
		std::cmatch m;
		if (std::regex_match(value.c_str(), m, map)) {
			union {
				unsigned __int8 sval[4];
				unsigned __int32 ival;
			} tstroke;

			textToStroke(strtotstr(m[1].str()), tstroke.sval, d->format);
			int sufx = strtosuf(m[2].str());
			d->suffix.push_back(std::make_pair(tstroke.ival, sufx));
		}
	}
}

void loadDictSettings(dictionary* d, const tstring& file) {
	d->format = TEXT("#STKPWHRAO*EUFRPBLGTSDZ");
	textToStroke(tstring(TEXT("*")), d->sdelete, d->format);
	textToStroke(tstring(TEXT("*")), d->number, d->format);
	textToStroke(tstring(TEXT("T-B")), d->stab, d->format);
	textToStroke(tstring(TEXT("R-RPB")), d->sreturn, d->format);
	textToStroke(tstring(TEXT("-")), d->scut, d->format);
	textToStroke(tstring(TEXT("-")), d->scopy, d->format);
	textToStroke(tstring(TEXT("-")), d->spaste, d->format);
	textToStroke(tstring(TEXT("-")), d->sreprocess, d->format);
	textToStroke(tstring(TEXT("-")), d->sleft, d->format);
	textToStroke(tstring(TEXT("-")), d->sright, d->format);
	textToStroke(tstring(TEXT("-")), d->sshleft, d->format);
	textToStroke(tstring(TEXT("-")), d->sshright, d->format);

	d->longest = 1;
	d->lchars = 1;
	d->items = 0;
	d->settingslocation = file;
	d->extras = FALSE;
	

	HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex property("^\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");

	std::cmatch m;

	

	if (hfile != INVALID_HANDLE_VALUE) {
		char c;
		DWORD bytes;
		std::string cline("");

		//detect and erase BOM
		///239, 187, 191
		unsigned __int8 bom[3] = "\0\0";
		ReadFile(hfile, bom, 3, &bytes, NULL);
		if (bom[0] != 239 || bom[1] != 187 || bom[2] != 191) {
			cline += bom[0];
			cline += bom[1];
			cline += bom[2];
			std::string::iterator end_pos = std::remove_if(cline.begin(), cline.end(), isReturn);
			cline.erase(end_pos, cline.end());

		}

		ReadFile(hfile, &c, 1, &bytes, NULL);
		while (bytes > 0) {
			cline += c;
			std::string::size_type r = cline.find("\r\n");
			if (r != std::string::npos) {
				cline.erase(r, 2);
				if (std::regex_match(cline.c_str(), m, property)) {
					strtodsetting(d, m[1].str(), m[2].str());
				}
				cline.clear();
			}
			ReadFile(hfile, &c, 1, &bytes, NULL);
		}

		if (std::regex_match(cline.c_str(), m, property)) {
			strtodsetting(d, m[1].str(), m[2].str());
		}
		CloseHandle(hfile);
	}
}

void addentry(dictionary* d, const tstring &stroke, const std::string &text, DB_TXN* trans, bool overwrite = false) {
	int numstrokes = countStrokes(stroke, stroke.length());
	unsigned __int8* sdata = new unsigned __int8[numstrokes * 3];

	tstring::const_iterator it = stroke.cbegin();
	int i = 0;
	while (it != stroke.cend()) {
		textToStroke(sdata + i * 3, it, stroke.cend(), d->format);
		i++;
	}

	if (overwrite)
		d->addDItem(sdata, numstrokes * 3, text, trans);
	else
		d->addNewDItem(sdata, numstrokes * 3, text, trans);

	delete sdata;
}

void appendUser(dictionary* d, const tstring& stroke, const std::string& text) {
	const static tregex rx(TEXT("\\\\[^\\\\]*$"));
	tstring file = std::regex_replace(d->settingslocation, rx, TEXT("\\user"));
	
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		SetFilePointer(hfile, 0, NULL, FILE_END);
		writestr(hfile, "\r\n\"");
		writestr(hfile, ttostr(stroke));
		writestr(hfile, "\": \"");
		writestr(hfile, text);
		writestr(hfile, "\"");

		CloseHandle(hfile);
	}
}
//{\rtf1\ansi{\*\cxrev100}\cxdict{\*\cxsystem SL}{\stylesheet{\s0 Normal;}}

void SaveRTF(dictionary* d, const tstring &file, HWND progress) {
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		if (progress) {
			SendMessage(progress, PBM_SETRANGE32, 0, d->items);
			SendMessage(progress, PBM_SETPOS, 0, 0);
		}

		sharedData.totalprogress = d->items;
		sharedData.currentprogress = 0;
		PostMessage(controls.main, WM_LOAD_PROGRESS, 1, 0);

		DB_TXN* trans;
		d->env->txn_begin(d->env, NULL, &trans, DB_READ_UNCOMMITTED);
		DBC* startcursor;

		DBT keyin;
		keyin.data = new unsigned __int8[d->longest * 3];
		keyin.size = 0;
		keyin.ulen = d->longest * 3;
		keyin.dlen = 0;
		keyin.doff = 0;
		keyin.flags = DB_DBT_USERMEM;

		DBT strin;
		strin.data = new unsigned __int8[d->lchars + 1];
		strin.size = 0;
		strin.ulen = d->lchars + 1;
		strin.dlen = 0;
		strin.doff = 0;
		strin.flags = DB_DBT_USERMEM;

		writestr(hfile, "{\\rtf1\\ansi{\\*\\cxrev100}\\cxdict{\\*\\cxsystem StenoLite}{\\stylesheet{\\s0 Normal;}}\n");
		d->contents->cursor(d->contents, NULL, &startcursor, 0);
		int result = startcursor->get(startcursor, &keyin, &strin, DB_FIRST);

		while (result == 0) {
			writestr(hfile, "{\\*\\cxs ");
			tstring acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc, d->format);
			for (unsigned int i = 1; i * 3 < keyin.size; i++) {
				acc += TEXT("/");
				tstring tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i * 3]), tmp, d->format);
				acc += tmp;
			}
			writestr(hfile, ttostr(acc));
			writestr(hfile, "}");
			writestr(hfile, (char*)(strin.data));
			writestr(hfile, "\n");

			result = startcursor->get(startcursor, &keyin, &strin, DB_NEXT);
			SendMessage(progress, PBM_DELTAPOS, 1, 0);

			sharedData.currentprogress++;
			PostMessage(controls.main, WM_LOAD_PROGRESS, 2, 0);
		}
		startcursor->close(startcursor);
		trans->commit(trans, 0);

		writestr(hfile, "}");

		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
}


void SaveJson(dictionary* d, const tstring &file, HWND progress) {
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		if (progress) {
			SendMessage(progress, PBM_SETRANGE32, 0, d->items);
			SendMessage(progress, PBM_SETPOS, 0, 0);
		}
		sharedData.totalprogress = d->items;
		sharedData.currentprogress = 0;
		PostMessage(controls.main, WM_LOAD_PROGRESS, 1, 0);

		writeBOM(hfile);

		DB_TXN* trans;
		d->env->txn_begin(d->env, NULL, &trans, DB_READ_UNCOMMITTED);

		DBC* startcursor;

		DBT keyin;
		keyin.data = new unsigned __int8[d->longest * 3];
		keyin.size = 0;
		keyin.ulen = d->longest * 3;
		keyin.dlen = 0;
		keyin.doff = 0;
		keyin.flags = DB_DBT_USERMEM;

		DBT strin;
		strin.data = new unsigned __int8[d->lchars + 1];
		strin.size = 0;
		strin.ulen = d->lchars + 1;
		strin.dlen = 0;
		strin.doff = 0;
		strin.flags = DB_DBT_USERMEM;

		writestr(hfile, "{\r\n");
		d->contents->cursor(d->contents, trans, &startcursor, 0);
		int result = startcursor->get(startcursor, &keyin, &strin, DB_FIRST);

		while (result == 0) {
			writestr(hfile, "\"");
			tstring acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc, d->format);
			for (unsigned int i = 1; i * 3 < keyin.size; i++) {
				acc += TEXT("/");
				tstring tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i * 3]), tmp, d->format);
				acc += tmp;
			}
			writestr(hfile, ttostr(acc));
			writestr(hfile, "\": \"");
			writestr(hfile, (char*)(strin.data));

			result = startcursor->get(startcursor, &keyin, &strin, DB_NEXT);
			if (result == 0) 
				writestr(hfile, "\",\r\n");
			else
				writestr(hfile, "\"\r\n");

			if (progress)
				SendMessage(progress, PBM_DELTAPOS, 1, 0);

			sharedData.currentprogress++;
			PostMessage(controls.main, WM_LOAD_PROGRESS, 2, 0);
		}
	
		writestr(hfile, "}");

		startcursor->close(startcursor);
		trans->commit(trans, 0);

		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
}

void LoadJson(dictionary* d, const tstring &file, HWND progress, bool overwrite) {
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex parsejson("^\\\"(.+?)\\\"\\: \\\"(.*)\\\",?\\s*$");
	std::cmatch m;


	if (hfile != INVALID_HANDLE_VALUE) {
		DWORD fsize = GetFileSize(hfile, NULL);
		if (progress) {
			SendMessage(progress, PBM_SETRANGE32, 0, fsize);
			SendMessage(progress, PBM_SETPOS, 0, 0);
		}
		sharedData.totalprogress = fsize;
		sharedData.currentprogress = 0;
		PostMessage(controls.main, WM_LOAD_PROGRESS, 1, 0);

		DB_TXN* trans = NULL;
		d->env->txn_begin(d->env, NULL, &trans, DB_TXN_BULK);

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

		int totalb = 0;
		ReadFile(hfile, &c, 1, &bytes, NULL);
		while (bytes > 0) {
			totalb++;
			if (c != '\r')
				cline += c;
			std::string::size_type r = cline.find("\n");
			if (r != std::string::npos) {
				cline.erase(r, 1);
				if (std::regex_match(cline.c_str(), m, parsejson)) {
					addentry(d, strtotstr(m[1].str()), m[2].str(), trans, overwrite );
				}
				cline.clear();
				if (progress) {
					SendMessage(progress, PBM_DELTAPOS, totalb, 0);
				}
				sharedData.currentprogress += totalb;
				PostMessage(controls.main, WM_LOAD_PROGRESS, 2, 0);
				totalb = 0;
			}
			ReadFile(hfile, &c, 1, &bytes, NULL);
		}

		if (std::regex_match(cline.c_str(), m, parsejson)) {
			addentry(d, strtotstr(m[1].str()), m[2].str(), trans, overwrite);
		}

		trans->commit(trans, 0);

		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
 
}

void LoadRTF(dictionary* d, const tstring &file, HWND progress, bool overwrite) {
	//{\*\cxs STROKETEXT}ENTRYTEXT
	HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex parsertf(".*\\{\\\\\\*\\\\cxs (.+?)\\}(.+)$");
	std::cmatch m;



	if (hfile != INVALID_HANDLE_VALUE) {
		DWORD fsize = GetFileSize(hfile, NULL);
		if (progress) {
			SendMessage(progress, PBM_SETRANGE32, 0, fsize);
			SendMessage(progress, PBM_SETPOS, 0, 0);
		}
		sharedData.totalprogress = fsize;
		sharedData.currentprogress = 0;
		PostMessage(controls.main, WM_LOAD_PROGRESS, 1, 0);

		DB_TXN* trans = NULL;
		d->env->txn_begin(d->env, NULL, &trans, DB_TXN_BULK);

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

		int totalb = 0;
		ReadFile(hfile, &c, 1, &bytes, NULL);
		while (bytes > 0) {
			totalb++;
			if (c != '\r')
				cline += c;
			std::string::size_type r = cline.find("\n");
			if (r != std::string::npos) {
				cline.erase(r, 1);
				if (std::regex_match(cline.c_str(), m, parsertf)) {
					addentry(d, strtotstr(m[1].str()), m[2].str(), trans, overwrite);
				}
				cline.clear();
				if (progress) {
					SendMessage(progress, PBM_DELTAPOS, totalb, 0);
				}
				sharedData.currentprogress += totalb;
				PostMessage(controls.main, WM_LOAD_PROGRESS, 2, 0);
				totalb = 0;
			}
			ReadFile(hfile, &c, 1, &bytes, NULL);
		}

		if (std::regex_match(cline.c_str(), m, parsertf)) {
			addentry(d, strtotstr(m[1].str()), m[2].str(), trans, overwrite);
		}

		trans->commit(trans, 0);
		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
}

std::list<tstring> EnumDicts() {

	DWORD len = MAX_PATH;
	tstring res = GetDictDir();

	std::list<tstring> results;

	tstring filename = res + TEXT("*");

	WIN32_FIND_DATA FindFileData;

	int found = 0;

	HANDLE hFind = FindFirstFileEx(filename.c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && tstring(TEXT(".")).compare(FindFileData.cFileName) != 0 && tstring(TEXT("..")).compare(FindFileData.cFileName) != 0) {
			results.push_back(tstring(FindFileData.cFileName));
			found++;
		}
		if (!FindNextFile(hFind, &FindFileData))
		{
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	if (found == 0) {
		MessageBox(NULL, (tstring(TEXT("No dictionary directories found in: ")) + res).c_str(), TEXT("Error"), MB_OK);
	}

	return results;
}

void loadDictionaries() {
	DWORD len = MAX_PATH;
	tstring root = GetDictDir();


		tstring filename = root +TEXT("*");

		WIN32_FIND_DATA FindFileData;

		HANDLE hFind = FindFirstFileEx(filename.c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
		while (hFind != INVALID_HANDLE_VALUE)
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && tstring(TEXT(".")).compare(FindFileData.cFileName) != 0 && tstring(TEXT("..")).compare(FindFileData.cFileName) != 0) {
				tstring dir(FindFileData.cFileName);
				tstring compiled = root + dir + TEXT("\\bin");
				
				if (GetFileAttributes(compiled.c_str()) != INVALID_FILE_ATTRIBUTES) {

					dictionary* d = new dictionary(ttostr((root + dir)).c_str());

					loadDictSettings(d, (root + dir + TEXT("\\settings.txt")));


					//if (OpenDictionary(d, dir)) {
					if (d->open("bin", "bin2")) {
						sharedData.dicts.push_front(std::tuple<tstring, dictionary*>(dir, d));
						if (settings.dict.compare(dir) == 0) {
							setDictionary(d);
						}
					}
					else {
						MessageBox(NULL, (tstring(TEXT("Failed to open database, attempting catastrophic recovery\r\nFor dictionary in: ")) + strtotstr(d->hm)).c_str(), TEXT("Error"), MB_OK);
						if (d->opencrecovery("bin", "bin2")) {
							sharedData.dicts.push_front(std::tuple<tstring, dictionary*>(dir, d));
							if (settings.dict.compare(dir) == 0) {
								setDictionary(d);
							}
						}
						else {
							MessageBox(NULL, TEXT("All attempts failed, dictionary database files are unrecoverable"), TEXT("Error"), MB_OK);
						}
					}

				}
				else {
					dictionary* d = new dictionary(ttostr((root + dir)).c_str());
					loadDictSettings(d, (root + dir + TEXT("\\settings.txt")));
					d->items = 0;
					d->lchars = 0;
					d->longest = 0;

					//d->open(compiled.c_str(), (root + dir + std::string("\\bin2")).c_str(), true);
					if (d->open("bin", "bin2")) {
		
						if (GetFileAttributes((root + dir + TEXT("\\user")).c_str()) != INVALID_FILE_ATTRIBUTES)
						{
							LoadJson(d, (root + dir + TEXT("\\user")), NULL, true);
						}

						WIN32_FIND_DATA innerFindFileData;
						HANDLE hinnerFind = FindFirstFileEx((root + dir + TEXT("\\*.json")).c_str(), FindExInfoStandard, &innerFindFileData, FindExSearchNameMatch, NULL, 0);
						while (hinnerFind != INVALID_HANDLE_VALUE)
						{
							if ((innerFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
								LoadJson(d, root + dir + TEXT("\\") + innerFindFileData.cFileName, NULL);
							}
							if (!FindNextFile(hinnerFind, &innerFindFileData))
							{
								FindClose(hinnerFind);
								hinnerFind = INVALID_HANDLE_VALUE;
							}
						}

						hinnerFind = FindFirstFileEx((root + dir + TEXT("\\*.rtf")).c_str(), FindExInfoStandard, &innerFindFileData, FindExSearchNameMatch, NULL, 0);
						while (hinnerFind != INVALID_HANDLE_VALUE)
						{
							if ((innerFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
								LoadRTF(d, root + dir + TEXT("\\") + innerFindFileData.cFileName, NULL);
							}
							if (!FindNextFile(hinnerFind, &innerFindFileData))
							{
								FindClose(hinnerFind);
								hinnerFind = INVALID_HANDLE_VALUE;
							}
						}

						sharedData.dicts.push_front(std::tuple<tstring, dictionary*>(dir, d));

						if (settings.dict.compare(dir) == 0) {
							setDictionary(d);
						}
					}
					else {
						MessageBox(NULL, TEXT("Failed to open database, delete ALL database files"), TEXT("Error"), MB_OK);
					}
				}
			}
			if (!FindNextFile(hFind, &FindFileData))
			{
				FindClose(hFind);
				hFind = INVALID_HANDLE_VALUE;
			}
		}

}

void strtosetting(const std::string& setting, const std::string& value) {
	const static std::regex map("\\s*(\\S+)->(\\S+)\\s*");
	
	if (setting.compare("HEIGHT") == 0) {
		settings.height = std::atoi(value.c_str());
	}
	else if (setting.compare("TRANS") == 0) {
		if (0 != std::atoi(value.c_str())) {
			settings.trans = TRUE;
		}
	}
	else if (setting.compare("TOP") == 0) {
		if (0 != std::atoi(value.c_str())) {
			settings.top = TRUE;
		}
	}
	else if (setting.compare("MTRANS") == 0) {
		if (0 != std::atoi(value.c_str())) {
			settings.mistrans = TRUE;
		}
	}
	else if (setting.compare("PREFIX") == 0) {
		if (0 != std::atoi(value.c_str())) {
			settings.prefix = TRUE;
		}
	}
	else if (setting.compare("MODE") == 0) {
		settings.mode = std::atoi(value.c_str());
	}
	else if (setting.compare("SPACE") == 0) {
		settings.space = std::atoi(value.c_str());
	}
	else if (setting.compare("DICT") == 0) {
		settings.dict = strtotstr(value);
	}
	else if (setting.compare("FNAME") == 0) {
		settings.fname = strtotstr(value);
	}
	else if (setting.compare("X") == 0) {
		settings.xpos = std::atoi(value.c_str());
	}
	else if (setting.compare("Y") == 0) {
		settings.ypos = std::atoi(value.c_str());
	}
	else if (setting.compare("FWEIGHT") == 0) {
		settings.fweight = std::atoi(value.c_str());
	}
	else if (setting.compare("FSIZE") == 0) {
		settings.fsize = std::atoi(value.c_str());
	}
	else if (setting.compare("MAP") == 0) {
		std::cmatch m;
		if (std::regex_match(value.c_str(), m, map)) {
			__int8 ref = std::atoi(m[2].str().c_str());
			tstring temp = strtotstr(m[1].str());
			if (temp.compare(TEXT("f1")) == 0) {
				settings.map[LOBYTE(VK_F1)] = ref;
			}
			else if(temp.compare(TEXT("f2")) == 0) {
				settings.map[LOBYTE(VK_F2)] = ref;
			}
			else if (temp.compare(TEXT("f3")) == 0) {
				settings.map[LOBYTE(VK_F3)] = ref;
			}
			else if (temp.compare(TEXT("f4")) == 0) {
				settings.map[LOBYTE(VK_F4)] = ref;
			}
			else if (temp.compare(TEXT("f5")) == 0) {
				settings.map[LOBYTE(VK_F5)] = ref;
			}
			else if (temp.compare(TEXT("f6")) == 0) {
				settings.map[LOBYTE(VK_F6)] = ref;
			}
			else if (temp.compare(TEXT("f7")) == 0) {
				settings.map[LOBYTE(VK_F7)] = ref;
			}
			else if (temp.compare(TEXT("f8")) == 0) {
				settings.map[LOBYTE(VK_F8)] = ref;
			}
			else if (temp.compare(TEXT("f9")) == 0) {
				settings.map[LOBYTE(VK_F9)] = ref;
			}
			else if (temp.compare(TEXT("f10")) == 0) {
				settings.map[LOBYTE(VK_F10)] = ref;
			}
			else if (temp.compare(TEXT("f11")) == 0) {
				settings.map[LOBYTE(VK_F11)] = ref;
			}
			else if (temp.compare(TEXT("f12")) == 0) {
				settings.map[LOBYTE(VK_F12)] = ref;
			}
			else {
				HKL locale = GetKeyboardLayout(GetCurrentThreadId());
				for (auto i = temp.cbegin(); i != temp.cend(); i++) {
					TCHAR t = (TCHAR)(*i);
					if (t == TEXT('_')) {
						t = TEXT(' ');
					}
					settings.map[LOBYTE(VkKeyScanEx(t, locale))] = ref;
				}
			}
		}
	}
}

void loadSettings() {
	DWORD len = MAX_PATH;
	tstring root = GetDictDir();

	
	tstring file = root + TEXT("settings");

	settings.trans = FALSE;
	settings.top = FALSE;
	settings.space = 0;
	settings.prefix = FALSE;
	settings.mistrans = FALSE;
	settings.height = 300;
	settings.mode = 0;
	settings.xpos = 200;
	settings.ypos = 200;
	settings.fname = TEXT("Times New Roman");
	settings.fsize = 12;
	settings.fweight = 400;

	HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex property("^\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");
	
	std::cmatch m;

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

		ReadFile(hfile, &c, 1, &bytes, NULL);
		while (bytes > 0) {
			cline += c;
			std::string::size_type r = cline.find("\r\n");
			if (r != std::string::npos) {
				cline.erase(r, 2);
				if (std::regex_match(cline.c_str(), m, property)) {
					strtosetting(m[1].str(), m[2].str());
				}
				cline.clear();
			}
			ReadFile(hfile, &c, 1, &bytes, NULL);
		}

		if (std::regex_match(cline.c_str(), m, property)) {
			strtosetting(m[1].str(), m[2].str());
		}
		CloseHandle(hfile);
	}
}