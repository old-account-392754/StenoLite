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

DWORD WINAPI Gemini(LPVOID lpParam)
{
	//I got this idea from plover -- thanks!
	const static int keys[] = { -1, 22, 22, 22, 22, 22, 22, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, -1, -1, -1, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 22, 22, 22, 22, 22, 21 };
	HANDLE harray[2] = { readevent, shutoff };

	OVERLAPPED overlap;
	memset(&overlap, 0, sizeof(overlap));
	overlap.hEvent = readevent;

	DWORD read;
	BYTE buffer[6];

	ReadFile(com, buffer, 6, NULL, &overlap);
	WaitForMultipleObjects(2, harray, FALSE, INFINITE);

	DWORD totalread = 0;
	while (running) {
		read = 0;
		GetOverlappedResult(com, &overlap, &read, FALSE);
		totalread += read;
		if (totalread >= 6) {
			//sanity check
			if ((buffer[0] & 0x80) != 0 && (buffer[1] & 0x80) == 0 && (buffer[2] & 0x80) == 0 && (buffer[3] & 0x80) == 0 && (buffer[4] & 0x80) == 0 && (buffer[5] & 0x80) == 0) {

				unsigned __int32* cast = (unsigned __int32*)(&inputstate.stroke[0]);
				//*cast |= offset;

				for (int i = 0; i < 6; i++) {
					for (int j = 0; j < 7; i++) {
						if ((buffer[i] & (0x40 >> j)) != 0) {
							int into = keys[i * 7 + j];
							if (into != -1) {
								*cast |= 1 << into;
							}
						}
					}
				}

				if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
					sendstroke(inputstate.stroke);
				}
			}
			
			totalread = 0;
		}
		
		memset(&overlap, 0, sizeof(overlap));
		overlap.hEvent = readevent;
		ReadFile(com, (buffer + totalread), 6 - totalread, NULL, &overlap);
		WaitForMultipleObjects(2, harray, FALSE, INFINITE);
	}

	SetEvent(handoff);
	return 0;
}


DWORD WINAPI Passport(LPVOID lpParam)
{
	const static tstring pformat(TEXT("#STKPWHRAO*EUFQNBLGYXDZ"));
	HANDLE harray[2] = { readevent, shutoff };

	OVERLAPPED overlap;
	memset(&overlap, 0, sizeof(overlap));
	overlap.hEvent = readevent;

	DWORD read;
	BYTE buffer;

	std::string strbuffer;

	ReadFile(com, &buffer, 1, NULL, &overlap);
	WaitForMultipleObjects(2, harray, FALSE, INFINITE);

	while (running) {
		read = 0;
		GetOverlappedResult(com, &overlap, &read, FALSE);
		if (read != 0) {
			//process buffer
			strbuffer += (char)buffer;
			if ((char)buffer == '>') {
				std::string thestroke;
				auto i = strbuffer.cbegin();
				while (i != strbuffer.cend() && *i != '/') {
					i++;
				}
				if (i != strbuffer.cend())
					i++;
				char chr;
				while (i != strbuffer.cend() && *i != '/') {
					chr = *i;
					i++;
					if (i != strbuffer.cend()) {
						if (*i == '8' || *i == '9' || *i == 'a' || *i == 'b' || *i == 'c' || *i == 'd' || *i == 'e' || *i == 'f') {
							if (chr == '!' || chr == '+' || chr == '^') {

							}
							else if (chr == 'C') {
								thestroke += 'S';
							}
							else if (chr == '~') {
								thestroke += '*';
							}
							else  {
								thestroke += chr;
							}
						}
						i++;
					}
				}

				textToStroke(strtotstr(thestroke), inputstate.stroke, pformat);

				if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
					sendstroke(inputstate.stroke);
				}

				strbuffer.clear();
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