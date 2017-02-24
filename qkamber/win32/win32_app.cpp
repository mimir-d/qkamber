
#include "precompiled.h"
#include "win32_app.h"

#include "engine.h"
#include "renderer.h"
#include "win32_window.h"
#include "win32_input_system.h"
#include "resource.h"

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

    auto& renderer = m_context.get_renderer();
    renderer.pause(false);

    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return static_cast<int>(msg.wParam);

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (renderer.is_paused())
        {
            Sleep(10);
            continue;
        }
        render_one();
    }

    return 0;
}

void Win32App::render_one()
{
    auto& timer = m_context.get_timer();
    float abs_time = timer.get_abs_time();
    float diff_time = timer.get_diff_time();

    m_context.on_update(abs_time, diff_time);
    m_context.on_render(abs_time, diff_time);

    //     const float frame_time = 1.0f / 15.0f;
    //     const float ctime = m_timer->get_abs_time() - abs_time;
    //     if (ctime < frame_time)
    //         Sleep((DWORD)((frame_time - ctime) * 1000.0f));
}


