
#include "stdafx.h"
#include "timer.h"

Timer::Timer() :
	m_running(false)
{
	m_start_time.QuadPart = 0;
	m_last_count.QuadPart = 0;
	QueryPerformanceFrequency(&m_freq);
}

void Timer::start()
{
	if (m_running)
		return;

	QueryPerformanceCounter(&m_start_time);
	m_last_count = m_start_time;
	m_running = true;
}

void Timer::stop()
{
	m_running = false;
}

float Timer::get_abs_time()
{
	LARGE_INTEGER now, elapsed;
	QueryPerformanceCounter(&now);

	elapsed.QuadPart = (now.QuadPart - m_start_time.QuadPart) * 1000000;
	elapsed.QuadPart /= m_freq.QuadPart;
	return static_cast<float>(elapsed.QuadPart * 1e-6);
}

float Timer::get_diff_time()
{
	LARGE_INTEGER prev = m_last_count, elapsed;
	QueryPerformanceCounter(&m_last_count);

	elapsed.QuadPart = (m_last_count.QuadPart - prev.QuadPart) * 1000000;
	elapsed.QuadPart /= m_freq.QuadPart;
	return static_cast<float>(elapsed.QuadPart * 1e-6);
}