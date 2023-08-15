// DarkTitleBar.cpp : Defines the entry point for the application.
//

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "framework.h"
#include "DarkTitleBar.h"
#include "ShellHook.h"

#define WND_CLASS_NAME TEXT("DarkTitleBar")
#define WM_TRAY_MSG ( WM_USER + 1000 )

#define DEBUG_PRINT(...) {TCHAR cad[512]; wsprintf(cad, __VA_ARGS__);  OutputDebugString(cad);}

#define HELPER32 TEXT("Helper32.exe")
#define HELPER64 TEXT("Helper64.exe")

// {B8161DE2-F837-4B5E-90FB-9BF8C1540CC9}
static const GUID uidTrayIcon =
{ 0xb8161de2, 0xf837, 0x4b5e, { 0x90, 0xfb, 0x9b, 0xf8, 0xc1, 0x54, 0xc, 0xc9 } };

enum class PreferredAppMode {
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
};

using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903

// Global Variables:
HINSTANCE gl_hInstance;                                // current instance
HMENU gl_hTrayMenu;

HANDLE gl_hHelper32;
HANDLE gl_hHelper64;
HMODULE gl_hUxTheme;

// Forward declarations of functions included in this code module:
ATOM                RegisterDarkTitleBarClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutProc(HWND, UINT, WPARAM, LPARAM);

LRESULT             TrayMsgHandler(HWND, WPARAM, LPARAM);
HANDLE				CreateHelper(_In_ LPCTSTR, _In_ LPCTSTR, _In_ DWORD);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitCommonControls();

	RegisterDarkTitleBarClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: RegisterDarkTitleBarClass()
//
//  PURPOSE: Registers the window class.
//
ATOM RegisterDarkTitleBarClass(HINSTANCE hInstance)
{
	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszClassName = WND_CLASS_NAME;

	return RegisterClassExW(&wndClass);
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
	gl_hInstance = hInstance; // Store instance handle in our global variable

	gl_hUxTheme = LoadLibraryEx(TEXT("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

	if (gl_hUxTheme != NULL) {
		fnSetPreferredAppMode SetPreferredAppMode;
		SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(gl_hUxTheme, MAKEINTRESOURCEA(135));
		SetPreferredAppMode(PreferredAppMode::AllowDark);
	}

	HWND hWnd = CreateWindow(WND_CLASS_NAME, WND_CLASS_NAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)NULL, hInstance, NULL);

	if (!hWnd) {
		return FALSE;
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			// Add application tray icon
			NOTIFYICONDATA trayNotifyIconData;
			::ZeroMemory(&trayNotifyIconData, sizeof(NOTIFYICONDATA));
			trayNotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
			trayNotifyIconData.hWnd = hWnd;
			trayNotifyIconData.uID = uidTrayIcon.Data1;
			trayNotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			trayNotifyIconData.uCallbackMessage = WM_TRAY_MSG;

			HICON hIcon =
				(HICON)LoadImage(gl_hInstance, MAKEINTRESOURCE(IDI_DARKTITLEBAR), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED);

			if (hIcon == NULL) {
				MessageBox(NULL, TEXT("Icon creation faild! Application will now terminate!"),
					TEXT("Error"), MB_ICONERROR | MB_OK);
				return -1;
			}

			trayNotifyIconData.hIcon = hIcon;
			_tcscpy_s(trayNotifyIconData.szTip, TEXT("DarkTitleBar"));

			trayNotifyIconData.uVersion = NOTIFYICON_VERSION;

			if (!Shell_NotifyIcon(NIM_ADD, &trayNotifyIconData)) {
				MessageBox(NULL, TEXT("Shell icon registration faild! Application will now terminate!"),
					TEXT("Error"), MB_ICONERROR | MB_OK);
				return -1;
			}

			DWORD dwCurrentDirSize = GetCurrentDirectory(0, NULL);
			TCHAR* pszCurrentDir = (TCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCurrentDirSize * sizeof(TCHAR));
			if (pszCurrentDir == NULL) {
				return -1;
			}

			GetCurrentDirectory(dwCurrentDirSize, pszCurrentDir);

			gl_hHelper32 = CreateHelper(HELPER32, pszCurrentDir, dwCurrentDirSize);
			if (gl_hHelper32 == NULL) {
				HeapFree(GetProcessHeap(), 0, pszCurrentDir);
				return -1;
			}

#ifdef _WIN64
			gl_hHelper64 = CreateHelper(HELPER64, pszCurrentDir, dwCurrentDirSize);
			if (gl_hHelper64 == NULL) {
				TerminateProcess(gl_hHelper32, 0);
				HeapFree(GetProcessHeap(), 0, pszCurrentDir);
				return -1;
			}
#endif				

			break;
		}
		case WM_TRAY_MSG:
		{
			return TrayMsgHandler(hWnd, wParam, lParam);
		}
		case WM_MENUCOMMAND:
		{
			HMENU hMenu = (HMENU)lParam;

			// Get information about the clicked menu item
			MENUITEMINFO menuItemInfo;
			ZeroMemory(&menuItemInfo, sizeof(MENUITEMINFO));
			menuItemInfo.cbSize = sizeof(menuItemInfo);
			menuItemInfo.fMask = MIIM_ID | MIIM_DATA;
			GetMenuItemInfo(hMenu, (UINT)wParam, TRUE, &menuItemInfo);

			switch (menuItemInfo.wID) {
				case IDM_ABOUT: {
					DialogBox(gl_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutProc);
					break;
				}
				case IDM_EXIT: {
					DestroyWindow(hWnd);
					break;
				}
			}

			break;
		}
		case WM_DESTROY:
		{
			if(gl_hHelper32 != NULL) {
				TerminateProcess(gl_hHelper32, 0);
			}

#ifdef _WIN64
			if (gl_hHelper64 != NULL) {
				TerminateProcess(gl_hHelper64, 0);
			}
#endif

			if (gl_hUxTheme != NULL) {
				FreeLibrary(gl_hUxTheme);
			}

			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

/*
 * Tray icon message handling.
 */
LRESULT TrayMsgHandler(HWND hParent, WPARAM wParam, LPARAM lParam)
{
	HMENU hTrayPopupMenu = NULL;
	if (lParam == WM_RBUTTONDOWN) {
		DestroyMenu(gl_hTrayMenu);
		gl_hTrayMenu = LoadMenu(gl_hInstance, MAKEINTRESOURCE(IDC_DARKTITLEBAR));

		hTrayPopupMenu = GetSubMenu(gl_hTrayMenu, 0);
		// Modify the menu style so the notifications pass trough WM_MENUCOMMAND
		MENUINFO menuInfo;
		ZeroMemory(&menuInfo, sizeof(MENUINFO));
		menuInfo.cbSize = sizeof(MENUINFO);
		menuInfo.fMask = MIM_STYLE;
		GetMenuInfo(hTrayPopupMenu, &menuInfo);
		menuInfo.dwStyle |= MNS_NOTIFYBYPOS;
		SetMenuInfo(hTrayPopupMenu, &menuInfo);

		int nMenuItemLen = 0;

		SetForegroundWindow(hParent); // (KB135788) 

		// Get mouse cordinates
		POINT ptMouse;
		GetCursorPos(&ptMouse);

		TrackPopupMenu(hTrayPopupMenu,
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			ptMouse.x, ptMouse.y, 0, hParent, NULL);

		PostMessage(hParent, WM_NULL, 0, 0); // (KB135788) 
	}

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG: {
			DwmUseImmersiveDarkModeIfNeeded(hDlg);
			return (INT_PTR)TRUE;
		}			

		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}			
	}

	return (INT_PTR)FALSE;
}

HANDLE CreateHelper(_In_ LPCTSTR lpszHelperName, _In_ LPCTSTR lpszCurrentDir, _In_ DWORD dwCurrentDirSize) {
	if (lpszHelperName == NULL || _tcslen(lpszHelperName) <= 0) {
		return NULL;
	}

	if (lpszCurrentDir == NULL || _tcslen(lpszCurrentDir) <= 0) {
		return NULL;
	}

	SIZE_T lHelperExecutable = (dwCurrentDirSize + _tcslen(HELPER64) * sizeof(TCHAR));

	TCHAR* pszHelperPath = (TCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lHelperExecutable * sizeof(TCHAR));
	if (pszHelperPath == NULL) {
		return NULL;
	}

	_stprintf_s(pszHelperPath, lHelperExecutable, TEXT("% s\\% s"), lpszCurrentDir, lpszHelperName);

	STARTUPINFO siHelper;
	PROCESS_INFORMATION piHelper;

	ZeroMemory(&siHelper, sizeof(siHelper));
	siHelper.cb = sizeof(siHelper);
	ZeroMemory(&piHelper, sizeof(piHelper));

	BOOL bResult = CreateProcess(pszHelperPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &siHelper, &piHelper);
	if (!bResult) {
		HeapFree(GetProcessHeap(), 0, pszHelperPath);
		DEBUG_PRINT(TEXT("CreateProcess failed (%d).\n"), GetLastError());
		return NULL;
	}

	return piHelper.hProcess;
}
