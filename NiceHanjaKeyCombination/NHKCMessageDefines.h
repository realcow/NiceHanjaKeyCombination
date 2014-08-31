#pragma once

#include <wtypes.h>
#include <string>
#include <cstdint>

namespace NHKCMessages
{
    const DWORD WM_TRAY_ICON_CLICKED = WM_USER + 2931;
    const DWORD WM_SHORTCUT_KEY_PRESSED = WM_USER + 1423;

    const std::wstring kNhkcPipeName = L"\\\\.\\pipe\\JINWOO_PIPE";

    struct KeyHookMessage
    {
        static const std::uint32_t kSignature = 0xf1b11CC;
        std::uint32_t signature = kSignature;
        WORD key;
    };
}