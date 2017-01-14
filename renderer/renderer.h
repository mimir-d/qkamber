#pragma once

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
	void init(RenderDevice* device);
	void shutdown();

	void begin_frame();
	void end_frame();

	void on_update();
	void on_draw();

	virtual void update(float abs_time, float elapsed_time) = 0;
	virtual void draw(float abs_time, float elapsed_time) = 0;

    float get_fps() const;

protected:
    RenderDevice* m_dev;

	Timer m_timer;
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
