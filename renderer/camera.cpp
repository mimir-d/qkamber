
#include "stdafx.h"
#include "camera.h"

FpsCamera::FpsCamera(const vec3& eye, const vec3& at, const vec3& up)
{
    m_view = mat4::lookat(eye, at, up);
    m_proj = mat4::proj_perspective(3.14f / 2, 1.333f, 0.01f, 100.0f);
}