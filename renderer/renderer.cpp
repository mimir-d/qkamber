
#include "stdafx.h"
#include "renderer.h"
using namespace std;

void Renderer::init()
{
	flog();
	m_timer.start();
}
void Renderer::shutdown()
{
	flog();
	m_timer.stop();
}

void Renderer::begin_frame()
{
}

void Renderer::end_frame()
{
	//float elapsed = 
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