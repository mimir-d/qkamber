#pragma once

class Timer;
class Application;

class Window
{
public:
    // TODO: move these to ctor (create platform.h, copy all factories there and template
	virtual void init(Application* app, Timer* timer);
	virtual void mainloop();
	virtual int shutdown();

protected:
    Application* m_app;
    Timer* m_timer;
};

class AppWindowFactory
{
public:
	static std::unique_ptr<Window> create();
};
