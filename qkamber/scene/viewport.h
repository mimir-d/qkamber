#pragma once

#include "math3.h"

// TODO: maybe remove viewport control from user?
class Viewport
{
public:
    virtual ~Viewport() = default;

    virtual const mat3x4& get_clip() const = 0;
};

class RectViewport : public Viewport
{
public:
    RectViewport();
    ~RectViewport();

    const mat3x4& get_clip() const final;
    void set_params(int width, int height);

private:
    mat3x4 m_clip;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline RectViewport::RectViewport()
{
    flog("id = %#x", this);
    log_info("Created rect viewport");
}

inline RectViewport::~RectViewport()
{
    flog();
    log_info("Destroyed rect viewport");
}

inline const mat3x4& RectViewport::get_clip() const
{
    return m_clip;
}
