#pragma once

class Window;
class Renderer;

class Application
{
public:
	void init(std::unique_ptr<Renderer> renderer);
	void run();
	int shutdown();

private:
	std::unique_ptr<Window> m_window;
};