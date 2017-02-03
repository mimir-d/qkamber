#pragma once

#include "math3.h"

class Camera
{
public:
    const mat4& get_view() const;
    const mat4& get_proj() const;

protected:
    mat4 m_view;
    mat4 m_proj;
};

class FpsCamera : public Camera
{
public:
    FpsCamera(const vec3& eye, const vec3& at, const vec3& up);
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