
#include "stdafx.h"
#include "renderer.h"
using namespace std;

void Renderer::init()
{
	flog();
    m_frame_number = 0;
    m_fps_last_count = 0;
    m_fps_last_timestamp = 0;
    m_fps = 0;

	m_timer.start();
}
void Renderer::shutdown()
{
	flog();
	m_timer.stop();
}

void Renderer::begin_frame()
{
    m_frame_number ++;
    m_fps_last_count ++;

    float now = m_timer.get_abs_time();
    float delta = now - m_fps_last_timestamp;
    if (delta >= 1.0f)
    {
        m_fps = m_fps_last_count / delta;
        m_fps_last_count = 0;
        m_fps_last_timestamp = now;
    }
}

void Renderer::end_frame()
{
}

void Renderer::on_update()
{
	update(
		m_timer.get_abs_time(),
		m_timer.get_diff_time()
	);
}
void Renderer::on_draw(Gdiplus::Graphics& g)
{
	draw(
		g,
		m_timer.get_abs_time(),
		m_timer.get_diff_time()
	);
}

float Renderer::get_fps() const
{
    return m_fps;
}