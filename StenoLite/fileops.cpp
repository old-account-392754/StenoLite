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

TCHAR pathbuffer[MAX_PATH];

void writestr(HANDLE hfile, const std::string& data) {
	DWORD bytes;
	WriteFile(hfile, data.c_str(), data.length(), &bytes, NULL);
}


std::string GetDictDir() {
	DWORD len = MAX_PATH;
	const static std::regex rx("\\\\[^\\\\]*$");

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\STENOLITE\\DICTDIR"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)pathbuffer, &len) == ERROR_SUCCESS) {
			return TCHARtostr(pathbuffer, MAX_PATH);
		}
		RegCloseKey(hkey);
	}

	GetModuleFileName(NULL, pathbuffer, MAX_PATH);
	return std::regex_replace(TCHARtostr(pathbuffer, MAX_PATH), rx, "\\");
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
	std::string res = GetDictDir();

	
	std::string file = res + "settings";

	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
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
		writestr(hfile, settings.dict);
		writestr(hfile, "\r\n");

		writestr(hfile, "X = ");
		writestr(hfile, std::to_string(settings.xpos));
		writestr(hfile, "\r\n");

		writestr(hfile, "Y = ");
		writestr(hfile, std::to_string(settings.ypos));
		writestr(hfile, "\r\n");

		HKL locale = GetKeyboardLayout(GetCurrentThreadId());
		DWORD bytes;
		for (int i = 0; i < 256; i++){
			if (settings.map[i] != 0) {
				char c = MapVirtualKeyEx(i, MAPVK_VK_TO_CHAR, locale);
				if (c == ' ') {
					c = '_';
				}
				writestr(hfile, "MAP = ");
				WriteFile(hfile, &c, sizeof(char), &bytes, NULL);
				writestr(hfile, "->");
				writestr(hfile, std::to_string(settings.map[i]));
				writestr(hfile, "\r\n");
			}
		}

		CloseHandle(hfile);
	}
}

void saveDictSettings(dictionary* d) {
	static std::string sbuffer;
	HANDLE hfile = CreateFileA(d->settingslocation.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
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
		stroketocsteno(d->sdelete, sbuffer);
		writestr(hfile, sbuffer);
		writestr(hfile, "\r\n");

		writestr(hfile, "TAB = ");
		sbuffer.clear();
		stroketocsteno(d->stab, sbuffer);
		writestr(hfile, sbuffer);
		writestr(hfile, "\r\n");

		writestr(hfile, "RETURN = ");
		sbuffer.clear();
		stroketocsteno(d->sreturn, sbuffer);
		writestr(hfile, sbuffer);
		writestr(hfile, "\r\n");

		writestr(hfile, "NUMBER = ");
		sbuffer.clear();
		stroketocsteno(d->number, sbuffer);
		writestr(hfile, sbuffer);
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
			stroketocsteno(tstroke.sval, sbuffer);
			writestr(hfile, sbuffer);
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
		textToStroke(value, d->sdelete);
	}
	else if (setting.compare("TAB") == 0) {
		textToStroke(value, d->stab);
	}
	else if (setting.compare("RETURN") == 0) {
		textToStroke(value, d->sreturn);
	}
	else if (setting.compare("NUMBER") == 0) {
		textToStroke(value, d->number);
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

			textToStroke(m[1].str(), tstroke.sval);
			int sufx = strtosuf(m[2].str());
			d->suffix.push_back(std::make_pair(tstroke.ival, sufx));
		}
	}
}

void loadDictSettings(dictionary* d, const std::string& file) {
	textToStroke(std::string("*"), d->sdelete);
	textToStroke(std::string("*"), d->number);
	textToStroke(std::string("T-B"), d->stab);
	textToStroke(std::string("R-RPB"), d->sreturn);
	d->longest = 0;
	d->lchars = 0;
	d->items = 0;
	d->settingslocation = file;
	d->extras = FALSE;

	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex property("^\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");

	std::cmatch m;

	if (hfile != INVALID_HANDLE_VALUE) {
		char c;
		DWORD bytes;
		std::string cline("");

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

void addentry(dictionary* d, const std::string &stroke, const std::string &text, DB_TXN* trans, bool overwrite = false) {
	int numstrokes = countStrokes(stroke, stroke.length());
	unsigned __int8* sdata = new unsigned __int8[numstrokes * 3];

	std::string::const_iterator it = stroke.cbegin();
	int i = 0;
	while (it != stroke.cend()) {
		textToStroke(sdata + i * 3, it, stroke.cend());
		i++;
	}

	if (overwrite)
		d->addDItem(sdata, numstrokes * 3, text, trans);
	else
		d->addNewDItem(sdata, numstrokes * 3, text, trans);

	delete sdata;
}

void appendUser(dictionary* d, const std::string& stroke, const std::string& text) {
	const static std::regex rx("\\\\[^\\\\]*$");
	std::string file = std::regex_replace(d->settingslocation, rx, "\\user");
	
	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		SetFilePointer(hfile, 0, NULL, FILE_END);
		writestr(hfile, "\r\n\"");
		writestr(hfile, stroke);
		writestr(hfile, "\": \"");
		writestr(hfile, text);
		writestr(hfile, "\"");

		CloseHandle(hfile);
	}
}
//{\rtf1\ansi{\*\cxrev100}\cxdict{\*\cxsystem SL}{\stylesheet{\s0 Normal;}}

void SaveRTF(dictionary* d, const std::string &file, HWND progress) {
	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, 0, NULL);
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
			std::string acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc);
			for (int i = 1; i * 3 < keyin.size; i++) {
				acc += "/";
				std::string tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i * 3]), tmp);
				acc += tmp;
			}
			writestr(hfile, acc);
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


void SaveJson(dictionary* d, const std::string &file, HWND progress) {
	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, 0, NULL);
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

		writestr(hfile, "{\r\n");
		d->contents->cursor(d->contents, trans, &startcursor, 0);
		int result = startcursor->get(startcursor, &keyin, &strin, DB_FIRST);

		while (result == 0) {
			writestr(hfile, "\"");
			std::string acc;
			stroketocsteno((unsigned __int8*)(keyin.data), acc);
			for (int i = 1; i * 3 < keyin.size; i++) {
				acc += "/";
				std::string tmp;
				stroketocsteno(&(((unsigned __int8*)(keyin.data))[i*3]), tmp);
				acc += tmp;
			}
			writestr(hfile, acc);
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

void LoadJson(dictionary* d, const std::string &file, HWND progress, bool overwrite) {
	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
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
					addentry(d, m[1].str(), m[2].str(), trans, overwrite );
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
			addentry(d, m[1].str(), m[2].str(), trans, overwrite);
		}

		trans->commit(trans, 0);

		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
 
}

void LoadRTF(dictionary* d, const std::string &file, HWND progress, bool overwrite) {
	//{\*\cxs STROKETEXT}ENTRYTEXT
	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
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
					addentry(d, m[1].str(), m[2].str(), trans, overwrite);
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
			addentry(d, m[1].str(), m[2].str(), trans, overwrite);
		}

		trans->commit(trans, 0);
		PostMessage(controls.main, WM_LOAD_PROGRESS, 3, 0);
		CloseHandle(hfile);
	}
}

std::list<std::string> EnumDicts() {

	DWORD len = MAX_PATH;
	std::string res = GetDictDir();

	std::list<std::string> results;

	std::string filename = res + "*";

	WIN32_FIND_DATAA FindFileData;

	int found = 0;

	HANDLE hFind = FindFirstFileExA(filename.c_str(), FindExInfoBasic, &FindFileData, FindExSearchLimitToDirectories, NULL, 0);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && std::string(".").compare(FindFileData.cFileName) != 0 && std::string("..").compare(FindFileData.cFileName) != 0) {
			results.push_back(std::string(FindFileData.cFileName));
			found++;
		}
		if (!FindNextFileA(hFind, &FindFileData))
		{
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	if (found == 0) {
		MessageBox(NULL, (tstring(TEXT("No dictionary directories found in: ")) + strtotstr(res)).c_str(), TEXT("Error"), MB_OK);
	}

	return results;
}


DWORD WINAPI OpenDB(_In_  LPVOID lpParameter) {
	dictionary* d = (dictionary*)lpParameter;
	if (d->open("bin", "bin2", false)) {
		return 1;
	}
	return 0;
}

DWORD WINAPI OpenRDB(_In_  LPVOID lpParameter) {
	dictionary* d = (dictionary*)lpParameter;
	if (d->openrecovery("bin", "bin2")) {
		return 1;
	}
	return 0;
}

DWORD WINAPI OpenCRDB(_In_  LPVOID lpParameter) {
	dictionary* d = (dictionary*)lpParameter;
	if (d->opencrecovery("bin", "bin2")) {
		return 1;
	}
	return 0;
}

bool OpenDictionary(dictionary*d, const std::string& dir) {
	HANDLE h = CreateThread(NULL, 0, &OpenDB, (LPVOID)d, 0, NULL);
	DWORD exit = STILL_ACTIVE;
	for (int i = 0; exit == STILL_ACTIVE && i < 200; i++) {
		GetExitCodeThread(h, &exit);
		Sleep(10);
	}

	if (exit == 0 || exit == STILL_ACTIVE) {
		TerminateThread(h, 0);
		MessageBox(NULL, (tstring(TEXT("Failed to open database, attempting recovery\r\nFor dictionary in: ")) + strtotstr(d->hm)).c_str(), TEXT("Error"), MB_OK);

		h = CreateThread(NULL, 0, &OpenRDB, (LPVOID)d, 0, NULL);

		Sleep(10000);
		GetExitCodeThread(h, &exit);
		if (exit == 0 || exit == STILL_ACTIVE) {
			TerminateThread(h, 0);
			MessageBox(NULL, TEXT("Normal recovery failed, attempting catastrophic recovery"), TEXT("Error"), MB_OK);

			h = CreateThread(NULL, 0, &OpenCRDB, (LPVOID)d, 0, NULL);

			Sleep(10000);
			GetExitCodeThread(h, &exit);
			if (exit == 0 || exit == STILL_ACTIVE) {
				MessageBox(NULL, TEXT("All attempts failed, dictionary database files are unrecoverable"), TEXT("Error"), MB_OK);
				return false;
			}
		}
	}
	return true;
}

void loadDictionaries() {
	DWORD len = MAX_PATH;
	std::string root = GetDictDir();


		std::string filename = root + "*";

		WIN32_FIND_DATAA FindFileData;

		HANDLE hFind = FindFirstFileExA(filename.c_str(), FindExInfoBasic, &FindFileData, FindExSearchLimitToDirectories, NULL, 0);
		while (hFind != INVALID_HANDLE_VALUE)
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && std::string(".").compare(FindFileData.cFileName) != 0 && std::string("..").compare(FindFileData.cFileName) != 0) {
				std::string dir(FindFileData.cFileName);
				std::string compiled = root + dir + "\\bin";
				
				if (GetFileAttributesA(compiled.c_str()) != INVALID_FILE_ATTRIBUTES) {

					dictionary* d = new dictionary((root + dir).c_str());

					loadDictSettings(d, (root + dir + std::string("\\settings.txt")));


					//d->open(compiled.c_str(), (root + dir + std::string("\\bin2")).c_str(), false);
					if (OpenDictionary(d, dir)) {
						sharedData.dicts.push_front(std::tuple<std::string, dictionary*>(dir, d));

						if (settings.dict.compare(dir) == 0) {
							sharedData.currentd = d;
						}
					}

					/*if (d->open("bin", "bin2", false)) {
						sharedData.dicts.push_front(std::tuple<std::string, dictionary*>(dir, d));

						if (settings.dict.compare(dir) == 0) {
							sharedData.currentd = d;
						}
					}*/
				}
				else {
					dictionary* d = new dictionary((root + dir).c_str());
					loadDictSettings(d, (root + dir + "\\settings.txt"));
					d->items = 0;
					d->lchars = 0;
					d->longest = 0;

					//d->open(compiled.c_str(), (root + dir + std::string("\\bin2")).c_str(), true);
					if (d->open("bin", "bin2", true)) {

						if (GetFileAttributesA((root + dir + "\\user").c_str()) != INVALID_FILE_ATTRIBUTES)
						{
							LoadJson(d, (root + dir + "\\user"), NULL, true);
						}

						WIN32_FIND_DATAA innerFindFileData;
						HANDLE hinnerFind = FindFirstFileExA((root + dir + "\\*.json").c_str(), FindExInfoBasic, &innerFindFileData, FindExSearchNameMatch, NULL, 0);
						while (hinnerFind != INVALID_HANDLE_VALUE)
						{
							if ((innerFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
								LoadJson(d, root + dir + "\\" + innerFindFileData.cFileName, NULL);
							}
							if (!FindNextFileA(hinnerFind, &innerFindFileData))
							{
								FindClose(hinnerFind);
								hinnerFind = INVALID_HANDLE_VALUE;
							}
						}

						hinnerFind = FindFirstFileExA((root + dir + "\\*.rtf").c_str(), FindExInfoBasic, &innerFindFileData, FindExSearchNameMatch, NULL, 0);
						while (hinnerFind != INVALID_HANDLE_VALUE)
						{
							if ((innerFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
								LoadRTF(d, root + dir + "\\" + innerFindFileData.cFileName, NULL);
							}
							if (!FindNextFileA(hinnerFind, &innerFindFileData))
							{
								FindClose(hinnerFind);
								hinnerFind = INVALID_HANDLE_VALUE;
							}
						}

						sharedData.dicts.push_front(std::tuple<std::string, dictionary*>(dir, d));

						if (settings.dict.compare(dir) == 0) {
							sharedData.currentd = d;
						}
					}
					else {
						MessageBox(NULL, TEXT("Failed to open database, delete ALL database files"), TEXT("Error"), MB_OK);
					}
				}
			}
			if (!FindNextFileA(hFind, &FindFileData))
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
		settings.dict = value;
	}
	else if (setting.compare("X") == 0) {
		settings.xpos = std::atoi(value.c_str());
	}
	else if (setting.compare("Y") == 0) {
		settings.ypos = std::atoi(value.c_str());
	}
	else if (setting.compare("MAP") == 0) {
		std::cmatch m;
		if (std::regex_match(value.c_str(), m, map)) {
			__int8 ref = std::atoi(m[2].str().c_str());
			std::string temp = m[1].str();
			HKL locale = GetKeyboardLayout(GetCurrentThreadId());
			for (std::string::const_iterator i = temp.cbegin(); i != temp.cend(); i++) {
				char t = (char)(*i);
				if (t == '_') {
					t = ' ';
				}
				settings.map[LOBYTE(VkKeyScanEx(t, locale))] = ref;
			}
		}
	}
}

void loadSettings() {
	DWORD len = MAX_PATH;
	std::string root = GetDictDir();

	
	std::string file = root + "settings";

	settings.trans = FALSE;
	settings.top = FALSE;
	settings.space = 0;
	settings.prefix = FALSE;
	settings.mistrans = FALSE;
	settings.height = 300;
	settings.mode = 0;
	settings.xpos = 200;
	settings.ypos = 200;


	HANDLE hfile = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	std::regex property("^\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");
	
	std::cmatch m;

	if (hfile != INVALID_HANDLE_VALUE) {
		char c;
		DWORD bytes;
		std::string cline("");

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