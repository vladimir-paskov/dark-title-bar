// ShellHook.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "ShellHook.h"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

using namespace winrt::Windows;
using namespace winrt::Windows::UI::ViewManagement;

extern UISettings gl_uiSettings;

BOOL                IsColorLight(UI::Color& clr) {
	return (((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128));
}

extern "C" {
	SHELLHOOK_API BOOL IsDarkMode() {
		auto foregroundColor = gl_uiSettings.GetColorValue(UIColorType::Foreground);
		return IsColorLight(foregroundColor);
	}

	SHELLHOOK_API VOID DwmUseImmersiveDarkModeIfNeeded(_In_ HWND hWnd) {
		if (!IsDarkMode()) return;
		if (!IsWindow(hWnd)) return;

		BOOL useDarkMode = TRUE;
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
	}

	SHELLHOOK_API LRESULT CALLBACK ShellProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam) {
		__pragma(message(__FUNCDNAME__));

		switch (nCode) {
			case HSHELL_WINDOWCREATED:
			case HSHELL_WINDOWACTIVATED: {		

				BOOL useDarkMode = IsDarkMode();
				HWND hWindow = (HWND)wParam;

				BOOL isAlreadyDark;
				HRESULT hResult = DwmGetWindowAttribute(hWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &isAlreadyDark, sizeof(isAlreadyDark));

				if (hResult == S_OK) {
					if (useDarkMode && isAlreadyDark) {
						break;
					}

					DwmSetWindowAttribute(hWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
				}

				break;
			}
		}

		return CallNextHookEx(0, nCode, wParam, lParam);
	}
}
