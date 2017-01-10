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
    Font font(L"TimesNewRoman", 12);
    SolidBrush hb(Color(255, 0, 0));

    wostringstream ostr;
    ostr << "fps: " << fixed << setprecision(2) << get_fps() << ends;
    wstring fps_string = ostr.str();
    g.DrawString(fps_string.c_str(), fps_string.size(), &font, PointF(3, 3), &hb);

	Pen p(Color(255, 255, 0, 255));
	g.DrawLine(&p, 100.f, 100.f, 100.f + 50.f * sin(abs_time), 100.f + 50.f * cos(abs_time));

	g.FillRectangle(&hb, 150 + 50*sin(abs_time), 150 + 50*cos(abs_time/2), 350 - 50*cos(abs_time), 250-50*sin(abs_time/2));
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
// peekmessage for draw loop