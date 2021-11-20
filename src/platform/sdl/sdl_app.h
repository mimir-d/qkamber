#pragma once

#include "engine.h"

class SdlApp : public App
{
public:
    SdlApp(QkEngine::Context& context);
    ~SdlApp() = default;

    int mainloop() final;

private:
    void render_one();
};