#include <windows.h>
#include <chrono>
#include "ScreenInterface.h"
#include "Chip8.h"
Chip8* g_chip8 = nullptr;

uint8_t MapKey(WPARAM vk)
{
    switch (vk)
    {
    case '1': return 0x1; case '2': return 0x2; case '3': return 0x3; case '4': return 0xC;
    case 'Q': return 0x4; case 'W': return 0x5; case 'E': return 0x6; case 'R': return 0xD;
    case 'A': return 0x7; case 'S': return 0x8; case 'D': return 0x9; case 'F': return 0xE;
    case 'Z': return 0xA; case 'X': return 0x0; case 'C': return 0xB; case 'V': return 0xF;
    default: return 0xFF; // invalid
    }
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    if (msg == WM_KEYDOWN || msg == WM_KEYUP)
    {
        uint8_t k = MapKey(wParam);
        if (k != 0xFF && g_chip8)
            g_chip8->SetKey(k, msg == WM_KEYDOWN);
    }
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Chip8WindowClass";
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        0, L"Chip8WindowClass", L"Chip8 DirectX V0.01",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 320,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, SW_SHOW);

    ScreenInterface renderer;
    if (!renderer.Init(hwnd, 640, 320))
    {
        MessageBox(hwnd, L"Failed to init D3D11", L"Error", MB_OK);
        return -1;
    }

    Chip8 inst;
    g_chip8 = &inst;
    using clock = std::chrono::steady_clock;
    auto lastCycle = clock::now();
    auto lastTimer = clock::now();
    const auto cycleInterval = std::chrono::microseconds(1000000 / 600); // ~600Hz
    const auto timerInterval = std::chrono::microseconds(1000000 / 60);  // 60Hz
    MSG msg{};
    if (!inst.getRom())
        return 3;
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        auto now = clock::now();

        while (now - lastCycle >= cycleInterval)
        {
            inst.mainCycle();
            lastCycle += cycleInterval;
        }

        if (now - lastTimer >= timerInterval)
        {
            inst.setTimer();
            lastTimer = now;
        }
        renderer.Render(inst.GetFrame(), 64, 32);
        renderer.Present();
        
    }

    return 0;
}