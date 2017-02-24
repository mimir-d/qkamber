#pragma once

#include "math3.h"

class Viewport
{
public:
    Viewport();
    ~Viewport() = default;

    void set_params(int width, int height);
    const mat3x4& get_clip() const;

private:
    mat3x4 m_clip;
};

inline Viewport::Viewport()
{
    flog("id = %#x", this);
    log_info("Created viewport");
}

inline const mat3x4& Viewport::get_clip() const
{
    return m_clip;
}
