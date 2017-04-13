// renderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace Gdiplus;

#include "app.h"
#include "app_window.h"
#include "renderer.h"
#include "timer.h"

class MyRenderer : public Renderer
{
public:
	void update(float abs_time, float elapsed_time) override;
	void draw(Graphics& g, float abs_time, float elapsed_time) override;
};

void MyRenderer::update(float abs_time, float elapsed_time)
{
}

void MyRenderer::draw(Graphics& g, float abs_time, float elapsed_time)
{
	Pen p(Color(255, 255, 0, 255));
	g.DrawLine(&p, 200.f, 200.f, 200.f + 100.f * sin(abs_time), 200.f + 100.f * cos(abs_time));
}

int main()
{
	flog();
	Application app;
	try
	{
		unique_ptr<Renderer> renderer(new MyRenderer);
		app.init(std::move(renderer));
		app.run();
		int rc = app.shutdown();

		//cout << endl << "Press enter to continue ..." << endl;
		//cin.get();
		return rc;
	}
	catch (exception& ex)
	{
		cout << "Exception caught: " << ex.what() << endl;
		return 1;
	}
}

// TODO:
// logger
// refactoir happlication
// renderer class: beginframe, update<keydown, draw, endframe/frame timer
// math: vec3, vec4, mat4x4, world, view, proj + , blit (double buffering)
// vertex buffer (vec3) + model
