#ifndef MY_STENODATA_H
#define MY_STENODATA_H

#include "stdafx.h"
#include <db.h>
#include <string>
#include <Windows.h>
#include <list>


int getsecondary(DB *secondary, const DBT *pkey, const DBT *pdata, DBT *skey);

struct dictionary {
	unsigned int longest;
	unsigned int lchars;
	unsigned int items;
	unsigned __int8 sdelete[4];
	unsigned __int8 stab[4];
	unsigned __int8 number[4];
	unsigned __int8 sreturn[4];
	std::string settingslocation;
	std::string hm;
	BOOL extras = FALSE;


	std::list<std::pair<unsigned __int32, int>> suffix;

	DB* contents = NULL;
	DB* secondary = NULL;
	DB_ENV* env = NULL;

	dictionary(const char* home);
	void open(const char* file, const char* file2, bool newd);
	void addNewDItem(unsigned __int8* s, const int &len, const std::string &str, DB_TXN* trans);
	void addDItem(unsigned __int8 *s, const int &len, const std::string &str, DB_TXN* trans);
	bool findDItem(unsigned __int8* s, const int &len, std::string &str, DB_TXN* trans);
	void deleteDItem(unsigned __int8 *s, const int &len, DB_TXN* trans);
	void close();
};


void loadDictionaries();
void stroketosteno(unsigned __int8* keys, TCHAR* buffer);

void stroketocsteno(unsigned __int8* keys, TCHAR* buffer, bool number = false);
void stroketocsteno(unsigned __int8* keys, std::string &buffer, bool number = false);
void stroketocsteno(unsigned __int8* keys, std::wstring &buffer, bool number = false);

void textToStroke(const std::string &stro, unsigned __int8* dest);
void textToStroke(unsigned __int8* dest, std::string::const_iterator &i, std::string::const_iterator &end);
int countStrokes(const TCHAR* text, const int &len);
int countStrokes(const std::string &text, const int &len);

unsigned __int8* texttomultistroke(const std::string &in, int& size);

#endif