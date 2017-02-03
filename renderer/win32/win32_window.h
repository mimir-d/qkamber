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

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_window_handle;

    MSG m_msg;
};