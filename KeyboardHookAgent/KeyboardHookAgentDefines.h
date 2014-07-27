#pragma once

#ifdef KEYBOARD_HOOK_AGENT_PROJECT
    #define KEYBOARD_HOOK_AGENT_API __declspec(dllexport)
#else
    #define KEYBOARD_HOOK_AGENT_API __declspec(dllimport)
#endif