#include "stdafx.h"
#include "globals.h"

 cstruct controls;
 sstruct settings;
 istruct inputstate;
 nstruct newwordwin;
 sdata sharedData;


 singlestroke::singlestroke() {
 }

 singlestroke::singlestroke(__int32 s) {
	 value.sval = s;
 }

 singlestroke::singlestroke(unsigned __int8* i) {
	 value.ival[0] = i[0];
	 value.ival[1] = i[1];
	 value.ival[2] = i[2];
	 value.ival[3] = i[3];
 }

 HWND modelesswnd;