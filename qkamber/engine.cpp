
#include "precompiled.h"
#include "engine.h"

#include "platform.h"

using namespace std;

QkEngine::Context::Context()
{
    flog();

    // init subsystems? all of these will have context as parent
    m_time.reset(new TimeSystem);
    m_render.reset(new RenderSystem(*this));
    m_input.reset(new InputSystem);

    log_info("Finished creating engine context");
}

QkEngine::Context::~Context()
{
    flog();

    // i want to manually destroy systems
    m_input = nullptr;
    m_render = nullptr;
    m_time = nullptr;

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
