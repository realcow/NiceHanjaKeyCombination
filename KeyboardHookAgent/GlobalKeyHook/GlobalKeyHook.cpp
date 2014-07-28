#include <windows.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <Utils/WindowActivator/ProcessActivator.h>
#include <crtdbg.h>

using namespace std;

extern HINSTANCE hDll;
HHOOK hKeyHook;
ULONGLONG cooldownInTick = 0;

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

vector<KeyHandlerEntry> keyHandlers =
{
    { VK_LEFT, OnArrowKey },
    { VK_UP, OnArrowKey },
    { VK_DOWN, OnArrowKey },
    { VK_RIGHT, OnArrowKey },

    { VK_HOME, bind([](WORD key, const wstring& s){ ActivateProcess(s); }, std::placeholders::_1, L"devenv.exe") },
    //{ VK_HOME, bind([](WORD key, const wstring& s){ ActivateProcess(s); }, std::placeholders::_1, L"WDExpress.exe") },
    { VK_END, bind([](WORD key, const wstring& s){ ActivateProcess(s); }, std::placeholders::_1, L"notepad++.exe") },
};

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
        DWORD64 kHanjaKeyLastingTime = 100;
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

        auto it = find_if(keyHandlers.cbegin(), keyHandlers.cend(), [&](const KeyHandlerEntry& entry) {
            return entry.key == wParam;
        });
        if (it == keyHandlers.cend()) { break; }

        DWORD64 kCooldownInTick = 50;
        auto now = ::GetTickCount64();
        if (now - cooldownInTick < kCooldownInTick) { break; }
        cooldownInTick = now;

        it->handler(wParam);
    } while (0);

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