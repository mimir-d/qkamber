#pragma once

class Timer;
class Renderer;

class Engine
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
        Renderer& get_renderer();
        Timer& get_timer();

    private:
        // TODO: context will be the keeper of systems
        std::unique_ptr<Timer> m_timer;
        std::unique_ptr<Renderer> m_renderer;
    };

public:
    Engine(Context& context);
    ~Engine();

    Context& get_context();
    int run();

private:
    Context& m_context;
};

class App
{
public:
    App(Engine::Context& context);
    virtual ~App() = default;

    virtual int mainloop() = 0;

protected:
    Engine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// Engine::Context impl
///////////////////////////////////////////////////////////////////////////////
inline Renderer& Engine::Context::get_renderer()
{
    return *m_renderer.get();
}

inline Timer& Engine::Context::get_timer()
{
    return *m_timer.get();
}

///////////////////////////////////////////////////////////////////////////////
// Engine impl
///////////////////////////////////////////////////////////////////////////////
inline Engine::Engine(Context& context) :
    m_context(context)
{
    flog();
    log_info("Created Qukamber engine instance");
}

inline Engine::~Engine()
{
    flog();
    log_info("Destroyed engine instance");
}

inline Engine::Context& Engine::get_context()
{
    return m_context;
}

///////////////////////////////////////////////////////////////////////////////
// App impl
///////////////////////////////////////////////////////////////////////////////
inline App::App(Engine::Context& context) :
    m_context(context)
{}
