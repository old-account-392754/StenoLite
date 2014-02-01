#include "stdafx.h"
#include "basicserial.h"
#include "texthelpers.h"
#include <list>

HANDLE readevent;
HANDLE datamutex;
HANDLE shutoff;
HANDLE handoff;
HANDLE handoff2;
HANDLE newdata;

HANDLE com;

std::list<BYTE> datalist;

bool running;

void InitEvents() {
	readevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	shutoff = CreateEvent(NULL, TRUE, FALSE, NULL);
	handoff = CreateEvent(NULL, FALSE, FALSE, NULL);
	handoff2 = CreateEvent(NULL, FALSE, FALSE, NULL);
	newdata = CreateEvent(NULL, FALSE, FALSE, NULL);
	datamutex = CreateMutex(NULL, FALSE, NULL);
}

void EndThreads() {
	running = false;
	HANDLE harray[2] = { handoff, handoff2 };
	SetEvent(shutoff);
	WaitForMultipleObjects(2, harray, TRUE, INFINITE);
	ResetEvent(shutoff);
}

DWORD WINAPI readSerial(LPVOID lpParam)
{
	HANDLE harray[2] = { readevent, shutoff };

	OVERLAPPED overlap;
	memset(&overlap, 0, sizeof(overlap));
	overlap.hEvent = readevent;

	BYTE buffer;
	ReadFile(com, &buffer, 1, NULL, &overlap);
	WaitForMultipleObjects(2, harray, FALSE, INFINITE);

	while (running) {
		WaitForSingleObject(datamutex, INFINITE);
		datalist.push_back(buffer);
		ReleaseMutex(datamutex);
		SetEvent(newdata);


		memset(&overlap, 0, sizeof(overlap));
		overlap.hEvent = readevent;
		ReadFile(com, &buffer, 1, NULL, &overlap);
		WaitForMultipleObjects(2, harray, FALSE, INFINITE);
	}

	SetEvent(handoff);
	return 0;
}

HANDLE openCom(tstring port) {
	DCB dcb;
	HANDLE hCom;
	TCHAR *pcCommPort = TEXT("COM1"); //  Most systems have a COM1 port

	//  Open a handle to the specified com port.
	hCom = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL); 


	DCB dcbstruct;
	BuildCommDCB(TEXT("baud=9600 parity=N data=8 stop=1 xon=off rts=off"), &dcbstruct);

	SetCommState(hCom, &dcb);

	COMMTIMEOUTS timeouts;
	memset(&timeouts, 0, sizeof(timeouts));
	SetCommTimeouts(hCom, &timeouts);

	running = true;

	com = hCom;
	return hCom;
}