
#include "stdafx.h"
#include "renderer.h"
#include "win32/win32_render_device.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Renderer
///////////////////////////////////////////////////////////////////////////////
void Renderer::init(RenderDevice* device)
{
	flog();
    m_dev = device;

    m_frame_number = 0;
    m_fps_last_count = 0;
    m_fps_last_timestamp = 0;
    
    m_target_fps = 60;
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
    m_dev->clear();
}

void Renderer::end_frame()
{
    ostringstream ostr;
    ostr << "fps: " << fixed << setprecision(2) << m_fps << ends;
    m_dev->draw_text(ostr.str(), 3, 3);

    // count fps
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

    m_dev->swap_buffers();
    // delay for fps
}

void Renderer::on_update()
{
	update(m_timer.get_abs_time(), m_timer.get_diff_time());
}
void Renderer::on_render()
{
	render(m_timer.get_abs_time(), m_timer.get_diff_time());
}

float Renderer::get_fps() const
{
    return m_fps;
}

///////////////////////////////////////////////////////////////////////////////
// RenderDeviceFactory
///////////////////////////////////////////////////////////////////////////////
unique_ptr<RenderDevice> RenderDeviceFactory::create()
{
#ifdef WIN32
    dlog("RenderDeviceFactory creating a Win32RenderDevice...");
    return unique_ptr<RenderDevice>(new Win32RenderDevice);
#endif
    return nullptr;
}