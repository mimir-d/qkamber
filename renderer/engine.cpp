
#include "precompiled.h"
#include "engine.h"

#include "render_window.h"
#include "app.h"

Engine::Engine()
{
    // NOTE: this has to be inited _before_ any logging usage
    Logger::get().set_output_file("qukamber.log");

    flog();
    // init subsystems?

    m_global_timer.resume();
    log_info("Created Qukamber engine instance");
}

Engine::~Engine()
{
    flog();
    m_global_timer.stop();

    log_info("Destroyed engine instance");
}

void Engine::run(Application& app)
{
    // TODO: should create renderer here as well
    auto& renderer = app.get_renderer();
    renderer.init(&m_global_timer);

    app.on_create();

    std::unique_ptr<RenderWindow> window = AppWindowFactory::create();
    window->init(&app, &m_global_timer);
    window->mainloop();

    renderer.shutdown();
    m_exit_code = window->shutdown();
}

// todo: keep entity system here + subsystems
// timing is a subsystem?
