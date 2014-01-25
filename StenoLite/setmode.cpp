#include "stdafx.h"
#include "setmode.h"

#include "globals.h"
#include <regex>
#include <string>
#include "texthelpers.h"

void setDictionary(dictionary* d) {
	bool send = false;
	if (sharedData.currentd == NULL) {
		send = true;
	}
	else if (sharedData.currentd->format.compare(d->format) != 0) {
		send = true;
	}
	if (send) {
		tstring buffer;
		if (GetWindowTextLength(controls.mestroke) > 1)
			buffer += TEXT("\r\n");

		buffer += strtotstr(d->format);

		int lines = SendMessage(controls.mestroke, EM_GETLINECOUNT, 0, 0);
		if (lines > controls.numlines - 1) {
			int end = SendMessage(controls.mestroke, EM_LINEINDEX, 1, 0);
			SendMessage(controls.mestroke, EM_SETSEL, 0, end);
			SendMessage(controls.mestroke, EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
		}

		int len = SendMessage(controls.mestroke, WM_GETTEXTLENGTH, 0, 0);
		SendMessage(controls.mestroke, EM_SETSEL, len, len);
		SendMessage(controls.mestroke, EM_REPLACESEL, FALSE, (LPARAM)(buffer.c_str()));
	}
	
	sharedData.currentd = d;
}

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
		std::string pth = ttostr(pathbuffer);
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
