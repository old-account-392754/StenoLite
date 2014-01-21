#include "stdafx.h"

#ifndef MY_PSTROKE_H
#define MY_PSTROKE_H

#include <string>

DWORD WINAPI processStrokes(LPVOID lpParam);
void processSingleStroke(unsigned __int8* stroke);
std::string sendText(std::string fulltext, unsigned __int8 &flags, unsigned __int8 prevflags, BOOL &trimspace, bool redirect);
void addStroke(__int32 s);

#endif