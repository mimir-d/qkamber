#pragma once

#include "engine.h"

constexpr UINT CCM_ENGINE_PAUSE = WM_APP + 0x100;

class Win32App : public App
{
public:
    Win32App(QkEngine::Context& context);
    ~Win32App() = default;

    int mainloop() final;

private:
    void render_one();
};