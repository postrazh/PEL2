// CsvExport.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <CommCtrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "framework.h"
#include "CsvExport.h"
#include "injector.h"
#include "SCL.hpp"
#include "Exp.h"

#define IPC_IMPLEMENTATION
#include "ipc.h"

using namespace std;
using namespace scl;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL				CenterWindow(HWND hwndWindow);

// variables
ipc_sharedmemory mem;

CInjector injector;

int gFlag = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CSVEXPORT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CSVEXPORT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	// clean up
	ipc_mem_close(&mem);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CSVEXPORT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CSVEXPORT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

HWND _hwndPELTree = NULL;

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam) {
	// Retrieve storage location for communication data
	DWORD dwPID = (DWORD)lParam;
	DWORD dwProcessId = 0x0;
	// Query process ID for hWnd
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	// Apply filter - if you want to implement additional restrictions,
	// this is the place to do so.
	if (dwPID == dwProcessId) {

		// ignore console window
		TCHAR szClassName[MAX_PATH];
		GetClassName(hWnd, szClassName, MAX_PATH);
		if (lstrcmp(szClassName, _T("ConsoleWindowClass")) == 0) {
			return TRUE;
		}

		// stop at the top level window
		if (GetWindow(hWnd, GW_OWNER) == 0 && IsWindowVisible(hWnd)) {
			// Found a window matching the process ID
			_hwndPELTree = hWnd;
			// Report success
			SetLastError(ERROR_SUCCESS);
			// Stop enumeration
			return FALSE;
		}		
	}
	// Continue enumeration
	return TRUE;
}

HWND FindWindowFromProcess(DWORD dwPID)
{
	if (!EnumWindows(EnumProc, (LPARAM)dwPID) && (GetLastError() == ERROR_SUCCESS)) {
		return _hwndPELTree;
	}
	
	return NULL;
}
inline bool is_exists(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

void ReadConfig()
{
	const char* confie_file_name = "PEL.ini";
	if (!is_exists(confie_file_name)) {
		config_file config_writer(confie_file_name, config_file::WRITE);

		config_writer.put("interval_seconds", 5);
		config_writer.put("reset_table_minutes", 10);
		config_writer.put("csv_path", "C:\\out.csv");
		config_writer.put("ftp_server", "192.168.2.119");
		config_writer.put("ftp_user", "user1");
		config_writer.put("ftp_password", "password1");
		config_writer.put("ftp_local_file", "c:\\out.csv");
		config_writer.put("ftp_remote_file", "/PEL_uploaded.csv");

		//write changes
		config_writer.write_changes();
		//close the file
		config_writer.close();
	}

	config_file config(confie_file_name, config_file::READ);

	// create IPC mem
	ipc_mem_init(&mem, (char*)"ipc_PEL_memory", 1024);

	if (ipc_mem_open_existing(&mem))
	{
		if (ipc_mem_create(&mem))
		{
			return;
		}
		memset(mem.data, 0, mem.size);
		int *p = (int*)mem.data;

		// write the interval_seconds
		int interval_seconds = config.get<int>("interval_seconds");
		*p = interval_seconds;
		p++;

		// write the reset_table_minutes
		int reset_table_minutes = config.get<int>("reset_table_minutes");
		*p = reset_table_minutes;
		p++;

		// write the flag
		*p = gFlag;
		p++;

		CHAR* q = (CHAR*)p;
		string value;

		// write the csv_path
		value = config.get<string>("csv_path");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;

		// write the ftp_server
		value = config.get<string>("ftp_server");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;

		// write the ftp_user
		value = config.get<string>("ftp_user");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;

		// write the ftp_password
		value = config.get<string>("ftp_password");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;

		// write the ftp_local_file
		value = config.get<string>("ftp_local_file");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;

		// write the ftp_remote_file
		value = config.get<string>("ftp_remote_file");
		lstrcpynA(q, value.c_str(), MAX_PATH);
		q += MAX_PATH;
	}

	//close the file
	config.close();
}

BOOL initPEL()
{
	// Load config
	ReadConfig();

	// Inject
	LPCWSTR dllName = _T("CsvDll.dll");
	LPCWSTR procPath = _T("C:\\Program Files (x86)\\DataView\\PEL.exe");

	DWORD dwPID;
	if (injector.InjectAuto(dllName, procPath, &dwPID) < 0) {
		return FALSE;
	}

	HWND hwndPEL = NULL;
	// loop till the PEL get started
	for (int i = 0; i < 20; i++) {
		// find PEL window
		hwndPEL = FindWindowFromProcess(dwPID);
		if (hwndPEL != NULL)
			break;

		Sleep(1000);
	}

	// find treeview
	if (hwndPEL == NULL) {
		MessageBox(NULL, _T("Can not find the PEL window"), _T("Error"), MB_OK);
		return FALSE;
	}

	HWND hwndMdiFrame = FindWindowEx(hwndPEL, NULL, _T("AfxMDIFrame140u"), NULL);
	HWND hwndTreeview = FindWindowEx(hwndMdiFrame, NULL, _T("SysTreeView32"), NULL);

	if (hwndTreeview == NULL) {
		MessageBox(NULL, _T("Can not find the treeview window"), _T("Error"), MB_OK);
		return FALSE;
	}

	// select treeview item
	HTREEITEM hRealTime = NULL;

	for (int i = 0; i < 20; i++) {
		HTREEITEM hRoot = TreeView_GetRoot(hwndTreeview);
		HTREEITEM  hChild = TreeView_GetNextItem(hwndTreeview, hRoot, TVGN_CHILD);
		if (hChild != NULL) {
			HTREEITEM  hChildChild = TreeView_GetNextItem(hwndTreeview, hChild, TVGN_CHILD);

			if (hChildChild != NULL) {
				HTREEITEM  hChildChildChild = TreeView_GetNextItem(hwndTreeview, hChildChild, TVGN_CHILD);

				if (hChildChildChild != NULL) {
					hRealTime = TreeView_GetNextItem(hwndTreeview, hChildChildChild, TVGN_NEXT);
					if (hRealTime != NULL) {
						TreeView_SelectItem(hwndTreeview, hRealTime);
						break;
					}
				}
			}
		}

		Sleep(1000);
	}

	if (hRealTime == NULL) {
		MessageBox(NULL, _T("Can not find the Real Time Status"), _T("Error"), MB_OK);
		return FALSE;
	}

	return TRUE;
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
	//
	CExpire exp(_T("PEL"), _T("LEFT2"), 100, TYPEDAYS);
	if (exp.HasExpired())
	{
		gFlag = 1;
	}

	//
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   CenterWindow(hWnd);
   UpdateWindow(hWnd);

   // init PEL
   if (!initPEL())
	   return FALSE;
   
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:				
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL CenterWindow(HWND hwndWindow)
{
	RECT rectWindow;

	GetWindowRect(hwndWindow, &rectWindow);

	int nWidth = rectWindow.right - rectWindow.left;
	int nHeight = rectWindow.bottom - rectWindow.top;
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	int nX = (nScreenWidth - nWidth) / 2;
	int nY = (nScreenHeight - nHeight) / 2;

	MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

	return TRUE;
}