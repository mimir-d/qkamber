#pragma once

#include "render_window.h"
#include "win32_input_system.h"

class Application;
class Renderer;

class Win32Window : public RenderWindow
{
public:
    void init(Application* app, Timer* timer) final;
    void mainloop() final;
    int shutdown() final;

private:
    void init_class();
    void init_window();
    void init_render_device();
    void init_inputs();

    void on_paint();
    void on_resize();
    LRESULT wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void pause(bool enable);
    template <typename Func>
    void paused(Func fun);

private:
    RECT m_client_rect;
    int m_window_state;
    bool m_paused = false;

    HWND m_window_handle;
    int m_exit_code = 0;
};

template <typename Func>
inline void Win32Window::paused(Func fun)
{
    pause(true);
    fun();
    pause(false);
}