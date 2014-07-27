#include <cstdio>
#include <crtdbg.h>
#include <windows.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <Utils/WindowActivator/ProcessActivator.h>

using namespace std;

HINSTANCE hDll = 0;
HHOOK hKeyHook;
ULONGLONG cooldownInTick = 0;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hDll = hModule;
        _RPT1(_CRT_WARN, "[NHKC] Hook Dll is attached to the process %u\n", ::GetCurrentProcessId());
        break;

    case DLL_PROCESS_DETACH:
        _RPT1(_CRT_WARN, "[NHKC] Hook Dll is dettached from the process %u\n", ::GetCurrentProcessId());
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

LRESULT CALLBACK KeyHookProc(int code, WPARAM wParam, LPARAM lParam);

__declspec(dllexport)
void InstallKeyboardHookAgent()
{
    hKeyHook = ::SetWindowsHookExW(WH_KEYBOARD, KeyHookProc, hDll, 0);
}

__declspec(dllexport)
void UninstalKeyboardHookAgent()
{
    ::UnhookWindowsHookEx(hKeyHook);
}

struct KeyHandlerEntry
{
    WORD key;
    function<void(WORD /*key*/)> handler;
};

void OnArrowKey(WORD key);
void OnHomeKey(WORD key);

vector<KeyHandlerEntry> keyHandlers = 
    {
        { VK_LEFT, OnArrowKey },
        { VK_UP, OnArrowKey },
        { VK_DOWN, OnArrowKey },
        { VK_RIGHT, OnArrowKey },

        { VK_HOME, OnHomeKey },
    };

LRESULT CALLBACK KeyHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    do 
    {
        if (code != HC_ACTION) { break; }

        if ((::GetKeyState(VK_HANJA) & 0x8000) == 0) { break; }
        
        auto it = find_if(keyHandlers.cbegin(), keyHandlers.cend(), [&](const KeyHandlerEntry& entry) {
            return entry.key == wParam;
        });
        if (it == keyHandlers.cend()) { break; }

        DWORD64 kCooldownInTick = 50;
        auto now = ::GetTickCount64();
        if (now - cooldownInTick < kCooldownInTick) { break; }
        cooldownInTick = now;

        it->handler(wParam);
    }
    while (0);

    return ::CallNextHookEx(hKeyHook, code, wParam, lParam);
}

void OnArrowKey(WORD key)
{
    const int kArrowKeyRepeatCount = 2;
    for (int i = 0; i < kArrowKeyRepeatCount; i++)
    {
        ::keybd_event(static_cast<BYTE>(key), 0, 0, 0);
        ::keybd_event(static_cast<BYTE>(key), 0, KEYEVENTF_KEYUP, 0);
    }
}

void OnHomeKey(WORD key)
{
    const WCHAR kProcessName[] = L"WDExpress.exe";

    ActivateProcess(kProcessName);
}