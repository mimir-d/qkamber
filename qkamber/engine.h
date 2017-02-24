#pragma once

class Timer;
class Renderer;
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
        virtual void on_resize(int width, int height) = 0;

        virtual void on_update(float abs_time, float elapsed_time) = 0;
        virtual void on_render(float abs_time, float elapsed_time) = 0;

    public:
        Timer& get_timer();
        Renderer& get_renderer();
        InputSystem& get_input();

    private:
        // TODO: context will be the keeper of systems
        std::unique_ptr<Timer> m_timer;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<InputSystem> m_input;
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
inline Timer& QkEngine::Context::get_timer()
{
    return *m_timer.get();
}

inline Renderer& QkEngine::Context::get_renderer()
{
    return *m_renderer.get();
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
