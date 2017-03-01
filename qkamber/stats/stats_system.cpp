
#include "precompiled.h"
#include "stats_system.h"

#include "time/time_system.h"

StatsSystem::StatsSystem(QkEngine::Context& context) :
    Subsystem(context)
{
    flog("id = %#x", this);
    log_info("Created stats system");
}

StatsSystem::~StatsSystem()
{
    flog();
    log_info("Destroyed stats system");
}

void StatsSystem::process()
{
    float abs_time = m_context.get_time().get_abs_time();

    m_frame_number ++;
    m_fps_last_count ++;

    float delta = abs_time - m_fps_last_timestamp;
    if (delta >= 1.0f)
    {
        m_fps = m_fps_last_count / delta;
        m_fps_last_count = 0;
        m_fps_last_timestamp = abs_time;
    }
}