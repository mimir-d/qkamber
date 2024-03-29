
#include "precompiled.h"
#include "scene_system.h"

#include "camera.h"
#include "viewport.h"

#include "render/render_system.h"
#include "render/model.h"
#include "entity/entity_system.h"
#include "entity/srt_component.h"
#include "entity/model_component.h"

namespace
{
    class NullCamera : public Camera
    {
    public:
        const mat4& get_view() const final;
        const mat4& get_proj() const final;
        const mat4& get_view_inv() const final;
    };

    class NullViewport : public Viewport
    {
    public:
        const mat3x4& get_clip() const final;
    };

    inline const mat4& NullCamera::get_view() const
    {
        throw std::runtime_error("Attempted to use null camera");
    }

    inline const mat4& NullCamera::get_proj() const
    {
        throw std::runtime_error("Attempted to use null camera");
    }

    inline const mat4& NullCamera::get_view_inv() const
    {
        throw std::runtime_error("Attempted to use null camera");
    }

    inline const mat3x4& NullViewport::get_clip() const
    {
        throw std::runtime_error("Attempted to use null viewport");
    }
}

SceneSystem::SceneSystem(QkEngine::Context& context) :
    Subsystem(context),
    m_null_camera(new NullCamera),
    m_null_viewport(new NullViewport)
{
    flog("id = %#x", this);

    set_camera(nullptr);
    set_viewport(nullptr);
    log_info("Created scene system");
}

SceneSystem::~SceneSystem()
{
    flog();
    log_info("Destroyed scene system");
}

void SceneSystem::process()
{
    auto& render = m_context.get_render();

    // set render state
    auto& p = render.get_device().get_params();
    p.set_view_matrix(m_camera->get_view());
    p.set_proj_matrix(m_camera->get_proj());
    p.set_clip_matrix(m_viewport->get_clip());
    p.set_view_inv_matrix(m_camera->get_view_inv());

    // set lights
    auto& dev = render.get_device();
    for (size_t i = 0; i < dev.get_light_unit_count(); i++)
        dev.set_light_unit(i, i < m_lights.size() ? m_lights[i] : nullptr);

    // add items in render queue
    auto& q = render.get_queue();
    q.clear();

    for (const auto& agg : m_context.get_entity().filter_comp<SrtComponent, ModelComponent>())
    {
        auto& srt = std::get<0>(agg);
        auto& model = std::get<1>(agg);

        // TODO: add any visibility algorithms here
        for (const auto& unit : model.get_model()->get_units())
            q.add(srt.get_world(), srt.get_world_inv(), unit);
    }
}
