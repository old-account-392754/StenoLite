#include "stdafx.h"

#ifndef MY_PSTROKE_H
#define MY_PSTROKE_H

#include <string>
#include "texthelpers.h"

DWORD WINAPI processStrokes(LPVOID lpParam);
void processSingleStroke(unsigned __int8* stroke, ULONGLONG &thetime);
tstring sendText(tstring fulltext, unsigned __int8 &flags, unsigned __int8 prevflags, unsigned __int8 nextflags, BOOL &trimspace, bool redirect);
void addStroke(__int32 s);
inline bool compare3(const unsigned __int8* a, const unsigned __int8* b);
void sendstroke(unsigned __int8* keys);

#endif