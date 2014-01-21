#include "stdafx.h"

#ifndef MY_TEXTHELP_H
#define MY_TEXTHELP_H

#include <string>


#ifndef UNICODE  
typedef std::string tstring; 
#else
typedef std::wstring tstring;
#endif

unsigned int cntspaces(const std::string &str);
std::string trimstr(std::string const& str, char const* sepSet);
std::string TCHARtostr(const TCHAR* text, const int &len);
std::string getSubSeq(std::string::const_iterator &i, std::string::const_iterator &end);
tstring getWinStr(HWND hwnd);
std::string ttostr(const tstring &in);
tstring strtotstr(const std::string &in);

#endif