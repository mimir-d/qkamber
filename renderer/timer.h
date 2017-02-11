#pragma once

class Timer
{
public:
	Timer();
    ~Timer() = default;

	void resume();
	void stop();

	float get_abs_time();
	float get_diff_time();

private:
	bool m_running = false;
    app_clock::time_point m_last = app_clock::now();
};

inline Timer::Timer()
{
    flog("id = %#x", this);
}