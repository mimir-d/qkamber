#pragma once

class TimeSystem;
class RenderSystem;
class InputSystem;

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
        RenderSystem& get_render();
        InputSystem& get_input();

    private:
        // TODO: context will be the keeper of systems
        std::unique_ptr<TimeSystem> m_time;
        std::unique_ptr<RenderSystem> m_render;
        std::unique_ptr<InputSystem> m_input;

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

inline RenderSystem& QkEngine::Context::get_render()
{
    return *m_render.get();
}

inline InputSystem& QkEngine::Context::get_input()
{
    return *m_input.get();
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
