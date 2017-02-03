// renderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace Gdiplus;

#include "engine.h"
#include "app.h"
#include "window.h"
#include "renderer.h"
#include "timer.h"
#include "math3.h"
#include "mesh.h"

class MyApplication : public Application
{
public:
    void update(float abs_time, float elapsed_time) override;
    void render(float abs_time, float elapsed_time) override;

private:
    Mesh m_mesh;
    mat4 m_world_matrix;
};

void MyApplication::update(float abs_time, float elapsed_time)
{
    // TODO: check paren identation formatting
    // scene stuff
    m_world_matrix = mat4::rotate(
        1.0f * abs_time,
        5.8f * abs_time,
        3.0f * abs_time
    ) * mat4::scale(
        3.0f + abs(4.0f * sin(11.6f * abs_time)),
        3.0f + abs(4.0f * cos(11.6f * abs_time)),
        1.0f
    );

    auto& q = m_renderer.get_queue();
    q.add(m_world_matrix, m_mesh);

    // m_scene.update();
}

void MyApplication::render(float abs_time, float elapsed_time)
{
    // m_scene.render();

    m_renderer.render();




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

    Engine engine;
    try
    {
        MyApplication app;
        engine.run(app);

        //cout << endl << "Press enter to continue ..." << endl;
        //cin.get();
        return engine.get_exit_code();
    }
    catch (exception& ex)
    {
        cout << "Exception caught: " << ex.what() << endl;
        return 1;
    }
}

// TODO:
// refactoir happlication
// vertex buffer (vec3) + decl + model + index buffer
// inherit app and override update/render, renderer is created by app
// zbuffer/scanline-tri; display lists + sorting
// scene tree
//
#include "mesh.h"
int mainx()
{
    Mesh m;

    auto p = m.get_primitive();
    float x = *reinterpret_cast<float*>(p.vertices.lock());
    x = *reinterpret_cast<float*>(p.vertices.lock() + 12);
    return 0;
}
/*
renderer draws primitives
    primitive has a vdata and idata
>> drawing:
>> go thru decl and get vecref ptrs

model has model units
    model unit has a mesh and material, local transform
        mesh has a vertex data and index data, get primitive

vdata has vertex buffer + decl
idata has index buffer

decl has decl elements
    elem has offset, size, semantic

scene has scene nodes
    scene node maybe has a model + world transform
    */

// scene has a renderer ref
// find visible objects, put in render queue