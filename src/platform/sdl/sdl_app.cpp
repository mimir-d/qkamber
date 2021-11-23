
#include "precompiled.h"
#include "sdl_app.h"

#include "engine.h"
#include "render/render_system.h"
#include "scene/scene_system.h"
#include "stats/stats_system.h"
#include "time/time_system.h"
#include "platform/sdl/render/sdl_window.h"

using namespace std;

constexpr Sint32 EVENT_ENGINE_PAUSE = 1;
constexpr Sint32 EVENT_RESIZE_END = 2;

SdlApp::SdlApp(QkEngine::Context& context) :
    App(context)
{
    flog("id = %#x", this);

    // on_create gets called after we get a render target and everything is ready for rendering
    m_context.on_create();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
        throw std::runtime_error("SDL_Init failed");

    log_info("Created SDL application");
}

int SdlApp::mainloop()
{
    flog();
    SDL_Event e;
    bool paused = true;

    while (!m_context.get_exit_requested())
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_USEREVENT && e.user.code == EVENT_ENGINE_PAUSE)
            {
                paused = static_cast<bool>(e.user.data1);
                continue;
            }
            handle_event(e);
        }

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

void SdlApp::handle_event(const SDL_Event& e)
{
    switch (e.type)
    {
        case SDL_QUIT:
            m_context.notify_exit();
            break;

        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
                case SDL_WINDOWEVENT_SHOWN:
                    pause_timer(false);
                    break;

                case SDL_WINDOWEVENT_HIDDEN:
                    pause_timer(true);
                    break;

                case SDL_WINDOWEVENT_RESIZED:
                    handle_window_resize_start(e.window.windowID);
                    break;
            }
            break;

        case SDL_USEREVENT:
            switch (e.user.code)
            {
                case EVENT_RESIZE_END:
                    handle_window_resize_end(e.user.windowID);
                    pause_timer(false);
                    break;
            }
            break;
    }
}

void SdlApp::pause_timer(bool enable)
{
    auto& time = m_context.get_time();
    if (enable)
        time.stop();
    else
        time.resume();

    SDL_Event e;
    e.type = SDL_USEREVENT;
    e.user.code = EVENT_ENGINE_PAUSE;
    e.user.data1 = reinterpret_cast<void*>(enable);
    SDL_PushEvent(&e);
}

static Uint32 resize_debounce(Uint32 interval, void* param)
{
    Uint64 data = reinterpret_cast<Uint64>(param);

    SDL_Event e;
    e.type = SDL_USEREVENT;
    e.user.code = EVENT_RESIZE_END;
    e.user.windowID = static_cast<Uint32>(data);
    SDL_PushEvent(&e);

    return 0;
}

void SdlApp::handle_window_resize_start(Uint32 window_id)
{
    static SDL_TimerID resize_timer;
    SDL_RemoveTimer(resize_timer);
    resize_timer = SDL_AddTimer(50, resize_debounce, reinterpret_cast<void*>(window_id));

    pause_timer(true);
}

void SdlApp::handle_window_resize_end(Uint32 window_id)
{
    SDL_Window* window = SDL_GetWindowFromID(window_id);

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    void* data = SDL_GetWindowData(window, SDL_WINDOW_DATA_PTR);
    SdlWindow* sdl_window = static_cast<SdlWindow*>(data);

    sdl_window->resize(width, height);
}
