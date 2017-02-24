
#include "precompiled.h"
#include "renderer.h"

#include "engine.h"
#include "render_queue.h"
#include "render_primitive.h"
#include "mesh.h"
#include "math3.h"
#include "platform.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(Engine::Context& context) :
    m_context(context)
{
    flog("id = %#x", this);
    m_dev = RenderDeviceFactory::create(*this);
    log_info("Created renderer");
}

Renderer::~Renderer()
{
    flog();
    log_info("Destroyed renderer");
}

void Renderer::begin_frame()
{
    m_dev->clear();
}

void Renderer::render()
{
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

    float now = m_context.get_timer().get_abs_time();
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

void Renderer::resize(int width, int height)
{
    // TODO: not sure if this should exist
    m_context.on_resize(width, height);
}

void Renderer::pause(bool enabled)
{
    if (m_paused == enabled)
        return;

    auto& timer = m_context.get_timer();
    if (enabled)
    {
        timer.stop();
        dlog("Rendering stopped");
    }
    else
    {
        timer.resume();
        dlog("Rendering resumed");
    }
    m_paused = enabled;
}

