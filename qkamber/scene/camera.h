#pragma once

#include "engine.h"
#include "math3.h"

class Camera
{
public:
    virtual ~Camera() = default;

    virtual const mat4& get_view() const = 0;
    virtual const mat4& get_proj() const = 0;

    virtual const mat4& get_view_inv() const = 0;
};

class FpsCamera : public Camera
{
public:
    FpsCamera(QkEngine::Context& context, const vec3& position);
    ~FpsCamera();

    const mat4& get_view() const final;
    const mat4& get_proj() const final;

    const mat4& get_view_inv() const final;

    void set_proj_params(int width, int height);
    void update();

    const vec3& get_position() const;
    const vec2& get_rotation() const;

private:
    mat4 m_view, m_view_inv;
    mat4 m_proj;

    vec2 get_rotation_delta();
    vec3 get_position_delta();

    InputSystem& m_input;
    TimeSystem& m_time;

    vec3 m_position;
    vec2 m_rotation;

    // position delta stuff
    float m_accel_scaler = 20.0f;
    float m_accel_duration = 0.1f;
    float m_accel_time;
    vec3 m_velocity;
    vec3 m_acceleration;

    // rotation delta stuff
    float m_rotation_scaler = 0.0015f;
    float m_rotation_smooth_frames = 4.0f;
    vec2 m_mouse_last_abs;
    vec2 m_mouse_delta;
};

///////////////////////////////////////////////////////////////////////////////
// FpsCamera impl
///////////////////////////////////////////////////////////////////////////////
inline FpsCamera::FpsCamera(QkEngine::Context& context, const vec3& position) :
    m_input(context.get_input()),
    m_time(context.get_time()),
    m_position(position)
{
    flog("id = %#x", this);
    log_info("Created fps camera at %.3f %.3f %.3f", position.x(), position.y(), position.z());
}

inline FpsCamera::~FpsCamera()
{
    flog();
    log_info("Destroyed fps camera");
}

inline const mat4& FpsCamera::get_view() const
{
    return m_view;
}

inline const mat4& FpsCamera::get_proj() const
{
    return m_proj;
}

inline const mat4& FpsCamera::get_view_inv() const
{
    return m_view_inv;
}

inline const vec3& FpsCamera::get_position() const
{
    return m_position;
}

inline const vec2& FpsCamera::get_rotation() const
{
    return m_rotation;
}