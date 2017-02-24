#pragma once

#include "engine.h"

class Win32App : public App
{
public:
    Win32App(Engine::Context& context);
    ~Win32App() = default;

    int mainloop() final;

private:
    void render_one();
};