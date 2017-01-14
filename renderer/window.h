#pragma once

class Renderer;
class RenderDevice;

class Window
{
public:
	virtual void init(std::unique_ptr<Renderer> renderer);
	virtual void mainloop();
	virtual int shutdown();

protected:
	std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<RenderDevice> m_dev;
    bool m_paused;
};

class AppWindowFactory
{
public:
	static std::unique_ptr<Window> create();
};

