#pragma once

class Renderer;

class AppWindow
{
public:
	virtual void init(std::unique_ptr<Renderer> renderer);
	virtual void mainloop();
	virtual int shutdown();

	// TODO: rename
	virtual void OnKeyPressed(int key_code) = 0;
	// TODO: extract Graphics interface
	virtual void OnPaint(Gdiplus::Graphics& g) = 0;

protected:
	std::unique_ptr<Renderer> m_renderer;
};

class AppWindowFactory
{
public:
	static std::unique_ptr<AppWindow> create();
};

