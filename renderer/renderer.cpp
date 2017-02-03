
#include "stdafx.h"
#include "renderer.h"
#include "win32/win32_render_device.h"
#include "math3.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
void Renderer::init(Timer* timer)
{
    flog();
    m_timer = timer;

    m_dev = RenderDeviceFactory::create();

    m_frame_number = 0;
    m_fps_last_count = 0;
    m_fps_last_timestamp = 0;

    m_target_fps = 60;
    m_fps = 0;

    m_camera = std::unique_ptr<Camera>(new FpsCamera {
        { 0, 0, 10 },
        { 0, 0, 0 },
        { 0, 1, 0 }
    });
}
void Renderer::shutdown()
{
	flog();
}

void Renderer::render()
{
    begin_frame();

    mat4 projview = m_camera->get_proj() * m_camera->get_view();
    mat<float, 3, 4> clip = mat4::clip(0, 480, 640, -480, 0.0f, 1.0f);

    mat4 world = mat4::identity();

    mat4 wvp = projview * world;

    vec4 v0(1, -0.5, 0, 1);
    vec4 v1(0, 0.5, 0, 1);
    vec4 v2(-1, -0.5, 0, 1);

    v0 = wvp * v0;
    v0 *= 1.0f / v0.w();

    v1 = wvp * v1;
    v1 *= 1.0f / v1.w();

    v2 = wvp * v2;
    v2 *= 1.0f / v2.w();

    vec3 sv0 = clip * v0;
    vec3 sv1 = clip * v1;
    vec3 sv2 = clip * v2;

    m_dev->draw_tri(sv0.x(), sv0.y(), sv1.x(), sv1.y(), sv2.x(), sv2.y());

    end_frame();
}

void Renderer::begin_frame()
{
    m_dev->clear();
}

void Renderer::end_frame()
{
    ostringstream ostr;
    ostr << "fps: " << fixed << setprecision(2) << m_fps << ends;
    m_dev->draw_text(ostr.str(), 3, 3);

    // count fps
    m_frame_number ++;
    m_fps_last_count ++;

    float now = m_timer->get_abs_time();
    float delta = now - m_fps_last_timestamp;
    if (delta >= 1.0f)
    {
        m_fps = m_fps_last_count / delta;
        m_fps_last_count = 0;
        m_fps_last_timestamp = now;
    }

    m_dev->swap_buffers();
    // delay for fps
}

///////////////////////////////////////////////////////////////////////////////
// RenderDeviceFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<RenderDevice> RenderDeviceFactory::create()
{
#ifdef WIN32
    dlog("RenderDeviceFactory creating a Win32RenderDevice...");
    return unique_ptr<RenderDevice>(new Win32RenderDevice);
#endif
    return nullptr;
}