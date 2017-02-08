
#include "stdafx.h"
#include "resource.h"
#include "app.h"
#include "win32_window.h"
#include "win32_software_device.h"
using namespace std;

constexpr char* WINDOW_TITLE = "my little renderererererer";
constexpr char* WINDOW_CLASS = "WIN32_RENDERER";

#pragma warning(disable:4302)

void Win32Window::init(Application* app, Timer* timer)
{
    flog();
    Window::init(app, timer);

    init_class();
    init_window();

    m_paused = false;
}

void Win32Window::mainloop()
{
    flog();
    MSG msg;

    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            m_exit_code = msg.wParam;
            break;
        }

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
    return m_exit_code;
}

void Win32Window::init_class()
{
    flog();
    HINSTANCE instance = GetModuleHandle(nullptr);

    WNDCLASSEX wcex = { 0 };

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.hInstance      = instance;
    wcex.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(IDI_RENDERER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName  = WINDOW_CLASS;
    wcex.hIconSm        = LoadIcon(instance, MAKEINTRESOURCE(IDI_RENDERER));

    wcex.lpfnWndProc    = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLong(hWnd, GWL_USERDATA));
        return window->wnd_proc(hWnd, message, wParam, lParam);
    };

    if (!RegisterClassEx(&wcex))
        throw exception("Call to RegisterClassEx failed!");

    dlog("Registered win32 class: %s", WINDOW_CLASS);
}

void Win32Window::init_window()
{
    flog();
    HINSTANCE instance = GetModuleHandle(nullptr);

    m_window_handle = CreateWindow(
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!m_window_handle)
        throw exception("Call to CreateWindow failed!");

    // save this pointer for wndproc
    SetWindowLong(m_window_handle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    // need to init here because of update window
    init_render_device();

    ShowWindow(m_window_handle, SW_SHOW);
    UpdateWindow(m_window_handle);
    m_window_state = SIZE_RESTORED;

    dlog("Created window %#x: %s", m_window_handle, WINDOW_TITLE);
}

void Win32Window::init_render_device()
{
    flog();

    // init win32 specific render device
    auto& dev = m_app->get_renderer().get_device();
    static_cast<Win32RenderDevice&>(dev).win32_init(m_window_handle);
}

void Win32Window::on_paint()
{
    float abs_time = m_timer->get_abs_time();
    float diff_time = m_timer->get_diff_time();

    m_app->update(abs_time, diff_time);
    m_app->render(abs_time, diff_time);
}

void Win32Window::on_resize()
{
    RECT rc;
    GetClientRect(m_window_handle, &rc);
    if (!EqualRect(&m_client_rect, &rc))
    {
        const LONG width = rc.right - rc.left;
        const LONG height = rc.bottom - rc.top;

        paused([&]()
        {
            auto& dev = m_app->get_renderer().get_device();
            static_cast<Win32RenderDevice&>(dev).win32_resize(&rc);

            m_app->on_resize(width, height);
        });

        CopyRect(&m_client_rect, &rc);
    }
}

LRESULT Win32Window::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
        case WM_ACTIVATEAPP:
            m_paused = !wParam;
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            on_paint();
            EndPaint(hWnd, &ps);
            break;

        case WM_ERASEBKGND:
            return true;

        case WM_KEYDOWN:
            // TODO: maybe should input this as configurable
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;

        case WM_SETCURSOR:
            // hide the cursor when inside the window
            if (LOWORD(lParam) == HTCLIENT)
            {
                SetCursor(nullptr);
                return TRUE;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

        case WM_SIZE:
            switch (wParam)
            {
                case SIZE_MINIMIZED:
                    m_paused = true;
                    break;

                case SIZE_MAXIMIZED:
                case SIZE_RESTORED:
                    if (m_window_state == SIZE_MINIMIZED)
                    {
                        // unpause since window was minimized and now restored
                        m_paused = false;
                    }
                    on_resize();
                    break;
            }
            m_window_state = wParam;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
