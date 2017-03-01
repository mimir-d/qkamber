#pragma once

#include "engine.h"
#include "subsystem.h"

class StatsSystem : public Subsystem
{
public:
    StatsSystem(QkEngine::Context& context);
    ~StatsSystem();

    void process() final;

public:
    uint64_t get_frame_number() const;
    float get_fps() const;

private:
    uint64_t m_frame_number = 0;

    //float m_target_fps = 60;
    float m_fps = 0;
    uint32_t m_fps_last_count = 0;
    float m_fps_last_timestamp = 0;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline uint64_t StatsSystem::get_frame_number() const
{
    return m_frame_number;
}

inline float StatsSystem::get_fps() const
{
    return m_fps;
}
