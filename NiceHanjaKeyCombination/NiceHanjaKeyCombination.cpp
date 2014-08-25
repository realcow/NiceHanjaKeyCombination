﻿// NiceHanjaKeyCombination.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "NiceHanjaKeyCombination.h"
#include "KeyDispatcher.h"
#include "NHKCMessageDefines.h"
#include <KeyboardHookAgent/GlobalKeyHook/GlobalKeyHook.h>
#include <JinLibrary/jin/win/StartupRegister.h>
#include <JinLibrary/jin/win/PathPicker.h>

#include <shellapi.h>

#include <string>
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
namespace Global
{
    HINSTANCE hInst;                                // current instance
    HWND hwnd;                                      // 현재 윈도우. 현재는 윈도우가 하나 뿐
}

TCHAR szTitle[MAX_LOADSTRING];                    // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

     // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_NICEHANJAKEYCOMBINATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NICEHANJAKEYCOMBINATION));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NICEHANJAKEYCOMBINATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDC_NICEHANJAKEYCOMBINATION);
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   Global::hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void CreateTrayIcon(HWND hWnd);
void DestroyTrayIcon(HWND hWnd);
void DispatchStartupRelatedCommands(int command);

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND    - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY    - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_CREATE:
        InstallKeyboardHookAgent(hWnd);
        CreateTrayIcon(hWnd);
        Global::hwnd = hWnd;
        break;

    case NHKCMessages::WM_SHORTCUT_KEY_PRESSED:
        KeyDispatcher::onKeyPress(wParam);
        break;

    case NHKCMessages::WM_TRAY_ICON_CLICKED:
        switch (LOWORD(lParam))
        {
        case WM_CONTEXTMENU:
            {
                HMENU menu = ::LoadMenu(Global::hInst, MAKEINTRESOURCE(IDC_NICEHANJAKEYCOMBINATION));

                ::SetForegroundWindow(Global::hwnd); // 이거하지 않으면 메뉴가 사라지지 않는다.  
                ::TrackPopupMenu(::GetSubMenu(menu, 0), // GetSubMenu이어야 한다. LoadMenu로 얻는 메뉴는 안된다.  
                                 TPM_LEFTALIGN | TPM_LEFTBUTTON, 
                                 LOWORD(wParam), HIWORD(wParam), 0,
                                 Global::hwnd, nullptr);
                ::DestroyMenu(menu);
            }
            break;
        }
        break;

    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(Global::hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_REGISTER_TO_STARTUP:
        case IDM_DEREGISTER_FROM_STARTUP:
            DispatchStartupRelatedCommands(wmId);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        DestroyTrayIcon(hWnd);
        UninstalKeyboardHookAgent();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void DispatchStartupRelatedCommands(int command)
{
    OSVERSIONINFO winVersion;
    ZeroMemory(&winVersion, sizeof winVersion);
    winVersion.dwOSVersionInfoSize = sizeof winVersion;
    ::GetVersionEx(&winVersion);
    
    bool isAboveXP = false;
    isAboveXP = (winVersion.dwMajorVersion >= 6);

    const wstring kTaskName = L"NiceHanjaKeyCombination";
    switch (command)
    {
    case IDM_REGISTER_TO_STARTUP:
        {
            wstring thisExecPath = jin::win::PathPicker::GetModuleFileNameW(0);
            if (isAboveXP)
            {
                jin::win::StartupRegister::registerToTaskScheduler(kTaskName, thisExecPath);
            }
            else
            {
                jin::win::StartupRegister::registerToRunRegistry(kTaskName, thisExecPath);
            }
        }
        break;

    case IDM_DEREGISTER_FROM_STARTUP:
        if (isAboveXP)
        {
            jin::win::StartupRegister::deregisterFromTaskScheduler(kTaskName);
        }
        else
        {
            jin::win::StartupRegister::deregisterFromRunRegistry(kTaskName);
        }
        break;

    default:
        break;
    }
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CreateTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hWnd;
    nid.uID = 100;

    nid.uFlags |= NIF_ICON;
    nid.hIcon = (HICON)::LoadIcon(Global::hInst, MAKEINTRESOURCE(IDI_NICEHANJAKEYCOMBINATION));

    nid.uFlags |= NIF_TIP;
    wcscpy(nid.szTip, L"NHKCTray");

    nid.uFlags |= NIF_MESSAGE;
    nid.uCallbackMessage = NHKCMessages::WM_TRAY_ICON_CLICKED;

    BOOL ret = ::Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    ret = ::Shell_NotifyIcon(NIM_SETVERSION, &nid); // 사용할 버전을 알려줘야 한다. 근데 없어도 동작하긴 한다.  
}

void DestroyTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hWnd;
    nid.uFlags = 0;
    nid.uID = 100;
    BOOL ret = ::Shell_NotifyIcon(NIM_DELETE, &nid);
}