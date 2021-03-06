#pragma once

#include "engine.h"
#include "subsystem.h"

class Camera;
class Viewport;
class Light;

class SceneSystem : public Subsystem
{
public:
    using Lights = std::vector<const Light*>;

public:
    SceneSystem(QkEngine::Context& context);
    ~SceneSystem();

    void process() final;

    void set_camera(const Camera* camera);
    void set_viewport(const Viewport* viewport);
    void set_lights(const Lights& lights);

private:
    const Camera* m_camera;
    const Viewport* m_viewport;
    Lights m_lights;

    std::unique_ptr<Camera> m_null_camera;
    std::unique_ptr<Viewport> m_null_viewport;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void SceneSystem::set_camera(const Camera* camera)
{
    flog();

    if (!camera)
    {
        m_camera = m_null_camera.get();
        log_info("Set camera to null");
        return;
    }

    m_camera = camera;
    log_info("Set camera id = %#x", camera);
}

inline void SceneSystem::set_viewport(const Viewport* viewport)
{
    flog();

    if (!viewport)
    {
        m_viewport = m_null_viewport.get();
        log_info("Set viewport to null");
        return;
    }

    m_viewport = viewport;
    log_info("Set viewport id = %#x", viewport);
}

inline void SceneSystem::set_lights(const Lights& lights)
{
    m_lights = lights;
}
