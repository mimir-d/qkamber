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
#include "camera.h"
#include "viewport.h"
#include "mesh.h"
#include "input_system.h"

class MyApplication : public Application
{
public:
    void on_create() override;
    void on_resize(int width, int height) override;

    void update(float abs_time, float elapsed_time) override;
    void render(float abs_time, float elapsed_time) override;

private:
    FpsCamera m_camera { { 0, 0, 10 } };

    Viewport m_viewport;

    unique_ptr<Mesh> m_mesh;
    mat4 m_world_matrix[3];
};

void MyApplication::on_create()
{
    m_renderer.set_camera(&m_camera);
    m_renderer.set_viewport(&m_viewport);

    auto& dev = m_renderer.get_device();
    dev.set_polygon_mode(PolygonMode::Line);

    m_mesh = make_unique<Mesh>(dev);
}

void MyApplication::on_resize(int width, int height)
{
    m_camera.set_proj_params(width, height);
    m_viewport.set_params(width, height);
}

void MyApplication::update(float abs_time, float elapsed_time)
{
    m_camera.update(abs_time, elapsed_time);

    // TODO: check paren identation formatting
    // scene stuff

    auto& q = m_renderer.get_queue();
    for (int i = 0; i < 3; i++)
    {
        m_world_matrix[i] = mat4::translate(
            (i - 1) * 3.5f,
            0,
            0
        );
        m_world_matrix[i] *= mat4::rotate(
            (i+1) * abs_time,
            (i+1) * abs_time,
            (i+1) * abs_time
        );
        //m_world_matrix[i] *= mat4::rotate(0.5, 0, 0);
        m_world_matrix[i] *= mat4::scale(1.0f, 1.0f, 1.0f);

        q.add(m_world_matrix[i], *m_mesh);
    }
    // m_scene.update();
}

void MyApplication::render(float abs_time, float elapsed_time)
{
    // m_scene.render();

    m_renderer.render();
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
// TODO: log dates with time and move filename to left + log cat