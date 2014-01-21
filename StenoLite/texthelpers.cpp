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



std::string TCHARtostr(const TCHAR* text, const int &len) {
	std::string total("");
	for (int i = 0; i < len; i++) {
		if (text[i] == 0)
			break;
		total += (char)(text[i]);
	}
	return total;
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
	return std::string(in.begin(), in.end());
}

tstring strtotstr(const std::string &in) {
	return tstring(in.begin(), in.end());
}

std::string getSubSeq(std::string::const_iterator &i, std::string::const_iterator &end) {
	std::string acc("");
	std::string::const_iterator t = i + 1;
	bool tesc = false;
	bool first = true;
	int sub = 0;
	while (t != end) {
		if (first && *t == '[') {
			
		}
		else if (tesc) {
			tesc = false;
			if (*t != '[' && *t != ']' && *t != '\\')
				acc += '\\';
			acc += *t;
		}
		else if (*t == '[') {
			sub++;
			acc += *t;
		}
		else if (*t == '\\') {
			tesc = true;
		}
		else if (*t == ']') {
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