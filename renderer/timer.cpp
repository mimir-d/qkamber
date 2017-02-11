
#include "stdafx.h"
#include "timer.h"

using namespace std::chrono;

void Timer::resume()
{
	if (m_running)
		return;

    m_last = app_clock::now();
	m_running = true;
	dlog("Timer resumed");
}

void Timer::stop()
{
	m_running = false;
	dlog("Timer stopped");
}

float Timer::get_abs_time()
{
    // TODO: dont flow abs time if timer is stopped
    auto microsec = duration_cast<microseconds>(app_clock::now().time_since_epoch());
    return static_cast<float>(microsec.count() * 1e-6);
}

float Timer::get_diff_time()
{
    auto now = app_clock::now();
    auto microsec = duration_cast<microseconds>(now - m_last);
    m_last = now;
    return static_cast<float>(microsec.count() * 1e-6);
}

