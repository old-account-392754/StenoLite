// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

struct {
	unsigned __int8 keys[4];
	unsigned __int8 stroke[4];
} inputstate;

DWORD WINAPI SendStroke(LPVOID lpParam) {
	__int32 val = (__int32)(lpParam);

	COPYDATASTRUCT MyCDS;
	MyCDS.dwData = NULL;          // function identifier
	MyCDS.cbData = sizeof(__int32);  // size of data
	MyCDS.lpData = &val;           // data structure
	//
	// Call function, passing data in &MyCDS
	//
	HWND hwDispatch = FindWindow(TEXT("STENOLITE826"), NULL);
	if (hwDispatch != NULL)
		SendMessage(hwDispatch, WM_COPYDATA, (WPARAM)(HWND)NULL, (LPARAM)(LPVOID)&MyCDS);
	return 0;
}

static LPVOID lpvMem = NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping
#define MEM_SIZE 256



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	BOOL fInit, fIgnore;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		memset(&inputstate, 0, sizeof(inputstate));

		hMapObject = CreateFileMapping(
			INVALID_HANDLE_VALUE,   // use paging file
			NULL,                   // default security attributes
			PAGE_READWRITE,         // read/write access
			0,                      // size: high 32-bits
			MEM_SIZE,              // size: low 32-bits
			TEXT("dllmemfilemap")); // name of map object
		if (hMapObject == NULL) {
			MessageBox(NULL, TEXT("CreateFileMapping failed"), TEXT("ERROR"), MB_OK);
			return FALSE;
		}

		// The first process to attach initializes memory

		fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

		// Get a pointer to the file-mapped shared memory

		lpvMem = MapViewOfFile(
			hMapObject,     // object to map view of
			FILE_MAP_WRITE, // read/write access
			0,              // high offset:  map from
			0,              // low offset:   beginning
			0);             // default: map entire file
		if (lpvMem == NULL) {
			MessageBox(NULL, TEXT("MapViewOfFile failed"), TEXT("ERROR"), MB_OK);
			return FALSE;
		}

		// Initialize memory if this is the first process

		if (fInit)
			memset(lpvMem, 0, MEM_SIZE);

		//MessageBox(NULL, TEXT("All OK"), TEXT("OK"), MB_OK);

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		fIgnore = UnmapViewOfFile(lpvMem);

		// Close the process's handle to the file-mapping object

		fIgnore = CloseHandle(hMapObject);
		break;
	}
	return TRUE;
}

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

// SetSharedMem sets the contents of the shared memory 
__declspec(dllexport) VOID __cdecl SetSharedMem(unsigned __int8 buf[256]) {
	memcpy(lpvMem, buf, MEM_SIZE);
	}

__declspec(dllexport) LRESULT CALLBACK HookKeys(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	if (nCode < 0 || lpvMem == NULL) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	switch (wParam) {
	case WM_KEYDOWN:
	{
					   KBDLLHOOKSTRUCT* t = (KBDLLHOOKSTRUCT*)lParam;
					   if ((t->flags & LLKHF_INJECTED) != 0) {
						   return CallNextHookEx(NULL, nCode, wParam, lParam);
					   }
					   unsigned __int8 vk = LOBYTE(LOWORD(t->vkCode));
					   __int8 map = ((unsigned __int8*)lpvMem)[vk];
					   if (map != 0) {
						   map = map - 1;
						   if (map < 8) {
							   inputstate.keys[0] |= 1 << map;
							   inputstate.stroke[0] |= inputstate.keys[0];
						   }
						   else if (map < 16) {
							   inputstate.keys[1] |= 1 << (map - 8);
							   inputstate.stroke[1] |= inputstate.keys[1];
						   }
						   else if (map < 24) {
							   inputstate.keys[2] |= 1 << (map - 16);
							   inputstate.stroke[2] |= inputstate.keys[2];
						   }
						   else if (map < 32) {
							   inputstate.keys[3] |= 1 << (map - 24);
							   inputstate.stroke[3] |= inputstate.keys[3];
						   }

						   //_stprintf_s(dtxt, 99, TEXT("%i"), settings.map[vk]);
						   //MessageBox(NULL, dtxt, TEXT("Title"), MB_OK);
					   }
					   else {
						   return CallNextHookEx(NULL, nCode, wParam, lParam);
					   }


	}
		return -1;
	case WM_KEYUP:
	{
					 __int8 pre = inputstate.keys[0] | inputstate.keys[1] | inputstate.keys[2] | inputstate.keys[3];

					 KBDLLHOOKSTRUCT* t = (KBDLLHOOKSTRUCT*)lParam;
					 if ((t->flags & LLKHF_INJECTED) != 0) {
						 return CallNextHookEx(NULL, nCode, wParam, lParam);
					 }

					 unsigned __int8 vk = LOBYTE(LOWORD(t->vkCode));
					 unsigned __int8 map = ((unsigned __int8*)lpvMem)[vk];
					 if (map != 0) {
						 map = map - 1;
						 if (map < 8) {
							 inputstate.keys[0] &= ~(1 << map);
						 }
						 else if (map < 16) {
							 inputstate.keys[1] &= ~(1 << (map - 8));
						 }
						 else if (map < 24) {
							 inputstate.keys[2] &= ~(1 << (map - 16));
						 }
						 else if (map < 32) {
							 inputstate.keys[3] &= ~(1 << (map - 24));
						 }
						 __int8 post =  inputstate.keys[0] | inputstate.keys[1] | inputstate.keys[2] | inputstate.keys[3];
						 if (pre != 0 && post == 0) {
							 //sendstroke(inputstate.stroke);
							 __int32* val = (__int32*)inputstate.stroke;
							 CreateThread(NULL, 0, SendStroke, (LPVOID)(*val), 0, NULL);
							 inputstate.stroke[0] = inputstate.stroke[1] = inputstate.stroke[2] = inputstate.stroke[3] = 0;
						 }
					 }
					 else {
						 return CallNextHookEx(NULL, nCode, wParam, lParam);
					 }


	}
		return -1;
	default:
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
};
		

#ifdef __cplusplus
}         
#endif