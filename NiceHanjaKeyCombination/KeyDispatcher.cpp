#include "pch.h"
#include "KeyDispatcher.h"
#include <Utils/WindowActivator/ProcessActivator.h>

#include <functional>
#include <vector>
#include <algorithm>


using namespace std;
using namespace boost;

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

    { VK_HOME, ProcessActivator(L"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\Common7\\IDE\\devenv.exe") },
    { VK_END, ProcessActivator(L"C:\\Program Files (x86)\\Notepad++\\notepad++.exe") },
    { VK_NUMPAD0, ProcessActivator(L"C:\\Program Files (x86)\\Microsoft Office 15\\root\\office15\\POWERPNT.EXE") },
    { VK_NUMPAD1, ProcessActivator(L"C:\\Programs\\WorkUtil\\DbgView\\DebugView 4.79\\Dbgview.exe") },
};

void OnArrowKey(WORD key)
{
    const int kArrowKeyRepeatCount = 2;
    for (int i = 0; i < kArrowKeyRepeatCount; i++)
    {
        ::keybd_event(static_cast<BYTE>(key), 0, 0, 0);
        ::keybd_event(static_cast<BYTE>(key), 0, KEYEVENTF_KEYUP, 0);
    }
}

void KeyDispatcher::onKeyPress(WPARAM key)
{
    auto it = find_if(keyHandlers.cbegin(), keyHandlers.cend(), [&](const KeyHandlerEntry& entry) {
        return entry.key == key;
    });
    if (it == keyHandlers.cend()) { return; }
    it->handler(key);
}
