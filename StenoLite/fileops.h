#include "stdafx.h"

#ifndef MY_FILEOPS_H
#define MY_FILEOPS_H

#include "stenodata.h"
#include <list>
#include <string>

void saveSettings();
void saveDictSettings(dictionary* d);
void loadSettings();
void loadDictionaries();
void loadDictSettings(dictionary* d, const std::string& file);
std::list<std::string> EnumDicts();
void appendUser(dictionary* d, const std::string& stroke, const std::string& text);
void LoadJson(dictionary* d, const std::string &file, HWND progress = NULL, bool overwrite = false);
void LoadRTF(dictionary* d, const std::string &file, HWND progress = NULL, bool overwrite = false);
void SaveJson(dictionary* d, const std::string &file, HWND progress);
void SaveRTF(dictionary* d, const std::string &file, HWND progress);


#endif