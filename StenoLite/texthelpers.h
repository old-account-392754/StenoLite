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
tstring getSubSeq(tstring::const_iterator &i, tstring::const_iterator &end);
tstring getWinStr(HWND hwnd);
std::string ttostr(const tstring &in);
tstring strtotstr(const std::string &in);

#endif