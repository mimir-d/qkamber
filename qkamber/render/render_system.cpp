
#include "precompiled.h"
#include "render_system.h"

#include "engine.h"
#include "render_queue.h"
#include "render_primitive.h"
#include "model/mesh.h"
#include "math3.h"
#include "platform.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
RenderSystem::RenderSystem(QkEngine::Context& context) :
    m_context(context)
{
    flog("id = %#x", this);
    m_dev = RenderDeviceFactory::create(context);
    log_info("Created renderer");
}

RenderSystem::~RenderSystem()
{
    flog();
    log_info("Destroyed renderer");
}

void RenderSystem::begin_frame()
{
    m_dev->clear();
}

void RenderSystem::render()
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

void RenderSystem::render_text(const std::string& text, int x, int y)
{
    m_dev->draw_text(text, x, y);
}

void RenderSystem::end_frame()
{
    // finished drawing the queued objects
    m_queue.clear();

    m_dev->swap_buffers();
}


