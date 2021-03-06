
#include "precompiled.h"
#include "time_system.h"

using namespace std::chrono;

void TimeSystem::process()
{
    // get duration since last frame and update time_point
    auto now = app_clock::now();
    auto microsec = duration_cast<microseconds>(now - m_last_diff);
    m_last_diff = now;
    m_frame_time = static_cast<float>(microsec.count() * 1e-6);
}

void TimeSystem::resume()
{
	if (m_running)
		return;

    // restart a new absolute period increment and diff increment
    m_last_abs = app_clock::now();
    m_last_diff = app_clock::now();

	m_running = true;
	dlog("Timer resumed");
}

void TimeSystem::stop()
{
    if (!m_running)
        return;

    // append the last running period duration to total
    auto microsec = duration_cast<microseconds>(app_clock::now() - m_last_abs);
    m_total_abs += static_cast<float>(microsec.count() * 1e-6);

    m_running = false;
	dlog("Timer stopped");
}

float TimeSystem::get_abs_time() const
{
    if (!m_running)
        return m_total_abs;

    // get total time since start of last running period and append to total time
    auto microsec = duration_cast<microseconds>(app_clock::now() - m_last_abs);
    return m_total_abs + static_cast<float>(microsec.count() * 1e-6);
}

float TimeSystem::get_diff_time() const
{
    if (!m_running)
        return 0;

    return m_frame_time;
}

