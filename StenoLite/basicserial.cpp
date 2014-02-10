#include "stdafx.h"
#include "basicserial.h"
#include "texthelpers.h"
#include <vector>
#include "globals.h"
#include "pstroke.h"

HANDLE readevent;
HANDLE shutoff;
HANDLE handoff;

HANDLE com;

bool running;

#pragma pack(push, 1) 
struct StenturaRequest
{
	BYTE lead;
	BYTE seq;
	WORD length;
	WORD action;
	WORD p1;
	WORD p2;
	WORD p3;
	WORD p4;
	WORD p5;
	WORD checksum;

};

struct StenturaResponse
{
	BYTE lead;
	BYTE seq;
	WORD length;
	WORD action;
	WORD error;
	WORD p1;
	WORD p2;
	WORD checksum;
};
#pragma pack(pop)

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
		WaitForSingleObject(shutoff, 1); // to ensure that the shutoff event is reset
		CloseHandle(com);
	}
}

#define REAL_FILE "REALTIME.000"

WORD StenturaChecksum(BYTE* data, unsigned int length){
	static const WORD cktable[] = {
		0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
		0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
		0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
		0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
		0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
		0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
		0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
		0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
		0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
		0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
		0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
		0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
		0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
		0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
		0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
		0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
		0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
		0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
		0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
		0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
		0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
		0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
		0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
		0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
		0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
		0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
		0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
		0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
		0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
		0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
		0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
		0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040 };
	WORD checksum = 0;
	for (unsigned int i = 0; i < length; i++) {
		checksum = cktable[(checksum ^ data[i]) & 0xff] ^ ((checksum >> 8) & 0xff);
	}
	return checksum;
}

StenturaRequest* CreateRequest(BYTE seq, WORD action, BYTE* data, unsigned int datalen, WORD p1 = 0, WORD p2 = 0, WORD p3 = 0, WORD p4 = 0, WORD p5 = 0) {
	StenturaRequest* result;
	if (datalen == 0) {
		result = (StenturaRequest*)malloc(sizeof(StenturaRequest));
		result->length = sizeof(StenturaRequest);
	}
	else {
		result = (StenturaRequest*)malloc(sizeof(StenturaRequest)+datalen + sizeof(WORD));
		result->length = sizeof(StenturaRequest)+datalen + sizeof(WORD);
	}

	//MessageBox(NULL, (std::to_wstring(sizeof(StenturaRequest)) + TEXT(":") + std::to_wstring(((LONGLONG)result - (LONGLONG)&(result->checksum)))).c_str(), TEXT("SIZE"), MB_OK);

	result->lead = 0x01;
	result->seq = seq;
	result->action = action;
	result->p1 = p1;
	result->p2 = p2;
	result->p3 = p3;
	result->p4 = p4;
	result->p5 = p5;
	result->checksum = StenturaChecksum(((BYTE*)result) + 1, sizeof(StenturaRequest)-sizeof(BYTE)-sizeof(WORD));

	if (datalen != 0) {
		WORD cks = StenturaChecksum(data, datalen);
		memcpy(((BYTE*)result) + sizeof(StenturaRequest), data, datalen);
		memcpy(((BYTE*)result) + sizeof(StenturaRequest)+datalen, &cks, sizeof(WORD));
	}

	return result;
}

bool ReadResponseCyle(BYTE &seq, StenturaRequest* request, StenturaResponse* response, BYTE* &rdata, bool first = false) {
	HANDLE harray[2] = { readevent, shutoff };
	OVERLAPPED overlap;
	DWORD read = 0;

	memset(&overlap, 0, sizeof(overlap));
	overlap.hEvent = readevent;

	response->seq = 0;
	response->action = 0;
	response->length = 0;
	request->seq = seq;

	if (!WriteFile(com, request, request->length, NULL, &overlap)) {
		DWORD err = GetLastError();
		if (err != ERROR_IO_PENDING) {
			TCHAR lpMsgBuf[500] = TEXT("\0");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 500, NULL);
			MessageBox(NULL, (tstring(TEXT("WriteFile Error: ")) + lpMsgBuf).c_str(), (TEXT("Error ") + std::to_wstring(err)).c_str(), MB_OK);
			return false;
		}
	}

	WaitForMultipleObjects(2, harray, FALSE, INFINITE);

	if (!running) {
		return false;
	}

	if (GetOverlappedResult(com, &overlap, &read, FALSE)) {
		if (read < request->length) {
			MessageBox(NULL, (std::to_wstring(read) + tstring(TEXT(": ")) + TEXT("Unable to send full request")).c_str(), TEXT("Error"), MB_OK);
			return false;
		}
	}
	else {
		DWORD err = GetLastError();
		TCHAR lpMsgBuf[500] = TEXT("\0");
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 500, NULL);
		MessageBox(NULL, (tstring(TEXT("Error: ")) + lpMsgBuf).c_str(), (TEXT("Error ") + std::to_wstring(err)).c_str(), MB_OK);
		return false;
	}


	read = 0;
	do{
		memset(&overlap, 0, sizeof(overlap));
		overlap.hEvent = readevent;
		ReadFile(com, (BYTE*)(response) + read, sizeof(StenturaResponse)-read, NULL, &overlap);
		WaitForMultipleObjects(2, harray, FALSE, INFINITE);
		DWORD oldread = read;
		GetOverlappedResult(com, &overlap, &read, FALSE);
		read += oldread;
	} while (running && read > 0 && read < sizeof(StenturaResponse));

	if (!running) {
		return false;
	}

	rdata = NULL;


	// timed out -- stentura protocal is to re-send request
	int retries = 0;
	while (read == 0 && running && retries < 3 && !first) {
		memset(&overlap, 0, sizeof(overlap));
		overlap.hEvent = readevent;
		WriteFile(com, request, request->length, NULL, &overlap);
		WaitForMultipleObjects(2, harray, FALSE, INFINITE);
		GetOverlappedResult(com, &overlap, &read, FALSE);
		
		if (read < request->length) {
			MessageBox(NULL, TEXT("Unable to send full request"), TEXT("Error"), MB_OK);
			return false;
		}

		if (!running) {
			return false;
		}

		read = 0;

		do{
			memset(&overlap, 0, sizeof(overlap));
			overlap.hEvent = readevent;
			ReadFile(com, (BYTE*)(response)+read, sizeof(StenturaResponse)-read, NULL, &overlap);
			WaitForMultipleObjects(2, harray, FALSE, INFINITE);
			DWORD oldread = read;
			GetOverlappedResult(com, &overlap, &read, FALSE);
			read += oldread;
		} while (running && read > 0 && read < sizeof(StenturaResponse));

		if (read == 0)
			retries++;
	}

	if (!running) {
		return false;
	}

	if (retries == 3) {
		//refused to respond
		// like plover, we now ignore a failure to respond to an open request
		if (first)
			return true;
		MessageBox(NULL, TEXT("Stentura refused to respond to request"), TEXT("Error"), MB_OK);
		running = false;
		return false;
	}

	//grab any data associated with the response
	if (response->length > sizeof(StenturaResponse)) {
		rdata = (BYTE*)malloc(response->length - sizeof(StenturaResponse));

		read = 0;
		do{
			memset(&overlap, 0, sizeof(overlap));
			overlap.hEvent = readevent;
			ReadFile(com, rdata + read, response->length - sizeof(StenturaResponse) - read, NULL, &overlap);
			WaitForMultipleObjects(2, harray, FALSE, INFINITE);
			DWORD oldread = read;
			GetOverlappedResult(com, &overlap, &read, FALSE);
			read += oldread;
		} while (running && read > 0 && read < response->length - sizeof(StenturaResponse));
	}
	

	if (!running) {
		if (rdata != NULL) {
			free(rdata);
			rdata = NULL;
		}
		return false;
	}

	if (response->seq == seq && response->action == request->action) {
		//valid response recieved for the request
		seq++;
		return true;
	}
	else {
		if (rdata != NULL) {
			free(rdata);
			rdata = NULL;
		}
		seq++;
		return false;
	}
}


bool ReadSFile(BYTE& seq, WORD &block, WORD &bt, std::vector<DWORD>* results) {
	StenturaRequest* read = CreateRequest(seq, 0xB, NULL, 0, 1, 0, 512, block, bt);
	StenturaResponse response;
	BYTE* rdata = NULL;

	bool result = ReadResponseCyle(seq, read, &response, rdata);

	while (result && running && response.error == 0 && response.p1 != 0) {
		free(read);

		if (rdata != NULL) {
			if (results != NULL) {
				DWORD* dwords = (DWORD*)rdata;
				for (int i = 0; i < response.p1 / 4; i++) {
					results->push_back(dwords[i]);
				}
			}
			free(rdata);
			rdata = NULL;
		}

		bt += response.p1;
		if (bt >= 512) {
			block += 1;
			bt -= 512;
		}
		read = CreateRequest(seq, 0xB, NULL, 0, 1, 0, 512, block, bt);
		result = ReadResponseCyle(seq, read, &response, rdata);
	}


	free(read);
	if (rdata != NULL) {
		free(rdata);
		rdata = NULL;
	}


	// I have decided to ignore errors for the moment

	//if (response.error != 0) {
	//	MessageBox(NULL, TEXT("Stentura reported file read error"), TEXT("Error"), MB_OK);
	//	running = false;
	//	return false;
	//}

	return true;
}

BYTE reverse_byte(BYTE x)
{
	static const BYTE table[] = {
		0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
		0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
		0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
		0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
		0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
		0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
		0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
		0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
		0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
		0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
		0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
		0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
		0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
		0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
		0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
		0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
		0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
		0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
		0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
		0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
		0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
		0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
		0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
		0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
		0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
		0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
		0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
		0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
		0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
		0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
		0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
		0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
	};
	return table[x];
}

DWORD WINAPI Stentura(LPVOID lpParam)
{
	BYTE seq = 0;


	StenturaRequest* open = CreateRequest(seq, 0xA, (BYTE*)REAL_FILE, strnlen_s(REAL_FILE, 100)+1, 'A'); // note - not sending terminating null -- is this correct?
	StenturaResponse response;
	BYTE* rdata = NULL;

	if (!ReadResponseCyle(seq, open, &response, rdata, true)) {
		MessageBox(NULL, TEXT("Failed to open realtime file on Stentura"), TEXT("Error"), MB_OK);
		free(open);
		SetEvent(handoff);
		return 0;
	}

	// I have decided to ignore errors for the moment
	//if (response.error != 0) {
	//	MessageBox(NULL, TEXT("Stentura reported an error on opening file REALTIME.000"), TEXT("Error"), MB_OK);
	//	free(open);
	//	if (rdata != NULL)
	//		free(rdata);
	//	SetEvent(handoff);
	//	return 0;
	//}
	
	WORD block = 0;
	WORD bt = 0;
	std::vector<DWORD> data;
	unsigned __int32* cast = (unsigned __int32*)(&inputstate.stroke[0]);
	union {
		DWORD keys;
		BYTE each[4];
	} easeofuse;

	if (ReadSFile(seq, block, bt, NULL)) { //begin by reading and dicarding all existing data in realtime file; 
		bool result = ReadSFile(seq, block, bt, &data);
		while (running && result) {
			//process
			for (auto i = data.cbegin(); i != data.cend(); i++) {
				easeofuse.keys = (*i);
				if ((easeofuse.each[0] & 0xC0) == 0xC0 && (easeofuse.each[1] & 0xC0) == 0xC0 && (easeofuse.each[2] & 0xC0) == 0xC0 && (easeofuse.each[3] & 0xC0) == 0xC0) {
					*cast = ((DWORD)reverse_byte(easeofuse.each[0] & 0x0F) >> 4) |
						((DWORD)reverse_byte(easeofuse.each[2] & 0x3F) << 2) |
						((DWORD)reverse_byte(easeofuse.each[3] & 0x3F) << 8) |
						((DWORD)reverse_byte(easeofuse.each[4] & 0x3F) << 14); //!!! ugly
					if ((easeofuse.each[0] & 0x10) != 0) {
						inputstate.stroke[2] |= 0x40;
					}
					if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
						sendstroke(inputstate.stroke);
					}
				}
				
			}
			data.clear();
			result = ReadSFile(seq, block, bt, &data);
		}
	}

	SetEvent(handoff);
	return 0;
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
	inputstate.stroke[0] = inputstate.stroke[1] = inputstate.stroke[2] = 0;

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
		else {
			//timeout
			prevset = 0;
			if ((inputstate.stroke[0] | inputstate.stroke[1] | inputstate.stroke[2]) != 0) {
				sendstroke(inputstate.stroke);
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
				totalread = 0;
			}
			else {
				//attempt to restore sanity by shifting data back into position
				int i = 1;
				for (; i < 6; i++) {
					if ((buffer[i] & 0x80) != 0)
						break;
				}
				
				for (int j = 0; j + i < 6; j++) {
					buffer[j] = buffer[j + i];
				}

				totalread = 6-i;
			}
			
			
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


HANDLE openCom(tstring port, int baud, int timeoutms) {

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
			timeouts.ReadTotalTimeoutConstant = timeoutms;
			timeouts.WriteTotalTimeoutConstant = timeoutms;
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