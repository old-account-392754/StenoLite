#include "stdafx.h"
#include "setmode.h"

#include "globals.h"
#include <regex>
#include <string>
#include "texthelpers.h"
#include "basicserial.h"
#include "resource.h"

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

		buffer += d->format;

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

INT_PTR CALLBACK ComSettings(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hwndDlg, IDC_COMPORT), settings.port.c_str());
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam))
			{
			case IDOK:
				settings.port = getWinStr(GetDlgItem(hwndDlg, IDC_COMPORT));
				EndDialog(hwndDlg, IDOK);
				return TRUE;
			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;
			}
		}
	}
	return FALSE;
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
	else if (cmode == 3 || cmode == 4 || cmode == 5 || cmode == 6) {
		EndThreads();
	}
	cmode = mode;


	if (cmode == 0) {

	}
	else if (cmode == 1) {
		TCHAR pathbuffer[MAX_PATH];
		GetModuleFileName(NULL, pathbuffer, MAX_PATH);

		const static tregex rx(TEXT("\\\\[^\\\\]*$"));
		tstring file = std::regex_replace(pathbuffer, rx, TEXT("\\SLKeyCap.dll"));

		HMODULE dll = NULL;
		dll = LoadLibrary(file.c_str());
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
	else if (cmode == 3) {
		int result = IDOK;
		if (controls.inited) {
			result = DialogBox((HINSTANCE)GetWindowLong(controls.main, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_COM), controls.main, ComSettings);
		}
		if (result == IDOK) {
			//open with a .5 sec read timeout
			if (openCom(settings.port, CBR_9600, 500) != INVALID_HANDLE_VALUE) {
				CreateThread(NULL, 0, TXBolt, NULL, 0, NULL);
			}
			else {
				cmode = 0;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
	}
	else if (cmode == 4) {
		int result = IDOK;
		if (controls.inited) {
			result = DialogBox((HINSTANCE)GetWindowLong(controls.main, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_COM), controls.main, ComSettings);
		}
		if (result == IDOK) {
			if (openCom(settings.port, CBR_38400) != INVALID_HANDLE_VALUE) {
				CreateThread(NULL, 0, Passport, NULL, 0, NULL);
			}
			else {
				cmode = 0;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
	}
	else if (cmode == 5) {
		int result = IDOK;
		if (controls.inited) {
			result = DialogBox((HINSTANCE)GetWindowLong(controls.main, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_COM), controls.main, ComSettings);
		}
		if (result == IDOK) {
			if (openCom(settings.port, CBR_9600) != INVALID_HANDLE_VALUE) {
				CreateThread(NULL, 0, Gemini, NULL, 0, NULL);
			}
			else {
				cmode = 0;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
	}
	else if (cmode == 6) {
		int result = IDOK;
		if (controls.inited) {
			result = DialogBox((HINSTANCE)GetWindowLong(controls.main, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_COM), controls.main, ComSettings);
		}
		if (result == IDOK) {
			// open with a 1.5 second time out
			if (openCom(settings.port, CBR_9600, 1500) != INVALID_HANDLE_VALUE) {
				CreateThread(NULL, 0, Stentura, NULL, 0, NULL);
			}
			else {
				cmode = 0;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
	}
}
