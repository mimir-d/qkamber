
#include "stdafx.h"
#include "app.h"
#include "app_window.h"
#include "renderer.h"
using namespace std;

void Application::init(unique_ptr<Renderer> renderer)
{
	m_window = AppWindowFactory::create();
	m_window->init(std::move(renderer));
}

void Application::run()
{
	m_window->mainloop();
}

int Application::shutdown()
{
	return m_window->shutdown();
}