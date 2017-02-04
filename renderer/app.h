#pragma once

#include "renderer.h"

class Application
{
public:
    // TODO: temp
    Renderer& get_renderer()
    {
        return m_renderer;
    }

public:
    virtual void on_create() = 0;
    virtual void on_resize(int width, int height) = 0;

    virtual void update(float abs_time, float elapsed_time) = 0;
    virtual void render(float abs_time, float elapsed_time) = 0;

protected:
    Renderer m_renderer;
};
