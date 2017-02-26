#pragma once

class TimeSystem
{
public:
	TimeSystem();
    ~TimeSystem();

	void resume();
	void stop();

	float get_abs_time() const;
    // TODO: this should be done once a frame when this is turned into a system
	float get_diff_time();

private:
	bool m_running = false;
    float m_total_abs = 0;
    app_clock::time_point m_last_abs;
    app_clock::time_point m_last_diff;
};

inline TimeSystem::TimeSystem()
{
    flog("id = %#x", this);
    log_info("Created timer");
}

inline TimeSystem::~TimeSystem()
{
    flog();
    log_info("Destroyed timer");
}