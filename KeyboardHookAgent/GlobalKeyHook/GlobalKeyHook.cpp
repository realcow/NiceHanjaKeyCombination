#include "GlobalKeyHook.h"
#include <NiceHanjaKeyCombination\NHKCMessageDefines.h>
#include <windows.h>
#include <crtdbg.h>

using namespace std;

extern HINSTANCE hDll;
HHOOK hKeyHook;

#pragma data_seg(".npdata")
HWND nhkcHwnd = 0;
ULONGLONG cooldownInTick = 0;
#pragma data_seg()

#pragma comment (linker, "/SECTION:.npdata,RWS")

LRESULT CALLBACK KeyHookProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyHookProcForMonitoring(int code, WPARAM wParam, LPARAM lParam);

__declspec(dllexport)
void InstallKeyboardHookAgent(HWND _nhkcHwnd)
{
    bool monitorKeyboardHookEvent = false;
    auto keyHookProc = monitorKeyboardHookEvent ? KeyHookProcForMonitoring : KeyHookProc;
    hKeyHook = ::SetWindowsHookExW(WH_KEYBOARD, keyHookProc, hDll, 0);
    nhkcHwnd = _nhkcHwnd;
}

__declspec(dllexport)
void UninstalKeyboardHookAgent()
{
    ::UnhookWindowsHookEx(hKeyHook);
}

// *NOTE(realcow):
// VK_HANJA 후킹시 keydown은 발생하는데 keyup이 발생하지 않음. (WH_KEYBOARD, WH_KEYBOARD_LL)
// IME와 관련이 있는 것 같은데 이것 때문에 현재 눌려진 키 조합이 한자키를 포함하고 있는지 알 수 없는 상태.
// 문제 해결을 위해 취해야할 액션
//  - IME 동작을 조사해서 해결법을 찾아본다
//  - DirectInput을 써본다?
//  - 이것도 저것도 다 안되면 키보드 필터 드라이버를 쓴다.
// 그래서 현재는 마지막 한자키가 눌린 시간으로부터 일정시간 동안 한자키가 눌렸다고 간주하는 방식으로 하고 있음. 이 방식으로는 home, end 단축키만 대강 작동하는 상태 ㅠ
bool IsHanjaEnabled(WPARAM wParam, LPARAM lParam)
{
    static DWORD64 hanjaKeyLastPressedTime = 0;
    DWORD64 now = ::GetTickCount64();

    if (wParam == VK_HANJA)
    {
        _RPT0(_CRT_WARN, "hanja");
        hanjaKeyLastPressedTime = now;
        return true;
    }
    else
    {
        DWORD64 kHanjaKeyLastingTime = 300;
        if (now - hanjaKeyLastPressedTime < kHanjaKeyLastingTime)
        {
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK KeyHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    static bool hanjaPressed = false;
    do
    {
        if (code != HC_ACTION) { break; }
        
        if (!IsHanjaEnabled(wParam, lParam)) { break; }

        DWORD64 kCooldownInTick = 50;
        auto now = ::GetTickCount64();
        if (now - cooldownInTick < kCooldownInTick) { break; }
        cooldownInTick = now;

        _RPT1(_CRT_WARN, "hanja + %d key detected", wParam);
        ::PostMessage(nhkcHwnd, NHKCMessages::WM_SHORTCUT_KEY_PRESSED, wParam, lParam);
    } while (0);

    return ::CallNextHookEx(hKeyHook, code, wParam, lParam);
}

LRESULT CALLBACK KeyHookProcForMonitoring(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION) 
    { 
        _RPT2(_CRT_WARN, "wParam: %08X, lParam: %08X", wParam, lParam);
    }
    return ::CallNextHookEx(hKeyHook, code, wParam, lParam);
}
