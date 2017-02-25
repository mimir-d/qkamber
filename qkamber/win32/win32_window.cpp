
#include "precompiled.h"
#include "win32_window.h"

#include "win32_app.h"
#include "win32_software_device.h"
#include "win32_input_system.h"
#include "resource.h"

using namespace std;

constexpr char* WINDOW_CLASS = "WIN32_QK_RENDERER";
constexpr char* WINDOW_TITLE = "Qukamber renderer";

///////////////////////////////////////////////////////////////////////////////
// Win32ColorBuffer impl
///////////////////////////////////////////////////////////////////////////////
Win32ColorBuffer::Win32ColorBuffer(HDC surface_dc, int width, int height) :
    m_surface_dc(surface_dc)
{
    flog("id = %#x", this);

    m_dc = CreateCompatibleDC(surface_dc);
    if (GetDeviceCaps(surface_dc, BITSPIXEL) != 32)
    {
        // TODO: fix this with forcing the window bpp thru wgl
        throw exception("window needs to be 32bpp");
    }

    resize(width, height);
    log_info("Created win32 color buffer");
}

Win32ColorBuffer::~Win32ColorBuffer()
{
    DeleteObject(m_bitmap);
    DeleteDC(m_dc);
}

void Win32ColorBuffer::resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    // update dimensions
    m_width = width;
    m_height = height;

    BITMAPINFO bi = { 0 };
    BITMAPINFOHEADER& bmi = bi.bmiHeader;
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = GetDeviceCaps(m_surface_dc, PLANES);
    bmi.biBitCount = GetDeviceCaps(m_surface_dc, BITSPIXEL);
    // limit to 1920x1080 because of fixed point math (2048x2048 max)
    bmi.biWidth = min(width, 1920);
    bmi.biHeight = -min(height, 1080);
    bmi.biCompression = BI_RGB;

    DeleteObject(m_bitmap);
    m_bitmap = CreateDIBSection(
        m_surface_dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<PVOID*>(&m_data_ptr),
        nullptr, 0
    );

    m_stride = (bmi.biWidth * bmi.biBitCount + 0x1f) >> 5;

    SelectObject(m_dc, m_bitmap);

    SetBkMode(m_dc, TRANSPARENT);
    SetTextColor(m_dc, 0x0040FF00);
}

///////////////////////////////////////////////////////////////////////////////
// Win32DepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
Win32DepthBuffer::Win32DepthBuffer(int width, int height)
{
    flog("id = %#x", this);

    resize(width, height);
    log_info("Created win32 depth buffer");
}

void Win32DepthBuffer::resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    // update dimensions
    m_width = width;
    m_height = height;

    m_data.reset(new float[height * width]);

    m_data_clear.reset(new float[height * width]);
    std::fill(
        m_data_clear.get(), m_data_clear.get() + height * width,
        std::numeric_limits<float>::max()
    );
}

///////////////////////////////////////////////////////////////////////////////
// Win32Window impl
///////////////////////////////////////////////////////////////////////////////
Win32Window::Win32Window(QkEngine::Context& context, int width, int height) :
    m_context(context)
{
    flog("id = %#x", this);

    // TODO: should be able to create just 1 instances of win32window at any moment
    // TODO: right now, this might fail uncontrollably if create_render_target is called multiple times
    // register class just once?
    register_class();
    create_window(width, height);
    register_inputs();

    log_info("Finished creating win32 window target %#x", this);
}

Win32Window::~Win32Window()
{
    flog();

    DeleteDC(m_dc);
    DestroyWindow(m_window_handle);

    unregister_class();
    log_info("Destroyed win32 window");
}

void Win32Window::register_class()
{
    flog();
    HINSTANCE instance = GetModuleHandle(nullptr);

    WNDCLASSEX wcex = { 0 };

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.hInstance      = instance;
    wcex.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(IDI_QKAMBER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName  = WINDOW_CLASS;
    wcex.hIconSm        = LoadIcon(instance, MAKEINTRESOURCE(IDI_QKAMBER));

    wcex.lpfnWndProc    = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        if (message == WM_CREATE)
        {
            dlog("Got win32 message: WM_CREATE");

            // first message to go, save Win32Window instance to USERDATA
            LPVOID create_arg = reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_arg));
            return 0;
        }

        Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (!window)
        {
            // NOTE: there are a couple of messages that are sent before WM_CREATE gets a chance to set
            // the GWLP_USERDATA pointer for the window, so return the default wndproc before that happens
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return window->wnd_proc(hWnd, message, wParam, lParam);
    };

    if (!RegisterClassEx(&wcex))
        throw exception("RegisterClassEx failed");

    dlog("Registered win32 class: %s", WINDOW_CLASS);
}

void Win32Window::unregister_class()
{
    flog();
    HINSTANCE instance = GetModuleHandle(nullptr);

    UnregisterClass(WINDOW_CLASS, instance);
    dlog("Unregistered win32 class: %s", WINDOW_CLASS);
}

void Win32Window::create_window(int width, int height)
{
    flog();
    HINSTANCE instance = GetModuleHandle(nullptr);

    RECT rc = { 0, 0, width, height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
    const int window_width = rc.right - rc.left;
    const int window_height = rc.bottom - rc.top;

    m_window_handle = CreateWindow(
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_width, window_height,
        nullptr,
        nullptr,
        instance,
        reinterpret_cast<LPVOID>(this)
    );

    if (!m_window_handle)
        throw exception("CreateWindow failed");
    log_info("Created window hwnd = %#x, title = %s", m_window_handle, WINDOW_TITLE);

    m_dc = GetDC(m_window_handle);

    // create render buffer objects
    m_color_buf = std::make_unique<Win32ColorBuffer>(m_dc, width, height);
    m_depth_buf = std::make_unique<Win32DepthBuffer>(width, height);

    ShowWindow(m_window_handle, SW_SHOW);
    UpdateWindow(m_window_handle);
}

void Win32Window::register_inputs()
{
    flog();

    auto& input = m_context.get_input();
    auto& mouse = static_cast<Win32MouseDevice&>(input.get_mouse());
    auto& keyboard = static_cast<Win32KeyboardDevice&>(input.get_keyboard());

    mouse.win32_init(m_window_handle);
    keyboard.win32_init(m_window_handle);
}

void Win32Window::pause_timer(bool enable)
{
    auto& timer = m_context.get_timer();
    if (enable)
        timer.stop();
    else
        timer.resume();

    // also discard any input while paused
    m_discard_input = enable;
}

bool Win32Window::on_input(HRAWINPUT raw_handle)
{
    RAWINPUT raw_input;
    UINT raw_data_size = sizeof(raw_input);

    GetRawInputData(raw_handle, RID_INPUT, &raw_input, &raw_data_size, sizeof(RAWINPUTHEADER));

    auto& renderer = m_context.get_renderer();
    auto& input = m_context.get_input();

    // discard any input while paused
    if (m_discard_input)
        return 0;

    switch (raw_input.header.dwType)
    {
        case RIM_TYPEMOUSE:
            static_cast<Win32MouseDevice&>(input.get_mouse()).feed_input(raw_input.data.mouse);
            break;

        case RIM_TYPEKEYBOARD:
            static_cast<Win32KeyboardDevice&>(input.get_keyboard()).feed_input(raw_input.data.keyboard);
            break;
    }

    return 0;
}

void Win32Window::on_resize(RECT& rc)
{
    if (!EqualRect(&m_rect, &rc))
    {
        auto& renderer = m_context.get_renderer();
        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;

        // NOTE: only pause if the window state is not sizing because WM_ENTERSIZEMOVE does the pausing already
        if (m_window_state != WindowState::Sizing)
            pause_timer(true);

        m_color_buf->resize(width, height);
        m_depth_buf->resize(width, height);
        m_context.on_resize(width, height);

        if (m_window_state != WindowState::Sizing)
            pause_timer(false);

        CopyRect(&m_rect, &rc);
    }
}

LRESULT Win32Window::wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto& renderer = m_context.get_renderer();

    switch (msg)
    {
        case WM_ACTIVATEAPP:
            dlog("Got win32 message: WM_ACTIVATEAPP %d", wp);
            pause_timer(!wp);
            // pause the engine asap
            PostMessage(hwnd, CCM_ENGINE_PAUSE, static_cast<WPARAM>(!wp), 0);
            break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_SETCURSOR:
            // hide the cursor when inside the client area
            if (LOWORD(lp) == HTCLIENT)
            {
                SetCursor(nullptr);
                return TRUE;
            }
            return DefWindowProc(hwnd, msg, wp, lp);

        case WM_PAINT:
        {
            if (m_window_state != WindowState::Sizing)
                return DefWindowProc(hwnd, msg, wp, lp);

            // while sizing or moving, draw one frame with no elapsed time
            auto& timer = m_context.get_timer();
            m_context.on_update(timer.get_abs_time(), 0);
            m_context.on_render(timer.get_abs_time(), 0);
            break;
        }

        case WM_INPUT:
            on_input(reinterpret_cast<HRAWINPUT>(lp));
            break;

        case WM_ENTERSIZEMOVE:
            dlog("Got win32 message: WM_ENTERSIZEMOVE");
            m_window_state = WindowState::Sizing;

            pause_timer(true);
            break;

        case WM_EXITSIZEMOVE:
            dlog("Got win32 message: WM_EXITSIZEMOVE");
            m_window_state = WindowState::Normal;

            pause_timer(false);
            // ensure the engine isnt paused when exiting the size/move loop
            PostMessage(hwnd, CCM_ENGINE_PAUSE, static_cast<WPARAM>(false), 0);
            break;

        case WM_SIZE:
            switch (wp)
            {
                case SIZE_MAXIMIZED:
                case SIZE_RESTORED:
                    if (m_window_state == WindowState::Unknown)
                    {
                        dlog("Got win32 message: WM_SIZE at window creation");
                        m_window_state = WindowState::Normal;
                    }

                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    on_resize(rc);
                    break;
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }

    return 0;
}