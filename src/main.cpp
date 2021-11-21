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
#include "scene/light.h"
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
    Light m_light;

    vector<unique_ptr<EntitySystem::Entity>> m_objects;
};

void Context::on_create()
{
    auto& scene = get_scene();
    scene.set_camera(&m_camera);
    scene.set_viewport(&m_viewport);

    m_light.set_position({ 10, 20, 20, 1 });
    m_light.set_ambient({ 0.2, 0.2, 0.2, 1.0 });
    m_light.set_diffuse({ 1.0, 1.0, 1.0, 1.0 });
    m_light.set_specular({ 0.7, 0.7, 0.7, 1.0 });
    m_light.set_attenuation({ 0.0f, 0.0f, 0.0005f, 0.0025f });
    scene.set_lights({ &m_light });

    // initialize a render target for the app
    auto& dev = get_render().get_device();
    m_target = dev.create_render_target(640, 480);
    dev.set_render_target(m_target.get());

    auto& entity = get_entity();

    const string names[] =
    {
        "axis.3ds",
        "ship.3ds",
        ":prefab/color_cube",
        ":prefab/tex_cube",
        ":prefab/color_cube",
        ":prefab/tex_cube"
    };

    const vec3 positions[] =
    {
        { 0, 0, 0 },
        { 6, 6, 6 },
        { -8, 8, 6 },
        { -8, 4, 6 },
        { -4, 8, 6 },
        { -4, 4, 6 }
    };

    for (int i = 0; i < 6; i++)
    {
        m_objects.push_back(entity.create_entity(names[i]));
        auto& srt = m_objects[i]->get_component<SrtComponent>();
        srt.set_position(positions[i]);
    }

    auto& srt_axis = m_objects[0]->get_component<SrtComponent>();
    srt_axis.set_scale({ 2, 2, 2 });
    srt_axis.set_rotation({ PI/2, 0, 0, });

    auto& srt_ship = m_objects[1]->get_component<SrtComponent>();
    srt_ship.set_scale({ .5, .5, .5 });

    dev.set_polygon_mode(PolygonMode::Line);
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

    auto& dev = get_render().get_device();
    if (keyboard.get_key_pressed('1'))
        dev.set_polygon_mode(PolygonMode::Point);
    else if (keyboard.get_key_pressed('2'))
        dev.set_polygon_mode(PolygonMode::Line);
    else if (keyboard.get_key_pressed('3'))
        dev.set_polygon_mode(PolygonMode::Fill);
    else if (keyboard.get_key_pressed('4'))
        static_cast<SoftwareDevice&>(dev).debug_normals(true);
    else if (keyboard.get_key_pressed('5'))
        static_cast<SoftwareDevice&>(dev).debug_normals(false);

    // TODO: translate keys to platform independent
    if (keyboard.get_key_pressed(KEY_ESCAPE))
        notify_exit();

    m_camera.update();

    size_t rotate_indices[] = { 1, 4, 5 };
    for (size_t i : rotate_indices)
    {
        auto& srt = m_objects[i]->get_component<SrtComponent>();
        srt.set_rotation(vec3{ 0.25f, 0.25f, 0.25f } * abs_time);
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
// triange display lists/parallel? + sorting
// scene tree
