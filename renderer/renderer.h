#pragma once

#include "timer.h"

class Renderer
{
public:
	void init();
	void shutdown();

	void begin_frame();
	void end_frame();

	void on_update();
	void on_draw(Gdiplus::Graphics& g);

	virtual void update(float abs_time, float elapsed_time) = 0;
	virtual void draw(Gdiplus::Graphics& g, float abs_time, float elapsed_time) = 0;

    float get_fps() const;

protected:
	Timer m_timer;
    uint64_t m_frame_number;
    
    float m_fps;
    uint32_t m_fps_last_count;
    float m_fps_last_timestamp;
};