// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>


#include "GUI.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "Memory.h"
#include "Config.h"

static LRESULT WINAPI init(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

static WNDPROC originalWndproc;
static HMODULE module;

HINSTANCE g_hInstance = 0;
HANDLE g_hOutput = 0;
HWND hwnd = NULL;
HMENU hmenu = NULL;
CHAR title[] = "不要关闭本窗口！说的你好像真的能关掉一样！";
TCHAR c[];
HANDLE hdlWrite = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        
        //程序注入到CSGO进程中
    case DLL_PROCESS_ATTACH:

        module = hModule;
        originalWndproc = WNDPROC(SetWindowLongPtrA(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(init)));

        break;
    case DLL_PROCESS_DETACH:
        return TRUE;
    }
    return TRUE;
}


//循环
static LRESULT WINAPI init(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    SetWindowLongPtr(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(originalWndproc));

    config = std::make_unique<Config>();
    gui = std::make_unique<GUI>();
    interfaces = std::make_unique<const Interfaces>();
    memory = std::make_unique<Memory>();
    hooks = std::make_unique<Hooks>(module);
  

    //调用自己,达到循环目的。
    return CallWindowProc(originalWndproc, window, msg, wParam, lParam);
}