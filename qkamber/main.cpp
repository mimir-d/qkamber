// renderer.cpp : Defines the entry point for the console application.
//

#include "precompiled.h"

#include "engine.h"
#include "render/render_system.h"
#include "input/input_system.h"
#include "input/input_device.h"
#include "math3.h"
#include "scene/camera.h"
#include "scene/viewport.h"
#include "model/mesh.h"

using namespace std;

class Context : public QkEngine::Context
{
public:
    void on_create() final;
    void on_destroy() final;
    void on_resize(int width, int height) final;

    void on_update() final;
    void on_render() final;

private:
    std::unique_ptr<RenderTarget> m_target;
    PolygonMode m_poly_mode = PolygonMode::Fill;
    bool m_poly_mode_changed = false;

    FpsCamera m_camera { *this, { 0, 0, 15 } };

    Viewport m_viewport;

    unique_ptr<Mesh> m_mesh;
    mat4 m_world_matrix[3][3];
};

void Context::on_create()
{
    auto& render = get_render();
    render.set_camera(&m_camera);
    render.set_viewport(&m_viewport);

    // initialize a render target for the app
    auto& dev = render.get_device();
    m_target = dev.create_render_target(640, 480);

    dev.set_render_target(m_target.get());
    dev.set_polygon_mode(m_poly_mode);

    m_mesh = make_unique<Mesh>(dev);
}

void Context::on_destroy()
{
    get_render().get_device().set_render_target(nullptr);
    m_target = nullptr;
}

void Context::on_resize(int width, int height)
{
    m_camera.set_proj_params(width, height);
    m_viewport.set_params(width, height);
}

void Context::on_update()
{
    auto& time = get_time();
    float abs_time = time.get_abs_time();

    auto& render = get_render();
    auto& mouse = get_input().get_mouse();
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
            render.get_device().set_polygon_mode(m_poly_mode);

            m_poly_mode_changed = true;
        }
    }
    else
    {
        m_poly_mode_changed = false;
    }

    auto& keyboard = get_input().get_keyboard();
    // TODO: translate keys to platform independent
    if (keyboard.get_key_pressed(VK_ESCAPE))
        notify_exit();

    m_camera.update();

    // TODO: check paren identation formatting
    // scene stuff

    auto& q = render.get_queue();
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

void Context::on_render()
{
    auto& render = get_render();
    // m_scene.render();

    // TODO: this should not be public here
    render.begin_frame();
    render.render();

    vec3 p = m_camera.get_position();
    vec2 r = m_camera.get_rotation() * (180.0f / PI);
    render.render_text(print_fmt("cam pos = %.4f %.4f %.4f", p.x(), p.y(), p.z()), 3, 13);
    render.render_text(print_fmt("cam rot = %.4f %.4f", r.x(), -r.y()), 3, 23);

    render.end_frame();
}

int main()
{
    try
    {
        Context ctx;
        QkEngine(ctx).run();
    }
    catch (exception& ex)
    {
        cout << "Exception caught: " << ex.what() << endl;
        return 1;
    }
}

// TODO:
// refactoir happlication
// display lists + sorting
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
