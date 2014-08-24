#include "pch.h"
#include "ProcessActivator.h"
#include <jin/win/Process.h>
#include <tchar.h>
#include <tlhelp32.h>


using namespace std;

namespace
{
    BOOL FindProcessByImageName(const wstring& imageFilename);
    HWND FindWindowHandleFromPid(DWORD pid);
    bool SetForeground(HWND window);
    bool ActivateProcess(const wstring& imageFilename);
}

/*
    파일이름으로 프로세스를 찾아 그 프로세스가 가진 윈도우를 프론트로 내세운다
    참고링크: Taking a Snapshot and Viewing Processes(http://msdn.microsoft.com/ko-kr/library/windows/desktop/ms686701(v=vs.85).aspx)
*/
void ProcessActivator::operator()(WORD key)
{
    // 찾는 이미지 이름을 가진 프로세스를 검색
    DWORD pidToActivate = FindProcessByImageName(processImageName);
    if (pidToActivate == (DWORD)-1)
    {
        jin::win::Process process;
        process.StartInfo.FileName = processPath.wstring();
        process.Start();
        return;
    }

    // 프로세스가 가진 윈도우 중 Activate할 윈도우를 찾음.
    HWND hwndToActivate = FindWindowHandleFromPid(pidToActivate);
    if (!hwndToActivate)
    {
        return;
    }

    // 윈도우가 최소화 상태이면 복원하고 그렇지 않으면 'foreground' 윈도우로 만듦
    bool isMinimized = (::IsIconic(hwndToActivate) == TRUE);
    if (isMinimized)
    {
        _RPT0(_CRT_WARN, "That window is iconic");
        ::ShowWindow(hwndToActivate, SW_RESTORE);
    }
    else
    {
        // Activate
        SetForeground(hwndToActivate);
    }


    // 타겟 윈도우 타이틀 출력; 디버깅용
    if (0)
    {
        WCHAR buff[100];
        ::GetWindowText(hwndToActivate, buff, 100);
        printf("%S", buff);
    }
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

    void RunProcesAndWait()
    {


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

    // *TODO: 개선하자
    bool SetForeground(HWND window) 
    {
        // *NOTE: SearchExpress에서 가져옴. 언제 이걸 통합해야 하는데.
        // 그리고 미니마이즈 & 리스토어되는 애니매이션이 보인다.
        ::ShowWindow(window, SW_SHOWMINNOACTIVE); // 그나마 가장 잘 작동함. 그래도 작업관리자보다 앞에 튀어나오진 않음.
        ::ShowWindow(window, SW_RESTORE);

        // 이 방식은 현재 실행 중인 스레드가 포그라운드 윈도우 스레드이면 작동하는데, 그 외엔 그렇지 않더라.
        //::SetForegroundWindow(window); 
        return true;
    }
}