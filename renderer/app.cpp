
#include "stdafx.h"
#include "app.h"
#include "window.h"
#include "renderer.h"
using namespace std;

void Application::init()
{
	flog();

    m_renderer.init(&m_timer);
    m_timer.start();

    m_window = AppWindowFactory::create();
	m_window->init(this);
}

void Application::run()
{
	flog();
	m_window->mainloop();
}

int Application::shutdown()
{
	flog();
    m_timer.stop();
	return m_window->shutdown();
}