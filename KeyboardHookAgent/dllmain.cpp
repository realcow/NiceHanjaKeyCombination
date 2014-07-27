#include <windows.h>
#include <cstdio>
#include <crtdbg.h>

HINSTANCE hDll = 0;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hDll = hModule;
        //_RPT1(_CRT_WARN, "[NHKC] Hook Dll is attached to the process %u\n", ::GetCurrentProcessId());
        break;

    case DLL_PROCESS_DETACH:
        //_RPT1(_CRT_WARN, "[NHKC] Hook Dll is dettached from the process %u\n", ::GetCurrentProcessId());
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}