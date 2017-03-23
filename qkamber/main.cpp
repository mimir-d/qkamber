// renderer.cpp : Defines the entry point for the console application.
//

#include "precompiled.h"

#include "engine.h"
#include "math3.h"
#include "render/render_system.h"
#include "render/software_device.h"
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

    FpsCamera m_camera { *this, { 6, 6, 23 } };
    RectViewport m_viewport;

    std::unique_ptr<EntitySystem::Entity> m_obj_axis;
    std::unique_ptr<EntitySystem::Entity> m_obj_ship;
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

    auto& entity = get_entity();

    m_obj_axis = entity.create_entity("axis.3ds");
    auto& srt_axis = m_obj_axis->get_component<SrtComponent>();
    srt_axis.set_scale({ 2, 2, 2 });
    srt_axis.set_rotation({ PI/2, 0, 0, });

    m_obj_ship = entity.create_entity("ship.3ds");
    auto& srt_ship = m_obj_ship->get_component<SrtComponent>();
    srt_ship.set_position({ 6, 6, 6 });
    srt_ship.set_scale({ .5, .5, .5 });

    // TODO: temporary debug
    //static_cast<SoftwareDevice&>(dev).debug_normals(true);
    //dev.set_polygon_mode(PolygonMode::Line);
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

    auto& keyboard = get_input().get_keyboard();
    // TODO: should encode button transitions somehow

    auto& dev = get_render().get_device();
    if (keyboard.get_key_pressed('1'))
        dev.set_polygon_mode(PolygonMode::Point);
    else if (keyboard.get_key_pressed('2'))
        dev.set_polygon_mode(PolygonMode::Line);
    else if (keyboard.get_key_pressed('3'))
        dev.set_polygon_mode(PolygonMode::Fill);

    // TODO: translate keys to platform independent
    if (keyboard.get_key_pressed(VK_ESCAPE))
        notify_exit();

    m_camera.update();

    auto& srt = m_obj_ship->get_component<SrtComponent>();
    srt.set_rotation(vec3{ 0.25f, 0.25f, 0.25f } * abs_time);
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
