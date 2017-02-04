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

    void on_key_pressed(int key_code);
    void on_paint();
    void on_resize();

private:
    template <typename Func>
    void paused(Func fun);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    MSG m_msg;

    RECT m_client_rect;
    int m_window_state;
    HWND m_window_handle;
    bool m_paused;
};

template <typename Func>
inline void Win32Window::paused(Func fun)
{
    m_paused = true;
    fun();
    m_paused = false;
}