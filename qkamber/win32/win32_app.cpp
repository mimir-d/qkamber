
#include "precompiled.h"
#include "win32_app.h"

#include "engine.h"
#include "render/render_system.h"
#include "stats/stats_system.h"
#include "time/time_system.h"

using namespace std;

Win32App::Win32App(QkEngine::Context& context) :
    App(context)
{
    flog("id = %#x", this);

    // on_create gets called after we get a render target and everything is ready for rendering
    m_context.on_create();

    log_info("Created win32 application");
}

int Win32App::mainloop()
{
    flog();
    MSG msg;
    bool paused = true;

    while (!m_context.get_exit_requested())
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == CCM_ENGINE_PAUSE)
            {
                dlog("Got win32 message: CCM_ENGINE_PAUSE %d", msg.wParam);
                paused = !!msg.wParam;
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (paused)
        {
            Sleep(10);
            continue;
        }
        render_one();
    }
    m_context.on_destroy();

    return 0;
}

void Win32App::render_one()
{
    m_context.on_update();

    auto& render = m_context.get_render();
    render.process();

    auto& stats = m_context.get_stats();
    stats.process();

    auto& time = m_context.get_time();
    time.process();

    //     const float frame_time = 1.0f / 15.0f;
    //     const float ctime = m_timer->get_abs_time() - abs_time;
    //     if (ctime < frame_time)
    //         Sleep((DWORD)((frame_time - ctime) * 1000.0f));
}


