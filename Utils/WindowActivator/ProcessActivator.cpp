#include "pch.h"
#include "ProcessActivator.h"
#include <tchar.h>
#include <tlhelp32.h>

using namespace std;

namespace
{
    BOOL FindProcessByImageName(const wstring& imageFilename);
    HWND FindWindowHandleFromPid(DWORD pid);
    bool SetForeground(HWND window);
}

/*
    참고링크: Taking a Snapshot and Viewing Processes(http://msdn.microsoft.com/ko-kr/library/windows/desktop/ms686701(v=vs.85).aspx)
*/
bool ActivateProcess(const wstring& imageFilename)
{
    // 찾는 이미지 이름을 가진 프로세스를 검색
    DWORD pidToActivate = FindProcessByImageName(imageFilename);
    if (pidToActivate == (DWORD)-1)
    {
        return false;
    }

    // 프로세스가 가진 윈도우 중 Activate할 윈도우를 찾음.
    HWND hwndToActivate = FindWindowHandleFromPid(pidToActivate);
    if (!hwndToActivate)
    {
        return false;
    }

    // Activate
    SetForeground(hwndToActivate);

    // 타겟 윈도우 타이틀 출력; 디버깅용
    if (0)
    {
        WCHAR buff[100];
        ::GetWindowText(hwndToActivate, buff, 100);
        printf("%S", buff);
    }

    return true;
}

namespace
{
    BOOL FindProcessByImageName(const wstring& imageFilename)
    {
        HANDLE hProcessSnap;
        PROCESSENTRY32 pe32;
        DWORD foundPid = (DWORD)-1;

        // Take a snapshot of all processes in the system.
        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return (DWORD)-1;
        }

        // Set the size of the structure before using it.
        pe32.dwSize = sizeof(PROCESSENTRY32);

        // Retrieve information about the first process,
        // and exit if unsuccessful
        if (!Process32First(hProcessSnap, &pe32))
        {
            CloseHandle(hProcessSnap);          // clean the snapshot object
            return (DWORD)-1;
        }

        // Now walk the snapshot of processes, and
        // display information about each process in turn
        do
        {
            if (_wcsicmp(pe32.szExeFile, imageFilename.c_str()) == 0)
            {
                foundPid = pe32.th32ProcessID;
                break;
            }

        } while (Process32Next(hProcessSnap, &pe32));

        CloseHandle(hProcessSnap);
        return foundPid;
    }
    
    HWND GetTheHighestAncestorWindow(HWND h)
    {
        while (1)
        {
            HWND parentHwnd = ::GetParent(h);
            if (parentHwnd)
            {
                h = parentHwnd;
            }
            else
            {
                break;
            }
        }
        return h;
    }

    HWND FindWindowHandleFromPid(DWORD ownerProcessId)
    {
        HWND h = ::GetTopWindow(0);
        while (h)
        {
            DWORD pid;
            DWORD dwTheardId = ::GetWindowThreadProcessId(h, &pid);

            if (pid == ownerProcessId)
            {
                HWND ancestor = GetTheHighestAncestorWindow(h);
                if (!!ancestor && ::IsWindowVisible(ancestor) == TRUE)
                {
                    
                    h = ancestor;
                    break;
                }
            }
            h = ::GetNextWindow(h, GW_HWNDNEXT);
        }
        return h;
    }

    bool SetForeground(HWND window) 
    {
        ::SetForegroundWindow(window);
        return true;
    }
}
