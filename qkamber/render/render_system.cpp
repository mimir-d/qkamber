
#include "precompiled.h"
#include "render_system.h"

#include "engine.h"
#include "render_queue.h"
#include "render_primitive.h"
#include "scene/scene_system.h"
#include "scene/camera.h"
#include "scene/viewport.h"
#include "model/mesh.h"
#include "platform.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
RenderSystem::RenderSystem(QkEngine::Context& context) :
    Subsystem(context)
{
    flog("id = %#x", this);
    m_dev = RenderDeviceFactory::create(context);
    log_info("Created render system");
}

RenderSystem::~RenderSystem()
{
    flog();
    log_info("Destroyed render system");
}

void RenderSystem::process()
{
    begin_frame();

    // set camera and viewport
    auto& scene = m_context.get_scene();
    auto& camera = scene.get_camera();
    auto& viewport = scene.get_viewport();

    m_dev->set_view_matrix(camera.get_view());
    m_dev->set_proj_matrix(camera.get_proj());
    m_dev->set_clip_matrix(viewport.get_clip());

    for (auto& qi : m_queue)
    {
        m_dev->set_world_matrix(qi.world_matrix);
        m_dev->draw_primitive(qi.mesh.get_primitive());
    }

    m_context.on_render();
    end_frame();
}

void RenderSystem::begin_frame()
{
    m_dev->clear();
}

void RenderSystem::end_frame()
{
    // finished drawing the queued objects
    m_queue.clear();

    m_dev->swap_buffers();
}


