#include "stdafx.h"

#include "pview.h"
#include "globals.h"
#include "texthelpers.h"
#include "resource.h"
#include <Richedit.h>
#include <list>

pdata projectdata;

bool settingsel = false;

void SetTextSel(int min, int max) {
	CHARRANGE crnew;
	int index = projectdata.strokes.size();
	projectdata.selectionmax = max;
	projectdata.selectionmin = min;

	if (index == 0) {
		return;
	}

	if (max == 0) {
		crnew.cpMax = 0;
	}
	if (min == 0) {
		crnew.cpMin = 0;
	}

	auto it = (projectdata.strokes.cend()--);
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (index == max) {
			crnew.cpMax = ((indexedtext*)((*it)->textout))->startingindex + (*it)->textout->text.length();
		}
		if (index == min) {
			crnew.cpMin = ((indexedtext*)((*it)->textout))->startingindex + (*it)->textout->text.length();
		}
		index--;
	}

	SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, 0, (LPARAM)&crnew);
}

int StrokeFromTextIndx(int txtindex) {
	int index = projectdata.strokes.size();
	if (index == 0) {
		return 0;
	}

	auto it = (projectdata.strokes.cend()--);
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (txtindex >= ((indexedtext*)((*it)->textout))->startingindex + (*it)->textout->text.length() / 2) {
			return index;
		}
		index--;
	}
	if (it == projectdata.strokes.cbegin()) {
		if (txtindex >= ((indexedtext*)((*it)->textout))->startingindex + (*it)->textout->text.length() / 2) {
			index = 1;
		}
		else {
			index = 0;
		}
	}

	return index;
}

INT_PTR CALLBACK PViewProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg) {
	case WM_ACTIVATE:
		if (wParam == 0)
			modelesswnd = NULL;
		else
			modelesswnd = hwndDlg;
		return FALSE;
	case WM_SIZE:
	{
					RECT rt;
					GetClientRect(hwndDlg, &rt);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKELIST), NULL, 0, 0, rt.bottom-rt.top, controls.width, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_MAINTEXT), NULL, controls.width, 0, rt.bottom - rt.top, rt.left - rt.right - controls.width, SWP_NOZORDER);

					SetWindowPos(GetDlgItem(hwndDlg, IDC_PNEW), NULL, 0, rt.bottom-rt.top - 30, 30, rt.left - rt.right, SWP_NOZORDER);

					SetWindowPos(GetDlgItem(hwndDlg, IDC_PENTRY), NULL, 5, rt.bottom - rt.top - 25, 20, 160, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKE), NULL, 170, rt.bottom - rt.top - 25, 20, 160, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_POK), NULL, 235, rt.bottom - rt.top - 25, 20, 30, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PCANCEL), NULL, 270, rt.bottom - rt.top - 25, 20, 30, SWP_NOZORDER);
	}
	case WM_QUIT:
		projectdata.open = false;
		projectdata.dlg = NULL;
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(projectdata.dlg);
		projectdata.open = false;
		projectdata.dlg = NULL;
		return FALSE;
	case WM_INITDIALOG:
		projectdata.dlg = hwndDlg;

		SetParent(GetDlgItem(hwndDlg, IDC_PENTRY), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_PSTROKE), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_POK), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_PCANCEL), GetDlgItem(hwndDlg, IDC_PNEW));
		ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_HIDE);

		SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETEVENTMASK, 0, ENM_SELCHANGE);
		SetWindowText(GetDlgItem(hwndDlg, IDC_PSTROKELIST), TEXT("                       "));

		projectdata.strokes.clear();

		projectdata.open = true;
		projectdata.selectionmax = 0;
		projectdata.selectionmin = 0;
		projectdata.cursorpos = 0;

		PViewProc(hwndDlg, WM_SIZE, 0, 0);
		SetFocus(hwndDlg);
		return FALSE;
	case WM_NOTIFY:
		NMHDR* hdr = (NMHDR*)lParam;
		if (hdr->code == EN_SELCHANGE && !settingsel) {
			settingsel = true;
			SELCHANGE* s = (SELCHANGE*)lParam;
			if (hdr->hwndFrom == GetDlgItem(hwndDlg, IDC_PSTROKELIST)) {
				if (s->chrg.cpMin == s->chrg.cpMax) {
					//single line
					int line = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, s->chrg.cpMin);
					int lineindex = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
					CHARRANGE crnew;
					crnew.cpMin = lineindex;
					crnew.cpMax = lineindex + 23;
					SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
					SetTextSel(line, line);
				}
				else {
					int line = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, s->chrg.cpMin);
					int lineb = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, s->chrg.cpMax);
					int lineindex = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
					int lineindexb = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
					CHARRANGE crnew;
					crnew.cpMin = lineindex;
					crnew.cpMax = lineindexb + 23;
					SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
					SetTextSel(line, lineb);
				}
			}
			else if (hdr->hwndFrom == GetDlgItem(hwndDlg, IDC_MAINTEXT)) {
				SetTextSel(StrokeFromTextIndx(s->chrg.cpMin), StrokeFromTextIndx(s->chrg.cpMax));
			}
			settingsel = false;
		}
		return TRUE;
	}
	
	return FALSE;
	}