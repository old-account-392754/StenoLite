#include "stdafx.h"

#include "pview.h"
#include "globals.h"
#include "texthelpers.h"
#include "resource.h"
#include <Richedit.h>
#include <list>
#include <db.h>
#include <Windowsx.h>
#include <Commdlg.h>

pdata projectdata;



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
	int iindex = 0;

	auto it = (projectdata.strokes.end());
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (iindex == index) {
			return it;
		}
		iindex++;
	}

	return it;
}

void SetTextSel(unsigned int min, unsigned int max) {
	CHARRANGE crnew;
	unsigned int index = 1;
	projectdata.selectionmax = max;
	projectdata.selectionmin = min;


	if (max == 0) {
		crnew.cpMax = 0;
	}
	if (min == 0) {
		crnew.cpMin = 0;
	}

	if (projectdata.strokes.size() > 0){
	
		auto it = (--projectdata.strokes.cend());
		int pos = 0;
		for (; it != projectdata.strokes.cbegin(); it--) {
			if (index == max) {
				crnew.cpMax = pos + (*it)->textout->text.length();
			}
			if (index == min) {
				crnew.cpMin = pos + (*it)->textout->text.length();
			}
			if ((*it)->textout->first == *it)
				pos += (*it)->textout->text.length();
			index++;
		}
		if (index <= max) {
			crnew.cpMax = pos + (*it)->textout->text.length();
		}
		if (index <= min) {
			crnew.cpMin = pos + (*it)->textout->text.length();
		}
	}

	SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, 0, (LPARAM)&crnew);
}

std::list<singlestroke*>::iterator GetItemByText(unsigned int textindex) {
	int iindex = projectdata.strokes.size();
	if (iindex == 0) {
		return projectdata.strokes.end();
	}


	auto it = (--projectdata.strokes.end());
	int pos = 0;
	for (; it != projectdata.strokes.cbegin(); it--) {
		if ((*it)->textout->first == *it) {
			if (textindex < pos + (*it)->textout->text.length() / 2) {
				it++;
				return it;
			}
			pos += (*it)->textout->text.length();
		}
		iindex--;
	}

	if (it == projectdata.strokes.cbegin()) {
		if (textindex < pos + (*it)->textout->text.length() / 2) {
			it++;
			return it;
		}
		else {
			return it;
		}
	}

	return it;
}

int StrokeFromTextIndx(unsigned int txtindex) {
	unsigned int index = 1;
	if (projectdata.strokes.size() == 0) {
		return 0;
	}

	auto it = (--projectdata.strokes.cend());
	int pos = 0;
	for (; it != projectdata.strokes.cbegin(); it--) {
		if (txtindex < pos + (*it)->textout->text.length() / 2) {
			return index - 1;
		}

		if ((*it)->textout->first == *it) {
			pos += (*it)->textout->text.length();
		}
		index++;
	}
	if (it == projectdata.strokes.cbegin()) {
		if (txtindex < pos + (*it)->textout->text.length() / 2) {
			return index - 1;
		}
		else {
			return index;
		}
	}

	return index;
}



LRESULT CALLBACK StrokeList(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int initialchr = 0;
	static bool capturing = false;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		POINTL p;
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		initialchr = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_CHARFROMPOS, 0, (LPARAM)(&p));
		capturing = true;
		//return 0;
	case WM_MOUSEMOVE:
		if (!capturing)
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	{
			POINTL p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			int cchar = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_CHARFROMPOS, 0, (LPARAM)(&p));
			 
			int lesser;
			int greater;
			if (initialchr > cchar) {
				lesser = cchar;
				greater = initialchr;
			}
			else {
				lesser = initialchr;
				greater = cchar;
			}
						 projectdata.settingsel = true;
						 int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, lesser);
						 int lineb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, greater);

						 //protect against selecting in the middle of a word
						 auto it = GetItem(line);
						 if (it != projectdata.strokes.cend()) {
							 while ((*it)->textout->first != (*it)) {
								 it--;
								 line++;
							 }
						 }

						 if (line > lineb)
							 lineb = line;

						 if (lineb != line) {
							 it = GetItem(lineb);
							 if (it != projectdata.strokes.cend()) {
								 while ((*it)->textout->first != (*it)) {
									 it--;
									 lineb++;
								 }
							 }
						 }

						 int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
						 int lineindexb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
						 CHARRANGE crnew;
						 crnew.cpMin = lineindex + 23;
						 crnew.cpMax = lineindexb + 23;
						 SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
						 SetTextSel(line, lineb);
						 projectdata.settingsel = false;
	}
		return 0;
	case WM_LBUTTONUP:
		capturing = false;
		ReleaseCapture();
		return 0;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK MainText(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int initialchr = 0;
	static bool capturing = false;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		POINTL p;
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		initialchr = SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_CHARFROMPOS, 0, (LPARAM)(&p));
		capturing = true;
		//return 0;
	case WM_MOUSEMOVE:
		if (!capturing)
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		{
			POINTL p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			int cchar = SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_CHARFROMPOS, 0, (LPARAM)(&p));

			int lesser;
			int greater;
			if (initialchr > cchar) {
				lesser = cchar;
				greater = initialchr;
			}
			else {
				lesser = initialchr;
				greater = cchar;
			}
			projectdata.settingsel = true;
			int line = StrokeFromTextIndx(lesser);
			int lineb = StrokeFromTextIndx(greater);
			int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line, 0);
			int lineindexb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, lineb, 0);
			CHARRANGE crnew;
			crnew.cpMin = lineindex + 23;
			crnew.cpMax = lineindexb + 23;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
			SetTextSel(line, lineb);

			projectdata.settingsel = false;
		}
		return 0;
	case WM_LBUTTONUP:
		capturing = false;
		ReleaseCapture();
		return 0;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK PViewProc(_In_  HWND hwndDlg, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam) {
	NMHDR* hdr;

	switch (uMsg) {
	case WM_ACTIVATE:
		if (wParam == 0) {
			modelesswnd = NULL;
			projectdata.open = false;
		}
		else {
			modelesswnd = hwndDlg;
			projectdata.open = true;
		}
		return FALSE;
	case WM_SIZE:
	{
					RECT rt;
					GetClientRect(hwndDlg, &rt);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKELIST), NULL, 0, 0, projectdata.textwidth, rt.bottom - rt.top, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_MAINTEXT), NULL, projectdata.textwidth, 0, rt.right - rt.left - projectdata.textwidth, rt.bottom - rt.top, SWP_NOZORDER);

					SetWindowPos(GetDlgItem(hwndDlg, IDC_PNEW), NULL, 0, rt.bottom-rt.top - 30, rt.right - rt.left, 30, SWP_NOZORDER);

					SetWindowPos(GetDlgItem(hwndDlg, IDC_PENTRY), NULL, 5, rt.bottom - rt.top - 25, 160, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PSTROKE), NULL, 170, rt.bottom - rt.top - 25, 160, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_POK), NULL, 235, rt.bottom - rt.top - 25, 30, 20, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_PCANCEL), NULL, 270, rt.bottom - rt.top - 25, 30, 20, SWP_NOZORDER);
	}
		return TRUE;
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
		projectdata.d->close();
		return FALSE;
	case WM_INITDIALOG:
	{
						  
						  projectdata.dlg = hwndDlg;
						  projectdata.d = new dictionary("");
						  tstring form;
						  if (sharedData.currentd != NULL)
							  form = sharedData.currentd->format;
						  else
							  form = TEXT("#STKPWHRAO*EUFRPBLGTSDZ");

						  if (!projectdata.d->opentransient(form))
							  MessageBox(NULL, TEXT("Unable to open project's dictionary"), TEXT("Error"), MB_OK);

						  SetParent(GetDlgItem(hwndDlg, IDC_PENTRY), GetDlgItem(hwndDlg, IDC_PNEW));
						  SetParent(GetDlgItem(hwndDlg, IDC_PSTROKE), GetDlgItem(hwndDlg, IDC_PNEW));
						  SetParent(GetDlgItem(hwndDlg, IDC_POK), GetDlgItem(hwndDlg, IDC_PNEW));
						  SetParent(GetDlgItem(hwndDlg, IDC_PCANCEL), GetDlgItem(hwndDlg, IDC_PNEW));
						  ShowWindow(GetDlgItem(hwndDlg, IDC_PNEW), SW_HIDE);

						 
						  PARAFORMAT2 pf;
						  memset(&pf, 0, sizeof(pf));
						  pf.cbSize = sizeof(PARAFORMAT2);
						  pf.dwMask = PFM_OFFSETINDENT | PFM_SPACEAFTER | PFM_OFFSET;
						  pf.dxStartIndent = (24 * 1440) / 72;
						  pf.dySpaceAfter = (5 * 1440) / 72;
						  pf.dxOffset = -pf.dxStartIndent;
						  SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETPARAFORMAT, 0, (LPARAM)&pf);

						  CHARFORMAT2 cf;
						  memset(&cf, 0, sizeof(cf));
						  cf.cbSize = sizeof(CHARFORMAT2);
						  cf.dwMask = CFM_FACE | CFM_SIZE;
						  cf.yHeight = (10 * 1440) / 72;
						  _tcscpy_s(cf.szFaceName, LF_FACESIZE, TEXT("Consolas"));

						  if (SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf) == 0) {
							  _tcscpy_s(cf.szFaceName, LF_FACESIZE, TEXT("Courier New"));
							  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						  }

						  memset(&cf, 0, sizeof(cf));
						  cf.cbSize = sizeof(CHARFORMAT2);
						  cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
						  cf.yHeight = (settings.fsize * 1440) / 72;
						  cf.wWeight = settings.fweight;
						  _tcscpy_s(cf.szFaceName, LF_FACESIZE, settings.fname.c_str());

						  SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

						  SetWindowText(GetDlgItem(hwndDlg, IDC_PSTROKELIST), TEXT("                       "));
						  CHARRANGE crnew;
						  crnew.cpMax = 23;
						  crnew.cpMin = 23;
						  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crnew);
						  //SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_SETEVENTMASK, 0, ENM_SELCHANGE);
						  //SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETEVENTMASK, 0, ENM_SELCHANGE);

						  SetWindowSubclass(GetDlgItem(hwndDlg, IDC_PSTROKELIST), &StrokeList, 1245, NULL);
						  SetWindowSubclass(GetDlgItem(hwndDlg, IDC_MAINTEXT), &MainText, 1246, NULL);

						  projectdata.strokes.clear();

						  projectdata.addingnew = false;
						  projectdata.open = true;
						  projectdata.selectionmax = 0;
						  projectdata.selectionmin = 0;
						  projectdata.cursorpos = 0;

						  POINTL pt;
						  SendMessage(GetDlgItem(hwndDlg, IDC_PSTROKELIST), EM_POSFROMCHAR, (WPARAM)&pt, 1);
						  projectdata.textwidth = pt.x * 23;

						  PViewProc(hwndDlg, WM_SIZE, 0, 0);
						  SetFocus(hwndDlg);
	}
		return FALSE;
	case WM_NOTIFY:
		hdr = (NMHDR*)lParam;
		
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
		case IDM_PFONT:
			CHOOSEFONT cf;
			memset(&cf, 0, sizeof(cf));
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = hwndDlg;
			cf.Flags =  CF_NOSCRIPTSEL | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
			cf.rgbColors = RGB(0, 0, 0);

			LOGFONT lf;
			memset(&lf, 0, sizeof(lf));
			HDC screen = GetDC(NULL);
			lf.lfHeight = -MulDiv(settings.fsize, GetDeviceCaps(screen, LOGPIXELSY), 72);
			ReleaseDC(NULL, screen);
			lf.lfWeight = settings.fweight;

			_tcscpy_s(lf.lfFaceName, LF_FACESIZE, settings.fname.c_str());
			cf.lpLogFont = &lf;

			if (ChooseFont(&cf)) {
				settings.fsize = cf.iPointSize / 10;
				settings.fweight = lf.lfWeight;
				settings.fname = lf.lfFaceName;

				CHARFORMAT2 cf;
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
				cf.yHeight = (settings.fsize * 1440) / 72;
				cf.wWeight = settings.fweight;
				_tcscpy_s(cf.szFaceName, LF_FACESIZE, settings.fname.c_str());

				SendMessage(GetDlgItem(hwndDlg, IDC_MAINTEXT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
			}
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


void LaunchProjDlg(HINSTANCE hInst) {
	if (sharedData.currentd == NULL) {
		MessageBox(NULL, TEXT("No dictionary selected"), TEXT(""), MB_OK);
		return;
	}
	if (projectdata.dlg == NULL) {
		projectdata.dlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PROJECT), NULL, PViewProc);
		if (projectdata.dlg == NULL)
		{
			TCHAR* msg;
			// Ask Windows to prepare a standard message for a  code:
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);
			MessageBox(NULL, msg, TEXT("Error"), MB_OK);
		}
			

		ShowWindow(projectdata.dlg, SW_SHOW);
	}
}