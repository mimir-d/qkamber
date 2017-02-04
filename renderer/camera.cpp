
#include "stdafx.h"
#include "camera.h"

constexpr float FOV = static_cast<float>(M_PI) / 4;

void FpsCamera::set_params(const vec3& eye, const vec3& at, const vec3& up)
{
    m_view = mat4::lookat(eye, at, up);
}

void FpsCamera::set_proj_params(int width, int height)
{
    const float aspect = static_cast<float>(width) / height;
    m_proj = mat4::proj_perspective(FOV, aspect, 0.01f, 100.0f);
}