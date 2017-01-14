#pragma once

#include "app_window.h"

class Renderer;

class Win32AppWindow : public AppWindow
{
public:
    void init(std::unique_ptr<Renderer> renderer) override;
    void mainloop() override;
    int shutdown() override;

    void on_key_pressed(int key_code);
    void on_paint();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_window_handle;

    MSG m_msg;
};