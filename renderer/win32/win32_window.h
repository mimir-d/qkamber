#pragma once

#include "window.h"

class Application;
class Renderer;

class Win32Window : public Window
{
public:
    void init(Application* app, Timer* timer) override;
    void mainloop() override;
    int shutdown() override;

private:
    void init_class();
    void init_window();
    void init_render_device();

    void on_paint();
    void on_resize();
    LRESULT wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    template <typename Func>
    void paused(Func fun);

private:
    RECT m_client_rect;
    int m_window_state;
    bool m_paused;

    HWND m_window_handle;
    int m_exit_code;
};

template <typename Func>
inline void Win32Window::paused(Func fun)
{
    m_paused = true;
    fun();
    m_paused = false;
}