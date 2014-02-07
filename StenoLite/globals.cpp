#include "stdafx.h"
#include "globals.h"
#include <time.h>

 cstruct controls;
 sstruct settings;
 istruct inputstate;
 nstruct newwordwin;
 sdata sharedData;


 singlestroke::singlestroke(time_t time) {
	 timestamp = time;
 }

 singlestroke::singlestroke(__int32 s, time_t time) {
	 value.sval = s;
	 timestamp = time;
 }


 singlestroke::singlestroke(unsigned __int8* i, time_t time) {
	 value.ival[0] = i[0];
	 value.ival[1] = i[1];
	 value.ival[2] = i[2];
	 value.ival[3] = i[3];
	 timestamp = time;
 }

 HWND modelesswnd;