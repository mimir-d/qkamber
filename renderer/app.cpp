
#include "stdafx.h"
#include "app.h"
#include "window.h"
#include "renderer.h"
using namespace std;

void Application::init(unique_ptr<Renderer> renderer)
{
	flog();
	m_window = AppWindowFactory::create();
	m_window->init(std::move(renderer));
}

void Application::run()
{
	flog();
	m_window->mainloop();
}

int Application::shutdown()
{
	flog();
	return m_window->shutdown();
}