#pragma once

class Renderer
{
public:
	void init();
	void shutdown();

	void begin_frame();
	void end_frame();

	virtual void update(float elapsed_time) = 0;
	virtual void draw(Gdiplus::Graphics& g, float elapsed_time) = 0;
};