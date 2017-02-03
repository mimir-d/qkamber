
#include "stdafx.h"
#include "engine.h"

#include "window.h"
#include "app.h"

Engine::Engine() :
    m_exit_code(0)
{
    flog();
    // init logging
    // init subsystems?

    m_global_timer.start();
}

Engine::~Engine()
{
    flog();
    m_global_timer.stop();
}

void Engine::run(Application& app)
{
    // TODO: should create renderer here as well
    auto& renderer = app.get_renderer();
    renderer.init(&m_global_timer);

    std::unique_ptr<Window> window = AppWindowFactory::create();
    window->init(&app, &m_global_timer);
    window->mainloop();

    renderer.shutdown();
    m_exit_code = window->shutdown();
}
