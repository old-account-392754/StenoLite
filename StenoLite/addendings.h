#include "stdafx.h"

#ifndef MY_ENDINGS_H
#define MY_ENDINGS_H

#include <string>
#include "texthelpers.h"

#define ENDING_S	1
#define ENDING_IVE	2
#define ENDING_ION	3
#define ENDING_IONS	4
#define ENDING_TH	5
#define ENDING_LY	6
#define ENDING_ING	7
#define ENDING_INGS	8
#define ENDING_ED	9
#define ENDING_ST	10
#define ENDING_ER	11
#define ENDING_ERS	12
#define ENDING_NESS 13
#define ENDING_ABLE	14
#define ENDING_MENT	15
#define ENDING_FUL	16
#define ENDING_IST	17
#define ENDING_ISTS	18

#define NUM_ENDINGS 18

void addending(tstring &text, int ending);
int strtosuf(const std::string& source);
char* suftostr(const int& val);

#endif