#pragma once

#ifdef KEYBOARD_HOOK_AGENT_PROJECT
    #define DLLAPI __declspec(dllexport)
#else
    #define DLLAPI __declspec(dllimport)
#endif

DLLAPI
void InstallKeyboardHookAgent();

DLLAPI
void UninstalKeyboardHookAgent();
