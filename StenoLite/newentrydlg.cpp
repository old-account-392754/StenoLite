#include "stdafx.h"
#include "newentrydlg.h"


#include "resource.h"
#include <gdiplus.h>
#include "texthelpers.h"
#include <Windows.h>
#include "fileops.h"
#include "DView.h"

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

void NewDlgNextFocus() {
	if (newwordwin.running) {
		switch (newwordwin.focusedcontrol) {
		case 0:  newwordwin.focusedcontrol = 1;  inputstate.redirect = GetDlgItem(newwordwin.dlgwnd, IDC_TEXT); SendMessage(newwordwin.dlgwnd, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		case 1:  newwordwin.focusedcontrol = 2; SendMessage(newwordwin.dlgwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(newwordwin.dlgwnd, IDOK), TRUE); break;
		case 2:  newwordwin.focusedcontrol = 3; SendMessage(newwordwin.dlgwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(newwordwin.dlgwnd, IDCANCEL), TRUE); break;
		default: newwordwin.focusedcontrol = 0; inputstate.redirect = GetDlgItem(newwordwin.dlgwnd, IDC_STROKE); SendMessage(newwordwin.dlgwnd, WM_NEXTDLGCTL, (WPARAM)inputstate.redirect, TRUE); break;
		}
	}
}

INT_PTR CALLBACK NewEntryProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg) {
	case WM_PAINT:
	{
					 hdc = BeginPaint(hwndDlg, &ps);
					 Graphics gc(hdc);

					 HWND target = NULL;
					 switch (newwordwin.focusedcontrol) {
					 case 0:  target = GetDlgItem(hwndDlg, IDC_STROKE); break;
					 case 1:  target = GetDlgItem(hwndDlg, IDC_TEXT); break;
					 case 2:  target = GetDlgItem(hwndDlg, IDOK); break;
					 default: target = GetDlgItem(hwndDlg, IDCANCEL); break;
					 }
					 RECT ctrrect;
					 POINT topleft;
					 POINT bottomright;
					 GetWindowRect(target, &ctrrect);
					 topleft.x = ctrrect.left;
					 topleft.y = ctrrect.top;
					 bottomright.x = ctrrect.right;
					 bottomright.y = ctrrect.bottom;
					 ScreenToClient(hwndDlg, &topleft);

					 Pen blackPen(Color(155, 10, 10, 10), 4);
					 blackPen.SetAlignment(PenAlignment::PenAlignmentInset);
					 gc.DrawRectangle(&blackPen, topleft.x - 4, topleft.y - 4, (ctrrect.right - ctrrect.left) + 8, (ctrrect.bottom - ctrrect.top) + 8);

					 EndPaint(hwndDlg, &ps);

					 // Sleep(20);
					 return FALSE;
	}
	case WM_KILLFOCUS:
		inputstate.redirect = NULL;
	case WM_QUIT:
		newwordwin.running = false;
		inputstate.redirect = NULL;
		return FALSE;
	case WM_CLOSE:
		newwordwin.running = false;
		ShowWindow(hwndDlg, SW_HIDE);
		inputstate.redirect = NULL;
		newwordwin.dlgwnd = NULL;
		return FALSE;
	case WM_INITDIALOG:
		newwordwin.running = true;
		//SetFocus(GetDlgItem(hwndDlg, IDC_TEXT));
		inputstate.redirect = GetDlgItem(hwndDlg, IDC_STROKE);
		SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_STROKE), TRUE);
		newwordwin.focusedcontrol = 0;
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (HIWORD(wParam) == BN_SETFOCUS) {
				newwordwin.focusedcontrol = 2;
				inputstate.redirect = NULL;
				InvalidateRect(hwndDlg, NULL, TRUE);
				return FALSE;
			}
			else if (HIWORD(wParam) == BN_CLICKED) {
				newwordwin.running = false;
				newwordwin.dlgwnd = NULL;

				tstring txt = getWinStr(GetDlgItem(hwndDlg, IDC_TEXT));
				tstring stroke = getWinStr(GetDlgItem(hwndDlg, IDC_STROKE));
				int numstrokes = 0;
				std::string cstroke = ttostr(stroke);
				unsigned __int8* sdata = texttomultistroke(cstroke, numstrokes, sharedData.currentd->format);

				std::string trmd = trimstr(ttostr(txt), " ");
				DB_TXN* trans;
				sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, 0);
				sharedData.currentd->addDItem( sdata, numstrokes * 3, trmd, trans);
				trans->commit(trans, 0);
				appendUser(sharedData.currentd, cstroke, trmd);

				delete sdata;

				inputstate.redirect = NULL;
				SetForegroundWindow(newwordwin.prevfocus);
				DestroyWindow(hwndDlg);
				if (dviewdata.running)
					addDVEvent(DVE_RESIZE);

				return TRUE;
			}
		case IDCANCEL:
			if (HIWORD(wParam) == BN_SETFOCUS) {
				newwordwin.focusedcontrol = 3;
				inputstate.redirect = NULL;
				InvalidateRect(hwndDlg, NULL, TRUE);
				return FALSE;
			}
			else if (HIWORD(wParam) == BN_CLICKED) {
				newwordwin.running = false;
				newwordwin.dlgwnd = NULL;
				inputstate.redirect = NULL;
				SetForegroundWindow(newwordwin.prevfocus);
				DestroyWindow(hwndDlg);
				return TRUE;
			}
		case IDC_STROKE:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				newwordwin.focusedcontrol = 0;
				inputstate.redirect = GetDlgItem(hwndDlg, IDC_STROKE);
				inputstate.sendasstrokes = true;
				InvalidateRect(hwndDlg, NULL, TRUE);
			}
			else if (HIWORD(wParam) == EN_CHANGE) {
				tstring stroke = getWinStr(GetDlgItem(hwndDlg, IDC_STROKE));
				int numstrokes = 0;
				std::string cstroke = ttostr(stroke);
				unsigned __int8* sdata = texttomultistroke(cstroke, numstrokes, sharedData.currentd->format);

				std::string result;
				SetDlgItemText(hwndDlg, IDC_STROKEREPORT, TEXT("No existing entry"));
				if (numstrokes >= 1 && sharedData.currentd != NULL) {
					DB_TXN* trans;
					sharedData.currentd->env->txn_begin(sharedData.currentd->env, NULL, &trans, DB_READ_UNCOMMITTED);

					if (sharedData.currentd->findDItem(sdata, numstrokes * 3, result, trans)) {
						result = "Current entry: " + result;
						SetDlgItemText(hwndDlg, IDC_STROKEREPORT, strtotstr(result).c_str());
					}

					trans->commit(trans, 0);
				}
				delete sdata;
				
			}
			return FALSE;
		case IDC_TEXT:
			if (HIWORD(wParam) == EN_SETFOCUS) {
				newwordwin.focusedcontrol = 1;
				inputstate.sendasstrokes = false;
				inputstate.redirect = GetDlgItem(hwndDlg, IDC_TEXT);
				InvalidateRect(hwndDlg, NULL, TRUE);
			}
			if (HIWORD(wParam) == EN_CHANGE) {
			}
			return FALSE;
		default:
			return FALSE;
		}
	default:
		break;
	}
	return FALSE; // message not processed
}

void LaunchEntryDlg(HINSTANCE hInst) {
	if (newwordwin.dlgwnd == NULL) {
		newwordwin.focusedcontrol = 0;
		newwordwin.dlgwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_NEWITEM), controls.main, NewEntryProc);
		ShowWindow(newwordwin.dlgwnd, SW_SHOW);
		newwordwin.running = true;
	}
	else if (!newwordwin.running) {
		newwordwin.focusedcontrol = 0;
		SetDlgItemText(newwordwin.dlgwnd, IDC_STROKE, TEXT(""));
		SetDlgItemText(newwordwin.dlgwnd, IDC_TEXT, TEXT(""));
		SetDlgItemText(newwordwin.dlgwnd, IDC_STROKEREPORT, TEXT(""));
		ShowWindow(newwordwin.dlgwnd, SW_SHOW);
		newwordwin.running = true;
	}
}