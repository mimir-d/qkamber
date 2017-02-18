
#include "precompiled.h"
#include "renderer.h"
#include "render_queue.h"
#include "render_primitive.h"
#include "mesh.h"
#include "win32/win32_software_device.h"
#include "math3.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
void Renderer::init(Timer* timer)
{
    flog("id = %#x", this);
    m_timer = timer;

    m_dev = RenderDeviceFactory::create();

    m_frame_number = 0;
    m_fps_last_count = 0;
    m_fps_last_timestamp = 0;

    m_target_fps = 60;
    m_fps = 0;

    log_info("Initialized renderer");
}
void Renderer::shutdown()
{
	flog();
    log_info("Shutdown renderer");
}

void Renderer::begin_frame()
{
    m_dev->clear();
}

void Renderer::render()
{
    // TODO: f-it, just remove this
    GdiFlush();

    // set camera and viewport just once
    m_dev->set_view_matrix(m_camera->get_view());
    m_dev->set_proj_matrix(m_camera->get_proj());
    m_dev->set_clip_matrix(m_viewport->get_clip());

    for (auto& qi : m_queue)
    {
        m_dev->set_world_matrix(qi.world_matrix);
        m_dev->draw_primitive(qi.mesh.get_primitive());
    }
}

void Renderer::render_text(const std::string& text, int x, int y)
{
    m_dev->draw_text(text, x, y);
}

void Renderer::end_frame()
{
    // finished drawing the queued objects
    m_queue.clear();

    // TODO: i get the feeling fps is engine state
    m_dev->draw_text(print_fmt("fps: %.2f", m_fps), 3, 3);

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
    log_info("RenderDeviceFactory creating a Win32RenderDevice...");
    return unique_ptr<RenderDevice>(new Win32SoftwareDevice);
#endif
    return nullptr;
}
