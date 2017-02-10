#pragma once

#include "timer.h"

class Application;

class Engine
{
public:
    Engine();
    ~Engine();

    void run(Application& app);
    int get_exit_code() const;

private:
    Timer m_global_timer;
    int m_exit_code = 0;
};

inline int Engine::get_exit_code() const
{
    return m_exit_code;
}
