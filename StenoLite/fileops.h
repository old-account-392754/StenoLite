#include "stdafx.h"

#ifndef MY_FILEOPS_H
#define MY_FILEOPS_H

#include "stenodata.h"
#include <list>
#include <string>
#include "texthelpers.h"

void saveSettings();
void saveDictSettings(dictionary* d);
void loadSettings();
void loadDictionaries();
void loadDictSettings(dictionary* d, const tstring& file);
std::list<tstring> EnumDicts();
void appendUser(dictionary* d, const tstring& stroke, const std::string& text);
void LoadJson(dictionary* d, const tstring &file, HWND progress = NULL, bool overwrite = false);
void LoadRTF(dictionary* d, const tstring &file, HWND progress = NULL, bool overwrite = false);
void SaveJson(dictionary* d, const tstring &file, HWND progress);
void SaveRTF(dictionary* d, const tstring &file, HWND progress);
void writestr(HANDLE hfile, const std::string& data);
void writeBOM(HANDLE hfile);
bool isReturn(char value);
void LoadPloverJson(dictionary* d, const tstring &file, HWND progress);

#endif