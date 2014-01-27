#include "stdafx.h"

#include "pview.h"
#include "globals.h"
#include "texthelpers.h"
#include "resource.h"
#include <Richedit.h>
#include <list>
#include <db.h>

pdata projectdata;

bool settingsel = false;

void PViewNextFocus() {
	if (projectdata.addingnew) {
		switch (projectdata.focusedcontrol) {
		case 0:  projectdata.focusedcontrol = 1; inputstate.sendasstrokes = false; inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PENTRY);
			SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		case 1:  projectdata.focusedcontrol = 2; SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(projectdata.dlg, IDC_POK), TRUE); break;
		case 2:  projectdata.focusedcontrol = 3; SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(projectdata.dlg, IDC_PCANCEL), TRUE); break;
		default: projectdata.focusedcontrol = 0; inputstate.sendasstrokes = true;  inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PSTROKE);
			SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		}
	}
}

std::list<singlestroke*>::iterator GetItem(int index) {
	int iindex = projectdata.strokes.size();
	if (iindex == 0) {
		return projectdata.strokes.end();
	}

	auto it = (projectdata.strokes.end());
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (iindex == index) {
			return it;
		}
		iindex--;
	}

	return it;
}

void AdjustTextStart(std::list<singlestroke*>::iterator last, int adjustment) {
	auto it = (last--);
	for (; it != projectdata.strokes.cbegin(); it--) {
		if ((*it)->textout->first == (*it)) {
			((indexedtext*)((*it)->textout))->startingindex += adjustment;
		}
	}
	if ((*it)->textout->first == (*it)) {
		((indexedtext*)((*it)->textout))->startingindex += adjustment;
	}
}

void SetTextSel(unsigned int min, unsigned int max) {
	CHARRANGE crnew;
	unsigned int index = projectdata.strokes.size();
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

int StrokeFromTextIndx(unsigned int txtindex) {
	unsigned int index = projectdata.strokes.size();
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
	NMHDR* hdr;

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
		inputstate.redirect = NULL;
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(projectdata.dlg);
		projectdata.open = false;
		projectdata.dlg = NULL;
		inputstate.redirect = NULL;
		return FALSE;
	case WM_INITDIALOG:
		projectdata.dlg = hwndDlg;
		projectdata.d = new dictionary("");
		if (!projectdata.d->opentransient())
			MessageBox(NULL, TEXT("Unable to open project's dictionary"), TEXT("Error"), MB_OK);

		SetParent(GetDlgItem(hwndDlg, IDC_PENTRY), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_PSTROKE), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_POK), GetDlgItem(hwndDlg, IDC_PNEW));
		SetParent(GetDlgItem(hwndDlg, IDC_PCANCEL), GetDlgItem(hwndDlg, IDC_PNEW));
		ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_HIDE);

		SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETEVENTMASK, 0, ENM_SELCHANGE);
		SetWindowText(GetDlgItem(hwndDlg, IDC_PSTROKELIST), TEXT("                       "));

		projectdata.strokes.clear();

		projectdata.addingnew = false;
		projectdata.open = true;
		projectdata.selectionmax = 0;
		projectdata.selectionmin = 0;
		projectdata.cursorpos = 0;

		PViewProc(hwndDlg, WM_SIZE, 0, 0);
		SetFocus(hwndDlg);
		return FALSE;
	case WM_NOTIFY:
		hdr = (NMHDR*)lParam;
		if (hdr->code == EN_SELCHANGE && !settingsel) {
			settingsel = true;
			SELCHANGE* s = (SELCHANGE*)lParam;
			if (hdr->hwndFrom == GetDlgItem(hwndDlg, IDC_PSTROKELIST)) {
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
			else if (hdr->hwndFrom == GetDlgItem(hwndDlg, IDC_MAINTEXT)) {
				int line = StrokeFromTextIndx(s->chrg.cpMin);
				int lineb = StrokeFromTextIndx(s->chrg.cpMax);
				int lineindex = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
				int lineindexb = SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
				CHARRANGE crnew;
				crnew.cpMin = lineindex;
				crnew.cpMax = lineindexb + 23;
				SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
				SetTextSel(line, lineb);
			}
			settingsel = false;
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_POK:
		{
						ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_HIDE);
						projectdata.addingnew = false;
						inputstate.redirect = NULL;

						tstring txt = getWinStr(GetDlgItem(hwndDlg, IDC_PENTRY));
						tstring stroke = getWinStr(GetDlgItem(hwndDlg, IDC_PSTROKE));
						int numstrokes = 0;

						unsigned __int8* sdata = texttomultistroke(stroke, numstrokes, sharedData.currentd->format);
						std::string trmd = trimstr(ttostr(txt), " ");

						DB_TXN* trans;
						projectdata.d->env->txn_begin(projectdata.d->env, NULL, &trans, 0);
						projectdata.d->addDItem(sdata, numstrokes * 3, trmd, trans);
						trans->commit(trans, 0);

						delete sdata;
		}
			break;
		case IDC_PCANCEL:
			ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_HIDE);
			projectdata.addingnew = false;
			inputstate.redirect = NULL;
			break;
		}
		return TRUE;
	case WM_NEWITEMDLG:
		ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_SHOW);
		projectdata.focusedcontrol = 0;
		inputstate.redirect = GetDlgItem(projectdata.dlg, IDC_PSTROKE);
		inputstate.sendasstrokes = true;
		SendMessage(projectdata.dlg, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE);
		projectdata.addingnew = true;
		return TRUE;
	}
	
	return FALSE;
	}