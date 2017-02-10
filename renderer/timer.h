#pragma once

class Timer
{
public:
	Timer();

	void start();
	void stop();

	float get_abs_time();
	float get_diff_time();

private:
	bool m_running;
    // TODO: use std::chrono
	LARGE_INTEGER m_start_time, m_last_count;
	LARGE_INTEGER m_freq;
};