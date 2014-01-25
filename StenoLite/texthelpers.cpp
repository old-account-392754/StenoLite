#include "stdafx.h"
#include "texthelpers.h"

unsigned int cntspaces(const std::string &str) {
	int cnt = 0;
	for (std::string::const_iterator i = str.begin(); i != str.cend(); i++) {
		if (*i == ' ')
			cnt++;
	}
	return cnt;
}

std::string trimstr(std::string const& str, char const* sepSet)
{
	std::string::size_type const first = str.find_first_not_of(sepSet);
	return (first == std::string::npos)
		? std::string()
		: str.substr(first, str.find_last_not_of(sepSet) - first + 1);
}



tstring getWinStr(HWND hwnd) {
	int tlen = GetWindowTextLength(hwnd) + 1;
	TCHAR* text = new TCHAR[tlen];
	GetWindowText(hwnd, text, tlen);
	tstring tmp = text;
	delete text;
	return tmp;
}

std::string ttostr(const tstring &in) {
#ifndef UNICODE 
	return in;
#else
	//return std::string(in.begin(), in.end());
	int l = WideCharToMultiByte(CP_UTF8, 0, in.c_str(), -1, NULL, 0, NULL, NULL);
	char* t = new char[l];
	WideCharToMultiByte(CP_UTF8, 0, in.c_str(), -1, t, l, NULL, NULL);
	std::string total = t;
	delete t;
	return total;
#endif
}

tstring strtotstr(const std::string &in) {
#ifndef UNICODE 
	return in;
#else
	int len = MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, NULL, 0);
	wchar_t* t = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, t, len);
	std::wstring total = t;
	delete t;
	return total;
#endif
}

tstring getSubSeq(tstring::const_iterator &i, tstring::const_iterator &end) {
	tstring acc(TEXT(""));
	tstring::const_iterator t = i + 1;
	bool tesc = false;
	bool first = true;
	int sub = 0;
	while (t != end) {
		if (first && *t == TEXT('[')) {
			
		}
		else if (tesc) {
			tesc = false;
			if (*t != TEXT('[') && *t != TEXT(']') && *t != TEXT('\\'))
				acc += TEXT('\\');
			acc += *t;
		}
		else if (*t == TEXT('[')) {
			sub++;
			acc += *t;
		}
		else if (*t == TEXT('\\')) {
			tesc = true;
		}
		else if (*t == TEXT(']')) {
			if (sub == 0) {
				i = t;
				return acc;
			}
			else {
				sub--;
				acc += *t;
			}
		}
		else {
			acc += *t;
		}
		first = false;
		t++;
	}
	if (t == end) {
		i = end - 1;
	}
	return acc;
}