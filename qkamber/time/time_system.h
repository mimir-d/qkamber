#pragma once

#include "subsystem.h"

class TimeSystem : public Subsystem
{
public:
	TimeSystem(QkEngine::Context& context);
    ~TimeSystem();

    void process() final;

public:
	void resume();
	void stop();

	float get_abs_time() const;
	float get_diff_time() const;

private:
	bool m_running = false;
    float m_total_abs = 0;
    float m_frame_time = 0;
    app_clock::time_point m_last_abs;
    app_clock::time_point m_last_diff;
};

inline TimeSystem::TimeSystem(QkEngine::Context& context) :
    Subsystem(context)
{
    flog("id = %#x", this);
    log_info("Created timer");
}

inline TimeSystem::~TimeSystem()
{
    flog();
    log_info("Destroyed timer");
}