#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <dwmapi.h>

// WinRT

// https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/issues/47#issuecomment-668381574
#include "winrt/base.h"
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.ViewManagement.h>