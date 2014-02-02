#include "stdafx.h"
#ifndef MY_BASICSERIAL_H
#define MY_BASICSERIAL_H

#include "texthelpers.h"

void InitEvents();
void EndThreads();
DWORD WINAPI TXBolt(LPVOID lpParam);
DWORD WINAPI Passport(LPVOID lpParam);
DWORD WINAPI Gemini(LPVOID lpParam);
HANDLE openCom(tstring port, int baud);

#endif