
#include "stdafx.h"
#include "resource.h"
#include "app.h"
#include "win32_window.h"
#include "win32_software_device.h"
#include "win32_input_system.h"

using namespace std;

constexpr char* WINDOW_TITLE = "my little renderererererer";
constexpr char* WINDOW_CLASS = "WIN32_RENDERER";

#pragma warning(disable:4302)

void Win32Window::init(Application* app, Timer* timer)
{
    flog("id = %#x", this);
    Window::init(app, timer);

    init_class();
    init_window();
    init_inputs();

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
            if (msg.message == WM_QUIT)
            {
                m_exit_code = msg.wParam;
                return;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
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

        // get input
        // TODO: this should probably have its own method
        if (message == WM_INPUT)
        {
            HRAWINPUT raw_handle = reinterpret_cast<HRAWINPUT>(lParam);
            RAWINPUT input;
            UINT raw_data_size = sizeof(input);

            GetRawInputData(raw_handle, RID_INPUT, &input, &raw_data_size, sizeof(RAWINPUTHEADER));

            // discard any input while paused
            if (window->m_paused)
                return 0;

            auto& is = InputSystem::get_inst();
            switch (input.header.dwType)
            {
                case RIM_TYPEMOUSE:
                    static_cast<Win32MouseDevice&>(is.get_mouse()).feed_input(input.data.mouse);
                    break;

                case RIM_TYPEKEYBOARD:
                    static_cast<Win32KeyboardDevice&>(is.get_keyboard()).feed_input(input.data.keyboard);
                    break;
            }

            return 0;
        }

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
    dlog("Created hwnd = %#x, title = %s", m_window_handle, WINDOW_TITLE);

    // save this pointer for wndproc
    SetWindowLong(m_window_handle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    // need to init here because of update window
    init_render_device();

    ShowWindow(m_window_handle, SW_SHOW);
    UpdateWindow(m_window_handle);
    m_window_state = SIZE_RESTORED;
}

void Win32Window::init_render_device()
{
    flog();

    // init win32 specific render device
    auto& dev = m_app->get_renderer().get_device();
    static_cast<Win32SoftwareDevice&>(dev).win32_init(m_window_handle);
}

void Win32Window::init_inputs()
{
    flog();

    auto& is = InputSystem::get_inst();
    auto& mouse = static_cast<Win32MouseDevice&>(is.get_mouse());
    auto& keyboard = static_cast<Win32KeyboardDevice&>(is.get_keyboard());

    mouse.win32_init(m_window_handle);
    keyboard.win32_init(m_window_handle);
}

void Win32Window::on_paint()
{
    float abs_time = m_timer->get_abs_time();
    float diff_time = m_timer->get_diff_time();

    m_app->update(abs_time, diff_time);
    m_app->render(abs_time, diff_time);

//     const float frame_time = 1.0f / 15.0f;
//     const float ctime = m_timer->get_abs_time() - abs_time;
//     if (ctime < frame_time)
//         Sleep((DWORD)((frame_time - ctime) * 1000.0f));
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
            static_cast<Win32SoftwareDevice&>(dev).win32_resize(&rc);

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
            pause(!wParam);
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
                    pause(true);
                    break;

                case SIZE_MAXIMIZED:
                case SIZE_RESTORED:
                    if (m_window_state == SIZE_MINIMIZED)
                    {
                        // unpause since window was minimized and now restored
                        pause(false);
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

void Win32Window::pause(bool enabled)
{
    if (enabled)
    {
        m_timer->stop();
        dlog("Render stopped");
    }
    else
    {
        m_timer->resume();
        dlog("Render resumed");
    }
    m_paused = enabled;
}
