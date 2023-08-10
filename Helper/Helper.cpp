// Helper.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Helper.h"
#include "ShellHook.h"

#define WND_CLASS_NAME TEXT("DarkTitleBarHookHelper")

#define HOOK_DLL { PLATFORM == 64 ? TEXT("ShellHook64.dll") : TEXT("ShellHook32.dll") }

// Global Variables:
HINSTANCE hInst;                                // current instance

HINSTANCE gl_hShellHookDll;
HHOOK gl_shellHook;

// Forward declarations of functions included in this code module:
ATOM                RegisterHelperClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    RegisterHelperClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: RegisterHelperClass()
//
//  PURPOSE: Registers the window class.
//
ATOM RegisterHelperClass(HINSTANCE hInstance)
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
   hInst = hInstance; // Store instance handle in our global variable

   gl_hShellHookDll = LoadLibrary(HOOK_DLL);
   if (gl_hShellHookDll == NULL) {
       return FALSE;
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
    switch (message) {
        case WM_CREATE: {
            // Register a WH_SHELL hook
            gl_shellHook = SetWindowsHookEx(WH_SHELL, (HOOKPROC)ShellProc, gl_hShellHookDll, 0);
            break;
        }
        case WM_DESTROY: {
            if (gl_shellHook != NULL) {
                UnhookWindowsHookEx(gl_shellHook);
            }

            if (gl_hShellHookDll != NULL) {
                FreeLibrary(gl_hShellHookDll);
            }

            PostQuitMessage(0);
            break;
        }
        default: {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }        
    }

    return 0;
}
