#pragma once

#include "engine.h"
#include "subsystem.h"

class Camera;
class Viewport;

class SceneSystem : public Subsystem
{
public:
    SceneSystem(QkEngine::Context& context);
    ~SceneSystem();

    void process() final;

    void set_camera(Camera* camera);
    const Camera& get_camera() const;

    void set_viewport(Viewport* viewport);
    const Viewport& get_viewport() const;

private:
    Camera* m_camera;
    Viewport* m_viewport;

    std::unique_ptr<Camera> m_null_camera;
    std::unique_ptr<Viewport> m_null_viewport;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void SceneSystem::set_camera(Camera* camera)
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

inline const Camera& SceneSystem::get_camera() const
{
    return *m_camera;
}

inline void SceneSystem::set_viewport(Viewport* viewport)
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

inline const Viewport& SceneSystem::get_viewport() const
{
    return *m_viewport;
}