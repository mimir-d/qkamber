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
    FpsCamera m_camera;
    Viewport m_viewport;

    unique_ptr<Mesh> m_mesh;
    mat4 m_world_matrix[3];
};

void MyApplication::on_create()
{
    m_camera.set_params(
        { 0, 0, 10 },
        { 0, 0, 0 },
        { 0, 1, 0 }
    );
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
    auto& keyboard = InputSystem::get_inst().get_keyboard();
    if (keyboard.get_key_pressed('A'))
        dlog("pressed A");
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