
#include "stdafx.h"
#include "timer.h"

using namespace std::chrono;

void Timer::resume()
{
	if (m_running)
		return;

    // restart a new absolute period increment and diff increment
    m_last_abs = app_clock::now();
    m_last_diff = app_clock::now();

	m_running = true;
	dlog("Timer resumed");
}

void Timer::stop()
{
    if (!m_running)
        return;

    // append the last running period duration to total
    auto microsec = duration_cast<microseconds>(app_clock::now() - m_last_abs);
    m_total_abs += static_cast<float>(microsec.count() * 1e-6);

    m_running = false;
	dlog("Timer stopped");
}

float Timer::get_abs_time() const
{
    if (!m_running)
        return m_total_abs;

    // get total time since start of last running period and append to total time
    auto microsec = duration_cast<microseconds>(app_clock::now() - m_last_abs);
    return m_total_abs + static_cast<float>(microsec.count() * 1e-6);
}

float Timer::get_diff_time()
{
    if (!m_running)
        return 0;

    // get duration since last diff call and update time_point
    auto now = app_clock::now();
    auto microsec = duration_cast<microseconds>(now - m_last_diff);
    m_last_diff = now;

    return static_cast<float>(microsec.count() * 1e-6);
}

