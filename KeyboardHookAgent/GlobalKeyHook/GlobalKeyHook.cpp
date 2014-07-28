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
// VK_HANJA ��ŷ�� keydown�� �߻��ϴµ� keyup�� �߻����� ����. (WH_KEYBOARD, WH_KEYBOARD_LL)
// IME�� ������ �ִ� �� ������ �̰� ������ ���� ������ Ű ������ ����Ű�� �����ϰ� �ִ��� �� �� ���� ����.
// ���� �ذ��� ���� ���ؾ��� �׼�
//  - IME ������ �����ؼ� �ذ���� ã�ƺ���
//  - DirectInput�� �ẻ��?
//  - �̰͵� ���͵� �� �ȵǸ� Ű���� ���� ����̹��� ����.
// �׷��� ����� ������ ����Ű�� ���� �ð����κ��� �����ð� ���� ����Ű�� ���ȴٰ� �����ϴ� ������� �ϰ� ����. �� ������δ� home, end ����Ű�� �밭 �۵��ϴ� ���� ��
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