#include "stdafx.h"
#ifndef MY_BASICSERIAL_H
#define MY_BASICSERIAL_H

#include "texthelpers.h"

void InitEvents();
void EndThreads();
DWORD WINAPI TXBolt(LPVOID lpParam);
DWORD WINAPI Passport(LPVOID lpParam);
DWORD WINAPI Gemini(LPVOID lpParam);
DWORD WINAPI Stentura(LPVOID lpParam);
HANDLE openCom(tstring port, int baud, int timeoutms = 0);

#endif