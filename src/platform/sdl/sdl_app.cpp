
#include "precompiled.h"
#include "sdl_app.h"

#include "engine.h"
#include "render/render_system.h"
#include "scene/scene_system.h"
#include "stats/stats_system.h"
#include "time/time_system.h"

using namespace std;

SdlApp::SdlApp(QkEngine::Context& context) :
    App(context)
{
    flog("id = %#x", this);

    // on_create gets called after we get a render target and everything is ready for rendering
    m_context.on_create();

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("SDL_Init failed");

    log_info("Created SDL application");
}

int SdlApp::mainloop()
{
    flog();
    SDL_Event e;
    bool paused = false;

    while (!m_context.get_exit_requested())
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            break;

        if (paused)
        {
            SDL_Delay(10);
            continue;
        }
        render_one();
    }
    m_context.on_destroy();

    SDL_Quit();
    return 0;
}

void SdlApp::render_one()
{
    m_context.on_update();

    auto& scene = m_context.get_scene();
    scene.process();

    auto& render = m_context.get_render();
    render.process();

    auto& stats = m_context.get_stats();
    stats.process();

    auto& time = m_context.get_time();
    time.process();
}
