
#include "stdafx.h"
#include "app_window.h"
#include "renderer.h"
using namespace std;
using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////
// AppWindow
///////////////////////////////////////////////////////////////////////////////
void AppWindow::init(unique_ptr<Renderer> renderer)
{
	flog();
	m_renderer = std::move(renderer);
	m_renderer->init();
}

void AppWindow::mainloop()
{}

int AppWindow::shutdown()
{
	flog();
	m_renderer->shutdown();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Win32AppWindow
///////////////////////////////////////////////////////////////////////////////
class Win32AppWindow : public AppWindow
{
public:
	void init(unique_ptr<Renderer> renderer) override;
	void mainloop() override;
	int shutdown() override;

	void on_key_pressed(int key_code);
	void on_paint();

private:
	static const char* mc_window_title;
	static const char* mc_window_class;

	void create_framebuffers();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND m_windowHandle;

	HBRUSH m_backbuffer_brush;
	HBITMAP m_backbuffer_bitmap;
	HDC m_frontbuffer, m_backbuffer;

	ULONG_PTR m_gdiplusToken;
	MSG m_msg;
};

const char* Win32AppWindow::mc_window_title = "my little renderererererer";
const char* Win32AppWindow::mc_window_class = "win32app";

void Win32AppWindow::init(unique_ptr<Renderer> renderer)
{
	flog();
	AppWindow::init(std::move(renderer));

	WNDCLASSEX wcex;
	const HINSTANCE hInstance = GetModuleHandle(nullptr);

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

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
    wcex.lpszClassName  = mc_window_class;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
		throw exception("Call to RegisterClassEx failed!");
	dlog("Registered class: %s", mc_window_class);

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
    m_windowHandle = CreateWindow(
        mc_window_class,
		mc_window_title,
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!m_windowHandle)
		throw exception("Call to CreateWindow failed!");
	dlog("Created window %#x: %s", m_windowHandle, mc_window_title);

	// save this pointer for wndproc
	SetWindowLong(m_windowHandle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(m_windowHandle, SW_SHOW);
    UpdateWindow(m_windowHandle);

    create_framebuffers();
    m_paused = false;
}

void Win32AppWindow::mainloop()
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

int Win32AppWindow::shutdown()
{
	flog();

	// delete backbuffer
	DeleteObject(m_backbuffer_brush);
	DeleteObject(m_backbuffer_bitmap);
    DeleteDC(m_backbuffer);
    ReleaseDC(m_windowHandle, m_frontbuffer);
	
	GdiplusShutdown(m_gdiplusToken);

	AppWindow::shutdown();
    return static_cast<int>(m_msg.wParam);
}

void Win32AppWindow::on_key_pressed(int key_code)
{
	dlog("Pressed %d", key_code);
	switch (key_code)
	{
		case VK_ESCAPE:
			PostMessage(m_windowHandle, WM_CLOSE, 0, 0);
			break;
	}
}

void Win32AppWindow::on_paint()
{
	// clear backbuffer
	RECT rc;
	GetClientRect(m_windowHandle, &rc);
	FillRect(m_backbuffer, &rc, m_backbuffer_brush);

	m_renderer->begin_frame();
	m_renderer->on_draw(Graphics(m_backbuffer));
	m_renderer->end_frame();

	BitBlt(m_frontbuffer,
		rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		m_backbuffer,
		0, 0,
		SRCCOPY
	);
}

void Win32AppWindow::create_framebuffers()
{
	flog();

	RECT rc;
	GetClientRect(m_windowHandle, &rc);
    
	m_frontbuffer = GetDC(m_windowHandle);
    m_backbuffer = CreateCompatibleDC(m_frontbuffer);
	m_backbuffer_bitmap = CreateCompatibleBitmap(
		m_frontbuffer,
        rc.right - rc.left,
        rc.bottom - rc.top
	);

	SelectObject(m_backbuffer, m_backbuffer_bitmap);

	m_backbuffer_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
}

LRESULT CALLBACK Win32AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	Win32AppWindow* window = reinterpret_cast<Win32AppWindow*>(GetWindowLong(hWnd, GWL_USERDATA));

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

///////////////////////////////////////////////////////////////////////////////
// AppWindowFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<AppWindow> AppWindowFactory::create()
{
	// switch on platform
#ifdef WIN32
	dlog("AppWindowFactory creating a Win32AppWindow...");
	return unique_ptr<AppWindow>(new Win32AppWindow);
#endif
}
