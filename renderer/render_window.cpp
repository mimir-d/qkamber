
#include "precompiled.h"
#include "render_window.h"
#include "win32/win32_window.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// AppWindow
///////////////////////////////////////////////////////////////////////////////
void RenderWindow::init(Application* app, Timer* timer)
{
    flog();
    m_app = app;
    m_timer = timer;
}

void RenderWindow::mainloop()
{}

int RenderWindow::shutdown()
{
	flog();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// AppWindowFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<RenderWindow> AppWindowFactory::create()
{
	// switch on platform
#ifdef WIN32
	log_info("AppWindowFactory creating a Win32AppWindow...");
	return unique_ptr<RenderWindow>(new Win32Window);
#endif
    return nullptr;
}
