 #include "Hooks.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"


#include "Interfaces.h"
#include "Memory.h"
#include "GUI.h"


#include <atomic>
#include <intrin.h>

#include "Hacker/ESP.h"
#include "Hacker/Aimbot.h"
#include "Config.h"


class HookGuard {
public:
    HookGuard() { ++atomic; }
    ~HookGuard() { --atomic; }
    static bool freed() { return atomic == 0; }
private:
    inline static std::atomic_int atomic;
};


static DWORD WINAPI waitOnUnload(HMODULE hModule) noexcept
{
    while (!HookGuard::freed())
        Sleep(50);

    interfaces->inputSystem->enableInput(true);
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    hooks.reset();
    memory.reset();
    interfaces.reset();
    gui.reset();
    

    FreeLibraryAndExitThread(hModule, 0);
}

static LRESULT WINAPI wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    HookGuard guard;

    LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);
    interfaces->inputSystem->enableInput(!gui->open);

    if (config->espEnable)
        ESP::collectMessages();

    return CallWindowProc(hooks->wndProc, window, msg, wParam, lParam);
}

static HRESULT D3DAPI present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
    HookGuard guard;

    static auto _ = ImGui_ImplDX9_Init(device);

    IDirect3DVertexDeclaration9* vertexDeclaration;
    device->GetVertexDeclaration(&vertexDeclaration);

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //TODO DRAW BEGIN
    if (interfaces->engine->isInGame()) {
        ESP::render(ImGui::GetBackgroundDrawList());

        Aimbot::run(ImGui::GetBackgroundDrawList());
    }
    //TODO DRAW END.

    //Draw Menu
    gui->render();

    if (ImGui::IsKeyPressed(VK_INSERT, false))
        gui->open = !gui->open;

    ImGui::GetIO().MouseDrawCursor = gui->open;

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    device->SetVertexDeclaration(vertexDeclaration);
    vertexDeclaration->Release();

    return hooks->present(device, src, dest, windowOverride, dirtyRegion);
}

static HRESULT D3DAPI reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    HookGuard guard;

    ImGui_ImplDX9_InvalidateDeviceObjects();
    auto result = hooks->reset(device, params);
    ImGui_ImplDX9_CreateDeviceObjects();

    return result;
}

static BOOL WINAPI setCursorPos(int X, int Y) noexcept
{
    HookGuard guard;

    return gui->open || hooks->setCursorPos(X, Y);
}

Hooks::Hooks(HMODULE module) noexcept
{

    assert(memory);

    this->module = module;

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    wndProc = WNDPROC(SetWindowLongPtrA(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(::wndProc)));


    //hook present()函数
    present = **reinterpret_cast<decltype(present)**>(memory->present);
    **reinterpret_cast<decltype(::present)***>(memory->present) = ::present;

    //hook reset()函数
    reset = **reinterpret_cast<decltype(reset)**>(memory->reset);
    **reinterpret_cast<decltype(::reset)***>(memory->reset) = ::reset;

    //hook setCursorPos()函数
    setCursorPos = *reinterpret_cast<decltype(setCursorPos)*>(memory->setCursorPos);
    *reinterpret_cast<decltype(::setCursorPos)**>(memory->setCursorPos) = ::setCursorPos;
}

void Hooks::restore() noexcept
{
    //恢复原来的样子
    **reinterpret_cast<void***>(memory->present) = present;
    **reinterpret_cast<void***>(memory->reset) = reset;
    *reinterpret_cast<void**>(memory->setCursorPos) = setCursorPos;

    SetWindowLongPtrA(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(wndProc));

    if (HANDLE thread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(waitOnUnload), module, 0, nullptr))
        CloseHandle(thread);
}
