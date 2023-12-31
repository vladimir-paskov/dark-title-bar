// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SHELLHOOK_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SHELLHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SHELLHOOK_EXPORTS
#define SHELLHOOK_API __declspec(dllexport)
#else
#define SHELLHOOK_API __declspec(dllimport)
#endif

extern "C" {
	SHELLHOOK_API BOOL IsDarkMode();
	SHELLHOOK_API VOID DwmUseImmersiveDarkModeIfNeeded(_In_ HWND hWnd);
	SHELLHOOK_API LRESULT CALLBACK ShellProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
}