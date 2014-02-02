#include "stdafx.h"
#include "basicserial.h"
#include "texthelpers.h"
#include <list>
#include "globals.h"
#include "pstroke.h"

HANDLE readevent;
HANDLE shutoff;
HANDLE handoff;

HANDLE com;

bool running;

void InitEvents() {
	running = false;
	readevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	shutoff = CreateEvent(NULL, FALSE, FALSE, NULL);
	handoff = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void EndThreads() {
	if (running) {
		running = false;
		SetEvent(shutoff);
		WaitForSingleObject(handoff, INFINITE);
		CloseHandle(com);
	}
}



DWORD WINAPI TXBolt(LPVOID lpParam)
{
	HANDLE harray[2] = { readevent, shutoff };

	OVERLAPPED overlap;
	memset(&overlap, 0, sizeof(overlap));
	overlap.hEvent = readevent;

	DWORD read;
	BYTE buffer;

	BYTE prevset = 0;
	ReadFile(com, &buffer, 1, NULL, &overlap);
	WaitForMultipleObjects(2, harray, FALSE, INFINITE);

	while (running) {
		read = 0;
		GetOverlappedResult(com, &overlap, &read, FALSE);
		if (read != 0) {
			//process buffer
			if (buffer == 0) {
				if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
					sendstroke(inputstate.stroke);
				}
			}
			else {
				if ((buffer & 0xC0) <= prevset) {
					if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
						sendstroke(inputstate.stroke);
					}
				}

				prevset = (buffer & 0xC0);

				BYTE index = prevset >> 6;
				BYTE remainder = buffer & 0x3F;

				unsigned int offset = ((unsigned int)remainder) << ((unsigned int)index * 6);
				unsigned __int32* cast = (unsigned __int32*)(&inputstate.stroke[0]);
				*cast |= offset;
			}
		}

		memset(&overlap, 0, sizeof(overlap));
		overlap.hEvent = readevent;
		ReadFile(com, &buffer, 1, NULL, &overlap);
		WaitForMultipleObjects(2, harray, FALSE, INFINITE);
	}

	SetEvent(handoff);
	return 0;
}


HANDLE openCom(tstring port, int baud) {

	com = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (com != INVALID_HANDLE_VALUE) {
		DCB dcbstruct;
		memset(&dcbstruct, 0, sizeof(DCB));
		dcbstruct.DCBlength = sizeof(DCB);

		dcbstruct.fBinary = TRUE;
		dcbstruct.BaudRate = baud;
		dcbstruct.Parity = NOPARITY;
		dcbstruct.ByteSize = 8;
		dcbstruct.StopBits = ONESTOPBIT;
	
		if (SetCommState(com, &dcbstruct)) {
			COMMTIMEOUTS timeouts;
			memset(&timeouts, 0, sizeof(timeouts));
			SetCommTimeouts(com, &timeouts);

			running = true;
		}
		else {

			DWORD err = GetLastError();
			TCHAR lpMsgBuf[500] = TEXT("\0");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 500, NULL);
			MessageBox(NULL, (tstring(TEXT("Could not set com state: ")) + lpMsgBuf).c_str(), (TEXT("Error ") + std::to_wstring(err)).c_str(), MB_OK);
			CloseHandle(com);
			com = INVALID_HANDLE_VALUE;
		}
	}
	else {
		DWORD err = GetLastError();
		TCHAR lpMsgBuf[500] = TEXT("\0");
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 500, NULL);
		MessageBox(NULL, (tstring(TEXT("Could not open com port: ")) + lpMsgBuf).c_str(), (TEXT("Error ") + std::to_wstring(err)).c_str(), MB_OK);
	}
	return com;
}