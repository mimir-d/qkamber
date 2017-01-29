
#include "stdafx.h"
#include "renderer.h"
#include "win32_window.h"
#include "win32_render_device.h"
using namespace std;
using namespace Gdiplus;

#define WINDOW_TITLE "my little renderererererer"
#define WINDOW_CLASS "win32app"

void Win32Window::init(unique_ptr<Renderer> renderer)
{
    flog();
    Window::init(std::move(renderer));

    WNDCLASSEX wcex;
    const HINSTANCE hInstance = GetModuleHandle(nullptr);

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = WINDOW_CLASS;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
        throw exception("Call to RegisterClassEx failed!");
    dlog("Registered class: %s", WINDOW_CLASS);

    // The parameters to CreateWindow explained:
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application dows not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    m_window_handle = CreateWindow(
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!m_window_handle)
        throw exception("Call to CreateWindow failed!");
    dlog("Created window %#x: %s", m_window_handle, WINDOW_TITLE);

    // init win32 specific render device
    static_cast<Win32RenderDevice*>(m_dev.get())->win32_init(m_window_handle);

    // save this pointer for wndproc
    SetWindowLong(m_window_handle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(m_window_handle, SW_SHOW);
    UpdateWindow(m_window_handle);

    m_paused = false;
}

void Win32Window::mainloop()
{
    flog();

    while (true)
    {
        while (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&m_msg);
            DispatchMessage(&m_msg);
        }
        if (m_msg.message == WM_QUIT)
            break;

        if (m_paused)
        {
            Sleep(10);
            continue;
        }
        on_paint();
    }
}

int Win32Window::shutdown()
{
    flog();


    Window::shutdown();
    return static_cast<int>(m_msg.wParam);
}

void Win32Window::on_key_pressed(int key_code)
{
    dlog("Pressed %d", key_code);
    switch (key_code)
    {
    case VK_ESCAPE:
        PostMessage(m_window_handle, WM_CLOSE, 0, 0);
        break;
    }
}

void Win32Window::on_paint()
{
    m_renderer->begin_frame();
    m_renderer->on_render();
    m_renderer->end_frame();
}

LRESULT CALLBACK Win32Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLong(hWnd, GWL_USERDATA));

    switch (message)
    {
        case WM_ACTIVATEAPP:
            window->m_paused = !wParam;
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            window->on_paint();
            EndPaint(hWnd, &ps);
            break;

        case WM_ERASEBKGND:
            return true;

        case WM_KEYDOWN:
            window->on_key_pressed(wParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}