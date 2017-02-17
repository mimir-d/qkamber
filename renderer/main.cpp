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
    PolygonMode m_poly_mode = PolygonMode::Fill;
    bool m_poly_mode_changed = false;

    FpsCamera m_camera { { 0, 0, 15 } };

    Viewport m_viewport;

    unique_ptr<Mesh> m_mesh;
    mat4 m_world_matrix[3][3];
};

void MyApplication::on_create()
{
    m_renderer.set_camera(&m_camera);
    m_renderer.set_viewport(&m_viewport);

    auto& dev = m_renderer.get_device();
    dev.set_polygon_mode(m_poly_mode);

    m_mesh = make_unique<Mesh>(dev);
}

void MyApplication::on_resize(int width, int height)
{
    m_camera.set_proj_params(width, height);
    m_viewport.set_params(width, height);
}

void MyApplication::update(float abs_time, float elapsed_time)
{
    auto& mouse = InputSystem::get_inst().get_mouse();
    // TODO: should encode button transitions somehow
    if (mouse.get_button_pressed(MouseDevice::RMB))
    {
        if (!m_poly_mode_changed)
        {
            switch (m_poly_mode)
            {
                case PolygonMode::Point: m_poly_mode = PolygonMode::Line; break;
                case PolygonMode::Line:  m_poly_mode = PolygonMode::Fill; break;
                case PolygonMode::Fill:  m_poly_mode = PolygonMode::Point; break;
            }
            m_renderer.get_device().set_polygon_mode(m_poly_mode);

            m_poly_mode_changed = true;
        }
    }
    else
    {
        m_poly_mode_changed = false;
    }

    m_camera.update(abs_time, elapsed_time);

    // TODO: check paren identation formatting
    // scene stuff

    auto& q = m_renderer.get_queue();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_world_matrix[i][j] = mat4::translate(
                (i - 1) * 3.5f,
                (j - 1) * 3.5f,
                0
            );
            //m_world_matrix[i][j] *= mat4::rotate(
            //    (i + j + 1) * abs_time,
            //    (i + j + 1) * abs_time,
            //    (i + j + 1) * abs_time
            //);
            //m_world_matrix[i] *= mat4::rotate(0.5, 0, 0);
            //m_world_matrix[i][j] *= mat4::scale(1.0f, 1.0f, 1.0f);

            q.add(m_world_matrix[i][j], *m_mesh);
        }
    }
    // m_scene.update();
}

void MyApplication::render(float abs_time, float elapsed_time)
{
    // m_scene.render();

    m_renderer.begin_frame();
    m_renderer.render();

    vec3 p = m_camera.get_position();
    vec2 r = m_camera.get_rotation() * (180.0f / PI);
    m_renderer.render_text(print_fmt("cam pos = %.4f %.4f %.4f", p.x(), p.y(), p.z()), 3, 13);
    m_renderer.render_text(print_fmt("cam rot = %.4f %.4f", r.x(), -r.y()), 3, 23);

    m_renderer.end_frame();
}

int main()
{
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