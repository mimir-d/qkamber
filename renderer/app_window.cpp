
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
	m_renderer = std::move(renderer);
	m_renderer->init();
}

void AppWindow::mainloop()
{}

int AppWindow::shutdown()
{
	m_renderer->shutdown();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Win32AppWindow
///////////////////////////////////////////////////////////////////////////////
#define IDT_REDRAW 0x0100

class Win32AppWindow : public AppWindow
{
public:
	void init(unique_ptr<Renderer> renderer) override;
	void mainloop() override;
	int shutdown() override;

	void on_key_pressed(int key_code);
	void on_paint(Graphics& g);

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND m_windowHandle;
	ULONG_PTR m_gdiplusToken;
	MSG m_msg;
};

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
    wcex.lpszClassName  = L"win32app";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
		throw exception("Call to RegisterClassEx failed!");
	log_debug("Registered class %s", "win32app");

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
        L"win32app",
		L"my little renderererererer",
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
    log_debug("Created window %s", "my little render");

	// save this pointer for wndproc
	SetWindowLong(m_windowHandle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(m_windowHandle, SW_SHOW);
    UpdateWindow(m_windowHandle);

	SetTimer(m_windowHandle, IDT_REDRAW, 16, nullptr);
}

void Win32AppWindow::mainloop()
{
	flog();
	while (GetMessage(&m_msg, nullptr, 0, 0))
    {
        TranslateMessage(&m_msg);
        DispatchMessage(&m_msg);
    }
}

int Win32AppWindow::shutdown()
{
	flog();
	KillTimer(m_windowHandle, IDT_REDRAW);
	GdiplusShutdown(m_gdiplusToken);

	AppWindow::shutdown();
    return static_cast<int>(m_msg.wParam);
}

void Win32AppWindow::on_key_pressed(int key_code)
{
	switch (key_code)
	{
		case VK_ESCAPE:
			PostMessage(m_windowHandle, WM_CLOSE, 0, 0);
			break;
	}
}

void Win32AppWindow::on_paint(Graphics& g)
{
	m_renderer->begin_frame();
	m_renderer->on_draw(g);
	m_renderer->end_frame();
}

LRESULT CALLBACK Win32AppWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	Win32AppWindow* window = reinterpret_cast<Win32AppWindow*>(GetWindowLong(hWnd, GWL_USERDATA));

	switch (message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			window->on_paint(Graphics(hdc));
			EndPaint(hWnd, &ps);
			break;

		case WM_KEYDOWN:
			window->on_key_pressed(wParam);
			break;

		case WM_TIMER:
			switch (wParam)
			{
				case IDT_REDRAW:
					window->m_renderer->on_update();
					RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
					UpdateWindow(hWnd);
					break;
			}
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
	return unique_ptr<AppWindow>(new Win32AppWindow);
#endif
}
