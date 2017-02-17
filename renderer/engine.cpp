
#include "precompiled.h"
#include "engine.h"

#include "window.h"
#include "app.h"

Engine::Engine()
{
    // NOTE: this has to be inited _before_ any logging usage
    Logger::get().set_output_file("qukamber.log");

    flog();
    // init subsystems?

    m_global_timer.resume();
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

    app.on_create();

    std::unique_ptr<Window> window = AppWindowFactory::create();
    window->init(&app, &m_global_timer);
    window->mainloop();

    renderer.shutdown();
    m_exit_code = window->shutdown();
}

// todo: keep entity system here + subsystems
// timing is a subsystem?
