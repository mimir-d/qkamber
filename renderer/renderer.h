#pragma once

#include "camera.h"
#include "viewport.h"
#include "timer.h"

class RenderDevice
{
public:
    virtual void draw_line(float x0, float y0, float x1, float y1) = 0;
    virtual void draw_tri(float x0, float y0, float x1, float y1, float x2, float y2) = 0;
    virtual void draw_text(const std::string& text, float x, float y) = 0;

    virtual void clear() = 0;
    virtual void swap_buffers() = 0;
};

class Renderer
{
public:
	void init(Timer* timer);
	void shutdown();

    void render();

    RenderDevice& get_device();

    float get_fps() const;

private:
    void begin_frame();
    void end_frame();

protected:
    std::unique_ptr<RenderDevice> m_dev;

    //TEMP:
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Viewport> m_viewport;

	Timer* m_timer;
    uint64_t m_frame_number;

    float m_target_fps, m_fps;
    uint32_t m_fps_last_count;
    float m_fps_last_timestamp;
};

class RenderDeviceFactory
{
public:
    static std::unique_ptr<RenderDevice> create();
};

///////////////////////////////////////////////////////////////////////////////
// Renderer impl
///////////////////////////////////////////////////////////////////////////////
inline RenderDevice& Renderer::get_device()
{
    return *m_dev;
}

inline float Renderer::get_fps() const
{
    return m_fps;
}