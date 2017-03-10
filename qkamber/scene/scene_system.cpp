
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
        const mat4& get_view() const;
        const mat4& get_proj() const;
    };

    class NullViewport : public Viewport
    {
    public:
        const mat3x4& get_clip() const final;
    };

    inline const mat4& NullCamera::get_view() const
    {
        throw std::exception("Attempted to use null camera");
    }

    inline const mat4& NullCamera::get_proj() const
    {
        throw std::exception("Attempted to use null camera");
    }

    inline const mat3x4& NullViewport::get_clip() const
    {
        throw std::exception("Attempted to use null viewport");
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
    auto& q = m_context.get_render().get_queue();
    // TODO: move queue clear here (wm_paint doesnt need to call update anymore)

    for (auto& agg : m_context.get_entity().filter_comp<SrtComponent, ModelComponent>())
    {
        // NOTE: maybe have const srt and compute the world matrix in entity_system.process
        auto& srt = std::get<0>(agg);
        auto& model = std::get<1>(agg);

        // TODO: add any visibility algorithms here
        for (auto& unit : model.get_model()->get_units())
            q.add(srt.get_world(), unit);
    }
}
