
#include "stdafx.h"
#include "app_window.h"
#include "renderer.h"
#include "win32/win32_app_window.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// AppWindow
///////////////////////////////////////////////////////////////////////////////
void AppWindow::init(unique_ptr<Renderer> renderer)
{
	flog();
    m_dev = RenderDeviceFactory::create();

	m_renderer = std::move(renderer);
	m_renderer->init(m_dev.get());
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
// AppWindowFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<AppWindow> AppWindowFactory::create()
{
	// switch on platform
#ifdef WIN32
	dlog("AppWindowFactory creating a Win32AppWindow...");
	return unique_ptr<AppWindow>(new Win32AppWindow);
#endif
    return nullptr;
}
