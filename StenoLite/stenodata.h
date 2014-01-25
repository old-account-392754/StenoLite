#ifndef MY_STENODATA_H
#define MY_STENODATA_H

#include "stdafx.h"
#include <db.h>
#include <string>
#include <Windows.h>
#include <list>
#include "texthelpers.h"


int getsecondary(DB *secondary, const DBT *pkey, const DBT *pdata, DBT *skey);

struct dictionary {
	unsigned int longest;
	unsigned int lchars;
	unsigned int items;
	unsigned __int8 sdelete[4];
	unsigned __int8 stab[4];
	unsigned __int8 number[4];
	unsigned __int8 sreturn[4];
	tstring settingslocation;
	std::string hm;
	BOOL extras = FALSE;
	tstring format;


	std::list<std::pair<unsigned __int32, int>> suffix;

	DB* contents = NULL;
	DB* secondary = NULL;
	DB_ENV* env = NULL;

	dictionary(const char* home);
	bool open(const char* file, const char* file2);
	void addNewDItem(unsigned __int8* s, const unsigned int &len, const std::string &str, DB_TXN* trans);
	void addDItem(unsigned __int8 *s, const unsigned int &len, const std::string &str, DB_TXN* trans);
	bool findDItem(unsigned __int8* s, const unsigned int &len, std::string &str, DB_TXN* trans);
	void deleteDItem(unsigned __int8 *s, const unsigned int &len, DB_TXN* trans);
	bool opencrecovery(const char* file, const char* file2);
	void close();
};


void loadDictionaries();
void stroketosteno(const unsigned __int8* keys, TCHAR* buffer, const tstring &format);

void stroketocsteno(const unsigned __int8* keys, std::wstring &buffer, const tstring &format, bool number = false);

void textToStroke(const tstring &stro, unsigned __int8* dest, const tstring &format);
void textToStroke(unsigned __int8* dest, tstring::const_iterator &i, tstring::const_iterator &end, const tstring &format);
int countStrokes(const TCHAR* text, const int &len);
int countStrokes(const tstring &text, const int &len);

unsigned __int8* texttomultistroke(const tstring &in, int& size, const tstring &format);
tstring stroketomultitext(const unsigned __int8* stroke, const unsigned int &numstrokes, const tstring& format);

#endif