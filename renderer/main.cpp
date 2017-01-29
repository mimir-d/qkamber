// renderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace Gdiplus;

#include "app.h"
#include "window.h"
#include "renderer.h"
#include "timer.h"
#include "math3.h"

class MyRenderer : public Renderer
{
public:
	void update(float abs_time, float elapsed_time) override;
	void render(float abs_time, float elapsed_time) override;
};

void MyRenderer::update(float abs_time, float elapsed_time)
{
}

void MyRenderer::render(float abs_time, float elapsed_time)
{
    mat4 world = mat4::rotate(
        0*1.0f * abs_time,
        5.8f * abs_time,
        0*3.0f * abs_time
    ) * mat4::scale(3 + abs(4*sin(11.6*abs_time)), 3 + abs(4*cos(11.6*abs_time)), 1);
    mat4 view = mat4::lookat(
        vec3(0, 0, 10),
        vec3(0, 0, 0),
        vec3(0, 1, 0)
    );
    mat4 proj = mat4::proj_perspective(3.14f / 2, 1.333f, 0.01f, 100.0f);
    mat<float, 3, 4> clip = mat4::clip(0, 480, 640, -480, 0, 1);
    mat4 wvp = proj * view * world;

    vec4 v0(1, -0.5, 0, 1);
    vec4 v1(0, 0.5, 0, 1);
    vec4 v2(-1, -0.5, 0, 1);

    v0 = wvp * v0;
    v0 *= 1.0f / v0.w();

    v1 = wvp * v1;
    v1 *= 1.0f / v1.w();

    v2 = wvp * v2;
    v2 *= 1.0f / v2.w();

    vec3 sv0 = clip * v0;
    vec3 sv1 = clip * v1;
    vec3 sv2 = clip * v2;

    m_dev->draw_tri(sv0.x(), sv0.y(), sv1.x(), sv1.y(), sv2.x(), sv2.y());

    //m_dev->draw_line(100.f, 100.f, 100.f + 50.f * sin(abs_time*2), 100.f + 50.f * cos(abs_time*2));
    //m_dev->draw_tri(
    //    150 + 50*sin(abs_time*2), 150 + 150*cos(abs_time),
    //    150 + 50*sin(abs_time*2), 250-50*sin(abs_time),
    //    350 - 150*cos(abs_time*2), 150 + 150*cos(abs_time)
    //);
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
// inherit app and override update/render, renderer is created by app
// zbuffer/scanline-tri; display lists + sorting
//
// 
int mainx()
{
    return 0;
}