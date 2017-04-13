#pragma once

class Renderer;

class AppWindow
{
public:
	virtual void init(std::unique_ptr<Renderer> renderer);
	virtual void mainloop();
	virtual int shutdown();

protected:
	std::unique_ptr<Renderer> m_renderer;
};

class AppWindowFactory
{
public:
	static std::unique_ptr<AppWindow> create();
};

