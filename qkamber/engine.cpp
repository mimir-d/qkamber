
#include "precompiled.h"
#include "engine.h"

#include "platform.h"

using namespace std;

QkEngine::Context::Context()
{
    flog();

    // init subsystems?
    m_timer.reset(new Timer);
    m_renderer.reset(new Renderer(*this));

    log_info("Finished creating engine context");
}

QkEngine::Context::~Context()
{
    flog();

    // i want to manually destroy systems
    m_renderer = nullptr;
    m_timer = nullptr;

    log_info("Destroyed engine context");
}

int QkEngine::run()
{
    flog();

    try
    {
        std::unique_ptr<App> app = AppFactory::create(m_context);

        log_info("Starting main message loop...");
        int rc = app->mainloop();
        log_info("Finished main message loop");

        return rc;
    }
    catch (exception& ex)
    {
        log_error(">> Exception: %s", ex.what());
        throw;
    }
}

// todo: keep entity system here + subsystems
// timing is a subsystem?
