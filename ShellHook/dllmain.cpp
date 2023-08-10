// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"

using namespace winrt::Windows;
using namespace winrt::Windows::UI::ViewManagement;

UISettings gl_uiSettings;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH: {
            gl_uiSettings = UISettings();
            break;
        }
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH: {
            break;
        }
    }

    return TRUE;
}

