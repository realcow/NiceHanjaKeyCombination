#include "GlobalKeyHook.h"
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
        ::PostMessage(nhkcHwnd, WM_USER + 1423, wParam, lParam);
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
