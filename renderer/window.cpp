
#include "stdafx.h"
#include "window.h"
#include "win32/win32_window.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// AppWindow
///////////////////////////////////////////////////////////////////////////////
void Window::init(Application* app, Timer* timer)
{
	flog();
    m_app = app;
    m_timer = timer;
}

void Window::mainloop()
{}

int Window::shutdown()
{
	flog();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// AppWindowFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<Window> AppWindowFactory::create()
{
	// switch on platform
#ifdef WIN32
	dlog("AppWindowFactory creating a Win32AppWindow...");
	return unique_ptr<Window>(new Win32Window);
#endif
    return nullptr;
}
