#pragma once

#include "math3.h"

class Camera
{
public:
    virtual ~Camera() = default;

    const mat4& get_view() const;
    const mat4& get_proj() const;

protected:
    mat4 m_view;
    mat4 m_proj;
};

class FpsCamera : public Camera
{
public:
    FpsCamera(const vec3& position);
    ~FpsCamera() = default;

    void set_proj_params(int width, int height);
    void update(float abs_time, float elapsed_time);

    const vec3& get_position() const;
    const vec2& get_rotation() const;

private:
    vec2 get_rotation_delta();
    vec3 get_position_delta(float elapsed_time);

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
// Camera impl
///////////////////////////////////////////////////////////////////////////////
inline const mat4& Camera::get_view() const
{
    return m_view;
}

inline const mat4& Camera::get_proj() const
{
    return m_proj;
}

///////////////////////////////////////////////////////////////////////////////
// FpsCamera impl
///////////////////////////////////////////////////////////////////////////////
inline FpsCamera::FpsCamera(const vec3& position) :
    m_position(position)
{
    flog("id = %#x", this);
}

inline const vec3& FpsCamera::get_position() const
{
    return m_position;
}

inline const vec2& FpsCamera::get_rotation() const
{
    return m_rotation;
}