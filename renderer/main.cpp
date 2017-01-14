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
	void draw(float abs_time, float elapsed_time) override;
};

void MyRenderer::update(float abs_time, float elapsed_time)
{
}

void MyRenderer::draw(float abs_time, float elapsed_time)
{
    m_dev->draw_line(100.f, 100.f, 100.f + 50.f * sin(abs_time*2), 100.f + 50.f * cos(abs_time*2));
    m_dev->draw_tri(
        150 + 50*sin(abs_time*2), 150 + 150*cos(abs_time),
        150 + 50*sin(abs_time*2), 250-50*sin(abs_time),
        350 - 150*cos(abs_time*2), 150 + 150*cos(abs_time)
    );
//     Font font(L"TimesNewRoman", 12);
//     SolidBrush hb(Color(150, 0, 200));
// 
//     wostringstream ostr;
//     ostr << "fps: " << fixed << setprecision(2) << get_fps() << ends;
//     wstring fps_string = ostr.str();
//     g.DrawString(fps_string.c_str(), fps_string.size(), &font, PointF(3, 3), &hb);
// 
// 	Pen p(Color(255, 150, 0, 255), 4);
// 	g.DrawLine(&p, 100.f, 100.f, 100.f + 50.f * sin(abs_time*2), 100.f + 50.f * cos(abs_time*2));
// 
// 	g.FillRectangle(&hb, 150 + 50*sin(abs_time*2), 150 + 150*cos(abs_time), 350 - 150*cos(abs_time*2), 250-50*sin(abs_time));
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
// refactoir happlication
// math: vec3, vec4, mat4x4, world, view, proj
// vertex buffer (vec3) + model + index buffer
// render device class to pass to on_draw