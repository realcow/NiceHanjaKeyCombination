#pragma once

#include "../KeyboardHookAgentDefines.h"
#include <Windows.h>

KEYBOARD_HOOK_AGENT_API
void InstallKeyboardHookAgent(HWND _nhkcHwnd);

KEYBOARD_HOOK_AGENT_API
void UninstalKeyboardHookAgent();