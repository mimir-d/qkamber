#pragma once

class Timer
{
public:
	Timer();
    ~Timer() = default;

	void resume();
	void stop();

	float get_abs_time() const;
	float get_diff_time();

private:
	bool m_running = false;
    float m_total_abs = 0;
    app_clock::time_point m_last_abs;
    app_clock::time_point m_last_diff;
};

inline Timer::Timer()
{
    flog("id = %#x", this);
}