#include "stdafx.h"
#include "search.h"

#include "globals.h"
#include <Windows.h>
#include "texthelpers.h"

DWORD WINAPI searchDictionary(LPVOID lpParam)
{
	while (controls.inited != TRUE) {
		Sleep(1);
	}

	DBC* cursor;
	DBT keyin;
	keyin.data = NULL;
	keyin.size = 0;
	keyin.ulen = 0;
	keyin.dlen = 0;
	keyin.doff = 0;
	keyin.flags = DB_DBT_USERMEM;

	DBT strin;
	strin.data = NULL;
	strin.size = 0;
	strin.ulen = 0;
	strin.dlen = 0;
	strin.doff = 0;
	strin.flags = DB_DBT_USERMEM;

	DBT pkey;
	pkey.data = NULL;
	pkey.size = 0;
	pkey.ulen = 0;
	pkey.dlen = 0;
	pkey.doff = 0;
	pkey.flags = DB_DBT_USERMEM;


	while (sharedData.running == TRUE) {
		WaitForSingleObject(sharedData.newtext, INFINITE);
		sharedData.addedtext = FALSE;
		if (sharedData.currentd != NULL && controls.currenttab == 2) {
			pkey.data = new unsigned __int8[sharedData.currentd->longest * 3 + 1];
			pkey.size = 0;
			pkey.ulen = sharedData.currentd->longest * 3 + 1;

			strin.data = new unsigned __int8[sharedData.currentd->lchars + 1];
			strin.size = 0;
			strin.ulen = sharedData.currentd->lchars + 1;

			keyin.data = new unsigned __int8[sharedData.currentd->lchars + 1];
			keyin.size = 0;
			keyin.ulen = sharedData.currentd->lchars + 1;


			SetWindowText(controls.mesuggest, TEXT(""));
			InvalidateRect(controls.main, NULL, TRUE);
			UpdateWindow(controls.main);

			std::string current("");
			std::list<singlestroke*>::iterator i = sharedData.strokes.begin();

			if (settings.prefix == FALSE) {
				//whole string search

				while (i != sharedData.strokes.cend() && sharedData.addedtext == FALSE && current.length() <= sharedData.currentd->lchars + 1) {
					current = (*i)->textout->text + current;
					std::string stripped = trimstr(current, " ");

					if (stripped.length() > sharedData.currentd->lchars) {
						break;
					}
					//loop for one string BB

					strcpy_s((char*)(strin.data), (size_t)(strin.ulen), stripped.c_str());
					strin.size = stripped.length() + 1;

					DB_TXN* trans;
					sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, DB_READ_COMMITTED | DB_TXN_NOWAIT);
					trans->set_priority(trans, 200);

					sharedData.currentd->secondary->cursor(sharedData.currentd->secondary, trans, &cursor, 0);

					bool outthisrun = false;
					int result = cursor->pget(cursor, &strin, &pkey, &keyin, DB_SET);

					while (sharedData.addedtext == FALSE && result == 0) {
						int len;
						TCHAR buffer[400];
						if (!outthisrun) {
							std::string msg = stripped + std::string("\r\n");
							std::copy(msg.begin(), msg.end(), buffer);
							buffer[msg.size()] = 0;

							len = SendMessage(controls.mesuggest, WM_GETTEXTLENGTH, 0, 0);
							SendMessage(controls.mesuggest, EM_SETSEL, len, len);
							SendMessage(controls.mesuggest, EM_REPLACESEL, FALSE, (LPARAM)buffer);
							outthisrun = true;
						}

						int cindex = 0;
						for (int j = 0; j < pkey.size; j += 3) {
							stroketocsteno(&(((unsigned __int8*)(pkey.data))[j]), &(buffer[cindex]));
							cindex = _tcsnlen(buffer, 400);
							if (j != pkey.size - 1) {
								buffer[cindex] = TEXT('/');
								buffer[cindex + 1] = 0;
							}
							cindex++;
						}
						cindex--;
						buffer[cindex] = TEXT('\r');
						buffer[cindex + 1] = TEXT('\n');
						buffer[cindex + 2] = 0;

						len = SendMessage(controls.mesuggest, WM_GETTEXTLENGTH, 0, 0);
						SendMessage(controls.mesuggest, EM_SETSEL, len, len);
						SendMessage(controls.mesuggest, EM_REPLACESEL, FALSE, (LPARAM)buffer);

						result = cursor->pget(cursor, &strin, &pkey, &keyin, DB_NEXT_DUP);
					}

					textoutput* cur = (*i)->textout;
					i++;
					while (i != sharedData.strokes.cend() && (*i)->textout == cur) {
						i++;
					}

					
					cursor->close(cursor);
					trans->commit(trans, 0);
				}
			}
			else {
				// prefix search
				std::string prev("");

				while (i != sharedData.strokes.cend() && sharedData.addedtext == FALSE && current.length() <= sharedData.currentd->lchars + 1) {

					std::string tmp = trimstr((*i)->textout->text + current, " ");
					if (tmp.find(' ') != std::string::npos || tmp.find('\n') != std::string::npos || tmp.find('\t') != std::string::npos) {
						break;
					}

					current = (*i)->textout->text + current;
					textoutput* cur = (*i)->textout;
					i++;
					while (i != sharedData.strokes.cend() && (*i)->textout == cur) {
						i++;
					}
				}
				current = trimstr(current, " ");

				if (current.length() >= 3) {

					strcpy_s((char*)(strin.data), (size_t)(strin.ulen), current.c_str());
					strin.size = current.length() + 1;

					DB_TXN* trans;
					sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, DB_READ_COMMITTED | DB_TXN_NOWAIT);
					trans->set_priority(trans, 200);

					sharedData.currentd->secondary->cursor(sharedData.currentd->secondary, trans, &cursor, 0);
					int result = cursor->pget(cursor, &strin, &pkey, &keyin, DB_SET_RANGE);

					while (sharedData.addedtext == FALSE && result == 0) {

						int len;
						TCHAR buffer[400];

						std::string msg = (char*)(keyin.data);
						if (current.compare(0, current.length(), (char*)(keyin.data), current.length()) != 0) {
							//no longer begins with substring
							break;
						}
						if (prev.compare((char*)(keyin.data)) != 0) {
							//needs new header
							prev = (char*)(keyin.data);

							std::copy(prev.begin(), prev.end(), buffer);
							buffer[prev.size()] = TEXT('\r');
							buffer[prev.size() + 1] = TEXT('\n');
							buffer[prev.size() + 2] = 0;

							len = SendMessage(controls.mesuggest, WM_GETTEXTLENGTH, 0, 0);
							SendMessage(controls.mesuggest, EM_SETSEL, len, len);
							SendMessage(controls.mesuggest, EM_REPLACESEL, FALSE, (LPARAM)buffer);
						}

						int cindex = 0;
						for (int j = 0; j < pkey.size; j += 3) {
							stroketocsteno(&(((unsigned __int8*)(pkey.data))[j]), &(buffer[cindex]));
							cindex = _tcsnlen(buffer, 400);
							if (j != pkey.size - 1) {
								buffer[cindex] = TEXT('/');
								buffer[cindex + 1] = 0;
							}
							cindex++;
						}
						cindex--;
						buffer[cindex] = TEXT('\r');
						buffer[cindex + 1] = TEXT('\n');
						buffer[cindex + 2] = 0;

						len = SendMessage(controls.mesuggest, WM_GETTEXTLENGTH, 0, 0);
						SendMessage(controls.mesuggest, EM_SETSEL, len, len);
						SendMessage(controls.mesuggest, EM_REPLACESEL, FALSE, (LPARAM)buffer);

						result = cursor->pget(cursor, &strin, &pkey, &keyin, DB_NEXT);
					}

					cursor->close(cursor);
					trans->commit(trans, 0);
				}



			}


			if (keyin.data != NULL)
			{
				delete keyin.data;
				keyin.data = NULL;
			}
			if (strin.data != NULL)
			{
				delete strin.data;
				strin.data = NULL;
			}
			if (pkey.data != NULL)
			{
				delete pkey.data;
				pkey.data = NULL;
			}
		}

	}
	sharedData.searching = FALSE;
	return 0;
}