
#include "stdafx.h"
#include "window.h"
#include "renderer.h"
#include "win32/win32_window.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// AppWindow
///////////////////////////////////////////////////////////////////////////////
void Window::init(unique_ptr<Renderer> renderer)
{
	flog();
    m_dev = RenderDeviceFactory::create();

	m_renderer = std::move(renderer);
	m_renderer->init(m_dev.get());
}

void Window::mainloop()
{}

int Window::shutdown()
{
	flog();
	m_renderer->shutdown();
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
