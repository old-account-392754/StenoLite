// SLTutor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SLTutor.h"
#include "..\\StenoLite\\texthelpers.h"
#include "..\\StenoLite\\stenodata.h"
#include <Commdlg.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HWND txtdisplay;
HWND txtin;
HWND hint;
HANDLE hfile = NULL;
std::string cline("");

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SLTUTOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}


bool isReturn(char value) {
	return value == '\r' || value == '\n';
}

std::string spaces("");

void readWord() {
	DWORD bytes;
	char c;
	ReadFile(hfile, &c, 1, &bytes, NULL);
	
	while (bytes > 0 && (c == '\n' || c == '\r' || c == ' ' || c == '\t')) {
		spaces += c;
		ReadFile(hfile, &c, 1, &bytes, NULL);
	}

	tstring temp = strtotstr(spaces);

	int len = SendMessage(txtdisplay, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(txtdisplay, EM_SETSEL, len, len);
	SendMessage(txtdisplay, EM_REPLACESEL, FALSE, (LPARAM)(temp.c_str()));

	spaces.clear();

	bool first = true;
	
	while (bytes > 0 && (c != '\n' && c != '\r' && c != ' ' && c != '\t')) {
		if (c == ',' || c == '.' || c == ';' || c == ':' || c == '\"' || c == '\'' || c == '(' || c == ')' || c == '!' || c == '[' || c == ']' || c == '?') {
			if (first)
				cline += c;
			else
				SetFilePointer(hfile, -1, NULL, FILE_CURRENT);
			return;
		}
		cline += c;
		
		first = false;
		ReadFile(hfile, &c, 1, &bytes, NULL);
	}

	if (bytes != 0) {
		spaces += c;
	}
}

void appendCline() {
	tstring temp = strtotstr(cline);

	int len = SendMessage(txtdisplay, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(txtdisplay, EM_SETSEL, len, len);
	SendMessage(txtdisplay, EM_REPLACESEL, FALSE, (LPARAM)(temp.c_str()));

	SetWindowText(txtin, TEXT(""));
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SLTUTOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SLTUTOR);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   RECT rt;
   GetClientRect(hWnd, &rt);

   txtdisplay = CreateWindow(TEXT("EDIT"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_READONLY,
	   rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top - 30, hWnd, NULL, hInstance, NULL);

   txtin = CreateWindow(TEXT("EDIT"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | ES_LEFT,
	   rt.left, rt.bottom - rt.top - 30, (rt.right - rt.left) / 2, 30, hWnd, NULL, hInstance, NULL);

   hint = CreateWindow(TEXT("STATIC"), TEXT(""),
	   WS_CHILD | WS_VISIBLE,
	   rt.left + (rt.right - rt.left) / 2, rt.bottom - rt.top - 30, (rt.right - rt.left) / 2, 30, hWnd, NULL, hInstance, NULL);

   SendMessage(txtdisplay, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)true);
   SendMessage(txtin, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)true);
   SendMessage(hint, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)true);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   SetFocus(txtin);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;
	bool change = false;
	switch (message)
	{
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		return (LRESULT)GetStockObject(WHITE_BRUSH);
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		if (wmEvent == EN_CHANGE && (HWND)lParam == txtin && !change) {
			change = true;
			tstring txt = getWinStr(txtin);
			std::string trmd = trimstr(ttostr(txt), " ");
			if (trmd.compare(cline) == 0 && trmd.length() > 0) {
				cline.clear();
				readWord();
				appendCline();
				UINT_PTR id = 0;
				SetTimer(hWnd, id, 3000, NULL);
			}
			change = false;
		}
		switch (wmId)
		{
		case IDM_LOAD:
		{
						 OPENFILENAME file;
						 TCHAR buffer[MAX_PATH] = TEXT("\0");
						 memset(&file, 0, sizeof(OPENFILENAME));
						 file.lStructSize = sizeof(OPENFILENAME);
						 file.hwndOwner = hWnd;
						 file.lpstrFilter = TEXT("Text Files\0*.txt\0\0");
						 file.nFilterIndex = 1;
						 file.lpstrFile = buffer;
						 file.nMaxFile = MAX_PATH;
						 file.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
						 if (GetOpenFileName(&file)) {
							 //setMode(0);
							 if (hfile != NULL)
								 CloseHandle(hfile);

							 hfile = CreateFile(buffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
							
							 cline.clear();
							 DWORD bytes;

								 //detect and erase BOM
								 unsigned __int8 bom[3] = "\0\0";
								 ReadFile(hfile, bom, 3, &bytes, NULL);
								 if (bom[0] != 239 || bom[1] != 187 || bom[2] != 191) {
									 SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
								 }

								 readWord();
								 appendCline();
								 UINT_PTR id = 0;
								 SetTimer(hWnd, id, 5000, NULL);
								 SetFocus(txtin);
						 }
		}
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
	{
					 COPYDATASTRUCT MyCDS;
					 MyCDS.dwData = 2;            // function identifier


					 BYTE* buffer = new BYTE[sizeof(HWND)+sizeof(unsigned int)+cline.length() + 1];
					 memcpy(buffer, &hWnd, sizeof(HWND));
					 unsigned int retlen = cline.length() + 1;
					 memcpy(buffer + sizeof(HWND), &retlen, sizeof(unsigned int));
					 memcpy(buffer + sizeof(HWND)+sizeof(unsigned int), cline.c_str(), retlen);

					 MyCDS.cbData = sizeof(HWND)+sizeof(unsigned int)+cline.length() + 1; // size of data
					 MyCDS.lpData = buffer;           // data structure

					 HWND hwDispatch = FindWindow(TEXT("STENOLITE826"), NULL);
					 if (hwDispatch != NULL)
						 SendMessage(hwDispatch, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&MyCDS);
					 delete buffer;
	}
		return 0;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			return 0;
		}
		GetClientRect(hWnd, &rt);
		SetWindowPos(txtdisplay, NULL, 0, 0, rt.right - rt.left, rt.bottom - rt.top - 30, 0);
		SetWindowPos(txtin, NULL, 0, rt.bottom - rt.top - 30, (rt.right - rt.left) / 2, 30, 0);
		SetWindowPos(hint, NULL, (rt.right - rt.left) / 2, rt.bottom - rt.top - 30, (rt.right - rt.left) / 2, 30, 0);
		break;
	case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT)lParam;
			//MessageBox(NULL, TEXT("MSG"), TEXT("MSG"), MB_OK);

			if (pMyCDS->dwData == 1) {
				BYTE* ptr = (BYTE*)(pMyCDS->lpData);
				HWND* ret = (HWND*)(&ptr[0]);
				unsigned int* len = (unsigned int*)(&ptr[sizeof(HWND)]);
				unsigned __int8* stroke = (unsigned __int8*)(&ptr[sizeof(HWND)+sizeof(unsigned int)]);
				
				bool first = true;
				if (*len > 0) {
					tstring buffer;
					for (int i = 0; i < *len / 3; i++) {
						if (stroke[i * 3] == 0 && stroke[i * 3 + 1] == 0 && stroke[i * 3 + 2] == 0) {
							if (i + 1 < *len / 3) {
								buffer += TEXT(";  ");
							}
							first = true;
						}
						else {
							if (!first)
								buffer += TEXT('/');
							stroketocsteno(&(stroke[i * 3]), buffer, TEXT("#STKPWHRAO*EUFRPBLGTSDZ"));
							
						}
					}
					
				}
				return TRUE;
			}
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
