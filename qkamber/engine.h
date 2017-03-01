#pragma once

class TimeSystem;
class EntitySystem;
class RenderSystem;
class SceneSystem;
class InputSystem;
class StatsSystem;

class QkEngine
{
public:
    class Context
    {
    public:
        Context();
        virtual ~Context();

        virtual void on_create() = 0;
        virtual void on_destroy() = 0;
        virtual void on_resize(int width, int height) = 0;

        virtual void on_update() = 0;
        virtual void on_render() = 0;

    public:
        void notify_exit();
        bool get_exit_requested() const;

        TimeSystem& get_time();
        EntitySystem& get_entity();
        RenderSystem& get_render();
        SceneSystem& get_scene();
        InputSystem& get_input();
        StatsSystem& get_stats();

    private:
        // TODO: context will be the keeper of systems
        std::unique_ptr<TimeSystem> m_time;
        std::unique_ptr<EntitySystem> m_entity;
        std::unique_ptr<RenderSystem> m_render;
        std::unique_ptr<SceneSystem> m_scene;
        std::unique_ptr<InputSystem> m_input;
        std::unique_ptr<StatsSystem> m_stats;

        bool m_exit_requested = false;
    };

public:
    QkEngine(Context& context);
    ~QkEngine();

    int run();

private:
    Context& m_context;
};

class App
{
public:
    App(QkEngine::Context& context);
    virtual ~App() = default;

    virtual int mainloop() = 0;

protected:
    QkEngine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// Engine::Context impl
///////////////////////////////////////////////////////////////////////////////
inline void QkEngine::Context::notify_exit()
{
    m_exit_requested = true;
}

inline bool QkEngine::Context::get_exit_requested() const
{
    return m_exit_requested;
}

inline TimeSystem& QkEngine::Context::get_time()
{
    return *m_time.get();
}

inline EntitySystem& QkEngine::Context::get_entity()
{
    return *m_entity.get();
}

inline RenderSystem& QkEngine::Context::get_render()
{
    return *m_render.get();
}

inline SceneSystem& QkEngine::Context::get_scene()
{
    return *m_scene.get();
}

inline InputSystem& QkEngine::Context::get_input()
{
    return *m_input.get();
}

inline StatsSystem& QkEngine::Context::get_stats()
{
    return *m_stats.get();
}

///////////////////////////////////////////////////////////////////////////////
// Engine impl
///////////////////////////////////////////////////////////////////////////////
inline QkEngine::QkEngine(Context& context) :
    m_context(context)
{
    flog();
    log_info("Created Qkamber engine instance");
}

inline QkEngine::~QkEngine()
{
    flog();
    log_info("Destroyed engine instance");
}

///////////////////////////////////////////////////////////////////////////////
// App impl
///////////////////////////////////////////////////////////////////////////////
inline App::App(QkEngine::Context& context) :
    m_context(context)
{}
