#include "stdafx.h"

#ifndef MY_NED_H
#define MY_NED_H

#include <Windows.h>
#include "globals.h"

void NewDlgNextFocus();
INT_PTR CALLBACK NewEntryProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam);
void LaunchEntryDlg(HINSTANCE hInst);

#endif