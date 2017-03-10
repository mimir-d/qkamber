// renderer.cpp : Defines the entry point for the console application.
//

#include "precompiled.h"

#include "engine.h"
#include "math3.h"
#include "render/render_system.h"
#include "scene/scene_system.h"
#include "input/input_system.h"
#include "input/input_device.h"
#include "scene/camera.h"
#include "scene/viewport.h"
#include "render/model.h"
#include "stats/stats_system.h"
#include "time/time_system.h"
#include "entity/entity_system.h"
#include "asset/asset_system.h"

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
    RectViewport m_viewport;

    unique_ptr<Model> m_model[9];
    std::unique_ptr<EntitySystem::Entity> m_ent[9];
};

void Context::on_create()
{
    auto& scene = get_scene();
    scene.set_camera(&m_camera);
    scene.set_viewport(&m_viewport);

    // initialize a render target for the app
    auto& dev = get_render().get_device();
    m_target = dev.create_render_target(640, 480);

    dev.set_render_target(m_target.get());
    dev.set_polygon_mode(m_poly_mode);

    auto& entity = get_entity();
    for (int i = 0; i < 9; i++)
    {
        m_model[i] = make_unique<Model>(dev, get_asset(), i % 3 == 0 ? "tex0.bmp" : i % 3 == 1 ? "tex3.bmp" : "tex4.bmp");

        m_ent[i] = entity.create_entity();

        auto& srt = m_ent[i]->add_component<SrtComponent>();
        srt.set_position({ (i/3 - 1) * 3.5f, (i%3 - 1) * 3.5f, 0 });

        auto& model = m_ent[i]->add_component<ModelComponent>();
        model.set_model(m_model[i].get());
    }
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

    for (int i = 0; i < 0; i++)
    {
        auto& srt = m_ent[i]->get_component<SrtComponent>();
        srt.set_rotation(vec3{ 1, 1, 1 } * ((i / 3 + i % 3 + 1) * abs_time));
    }
}

void Context::on_render()
{
    auto& dev = get_render().get_device();

    int y = 0;
    auto& stats = get_stats();
    dev.draw_text(print_fmt("fps: %0.2f, frame count = %05u", stats.get_fps(), stats.get_frame_number()), 3, y += 3);

    vec3 p = m_camera.get_position();
    vec2 r = m_camera.get_rotation() * (180.0f / PI);
    dev.draw_text(print_fmt("cam pos = %.4f %.4f %.4f", p.x(), p.y(), p.z()), 3, y += 10);
    dev.draw_text(print_fmt("cam rot = %.4f %.4f", r.x(), -r.y()), 3, y += 10);
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
