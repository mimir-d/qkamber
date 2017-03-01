
#include "precompiled.h"
#include "engine.h"

#include "time/time_system.h"
#include "entity/entity_system.h"
#include "render/render_system.h"
#include "scene/scene_system.h"
#include "input/input_system.h"
#include "stats/stats_system.h"
#include "platform.h"

using namespace std;

QkEngine::Context::Context()
{
    flog();

    // init subsystems? all of these will have context as parent
    m_time.reset(new TimeSystem{ *this });
    m_entity.reset(new EntitySystem{ *this });
    m_render.reset(new RenderSystem{ *this });
    m_scene.reset(new SceneSystem{ *this });
    m_input.reset(new InputSystem{ *this });
    m_stats.reset(new StatsSystem{ *this });

    log_info("Finished creating engine context");
}

QkEngine::Context::~Context()
{
    flog();

    // i want to manually destroy systems
    m_stats = nullptr;
    m_input = nullptr;
    m_scene = nullptr;
    m_render = nullptr;
    m_entity = nullptr;
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
