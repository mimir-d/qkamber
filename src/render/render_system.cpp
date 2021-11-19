
#include "precompiled.h"
#include "render_system.h"

#include "render_queue.h"
#include "render_primitive.h"
#include "mesh.h"
#include "material.h"

#include "scene/scene_system.h"
#include "scene/camera.h"
#include "scene/viewport.h"
#include "engine.h"
#include "platform.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
RenderSystem::RenderSystem(QkEngine::Context& context) :
    Subsystem{ context },
    m_cache{ *this, context.get_asset() }
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
    m_dev->clear();

    auto& p = m_dev->get_params();
    for (auto& qi : m_queue)
    {
        p.set_world_matrix(qi.world_matrix);
        p.set_world_inv_matrix(qi.world_inv_matrix);

        const auto& material = qi.model_unit.get_material();
        p.set_material(material);

        const auto& textures = material.get_textures();
        for (size_t i = 0; i < m_dev->get_texture_unit_count(); i++)
            m_dev->set_texture_unit(i, i < textures.size() ? textures[i] : nullptr);

        m_dev->draw_primitive(qi.model_unit.get_primitive());
    }

    m_context.on_render();
    m_dev->swap_buffers();
}
