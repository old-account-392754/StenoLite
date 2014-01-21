#include "stdafx.h"
#include "setmode.h"

#include "globals.h"
#include <regex>
#include <string>
#include "texthelpers.h"

void setMode(int mode) {
	static int cmode = 0;
	if (cmode == 0) {

	}
	else if (cmode == 1) {
		UnhookWindowsHookEx(inputstate.handle);
	}
	else if (cmode == 2) {

	}
	cmode = mode;


	if (cmode == 0) {

	}
	else if (cmode == 1) {
		TCHAR pathbuffer[MAX_PATH];
		GetModuleFileName(NULL, pathbuffer, MAX_PATH);

		const static std::regex rx("\\\\[^\\\\]*$");
		std::string pth = TCHARtostr(pathbuffer, MAX_PATH);
		std::string file = std::regex_replace(pth, rx, "\\SLKeyCap.dll");
		std::copy(file.begin(), file.end(), pathbuffer);
		pathbuffer[file.size()] = 0;

		HMODULE dll = NULL;
		dll = LoadLibrary(pathbuffer);
		if (dll == NULL) {
			MessageBox(NULL, TEXT("Could not load capture dll\r\nKeyboard mode will not function"), TEXT("Error"), MB_OK);
		}
		else {

			HOOKPROC addr = NULL;
			addr = (HOOKPROC)GetProcAddress(dll, "_HookKeys@12");
			if (addr == NULL) {
				MessageBox(NULL, TEXT("Could load hook function\r\nKeyboard mode will not function"), TEXT("Error"), MB_OK);
			}
			else {
				VOID(__cdecl *setmem)(unsigned __int8[256]) = (VOID(__cdecl *)(unsigned __int8[256]))GetProcAddress(dll, "SetSharedMem");
				if (setmem == NULL) {
					MessageBox(NULL, TEXT("Could load setmem\r\nKeyboard mode will not function"), TEXT("Error"), MB_OK);
				}
				setmem(settings.map);

				inputstate.handle = SetWindowsHookEx(WH_KEYBOARD_LL, addr, dll, 0);
				if (inputstate.handle == NULL) {
					MessageBox(NULL, TEXT("Could not hook keyboard\r\nKeyboard mode will not function"), TEXT("Error"), MB_OK);
				}
			}
		}
	}
	else if (cmode == 2) {
		RAWINPUTDEVICE Rid;

		Rid.usUsagePage = 65441;
		Rid.usUsage = 6;
		Rid.dwFlags = RIDEV_INPUTSINK;
		Rid.hwndTarget = controls.main;

		if (RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
			MessageBox(NULL, TEXT("Could not register Treal\r\nTreal mode will not function"), TEXT("Error"), MB_OK);
		}
	}
}
