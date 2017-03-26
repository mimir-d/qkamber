
#include "precompiled.h"
#include "render_cache.h"

#include "material.h"
#include "mesh.h"
#include "model.h"
#include "render_system.h"
#include "render_buffers.h"

using namespace std;

shared_ptr<Material> RenderCache::get_material(const string& name)
{
    flog();

    return cache_get(m_materials, name, [&]
    {
        string filename = dirname(name);

        // TODO: temporary
        // TODO: material config files, rather than geometry assets
        auto& raw_materials = m_asset.load_geometry(filename)->get_materials();
        auto raw_mat = std::find_if(
            raw_materials.begin(), raw_materials.end(),
            [&](auto& m) { return m.name == name; }
        );

        if (raw_mat == raw_materials.end())
            throw exception(print_fmt("material not found, name = %s", name.c_str()).c_str());
        return make_unique<Material>(*raw_mat, m_render);
    });
}

shared_ptr<Mesh> RenderCache::get_mesh(const std::string& name)
{
    flog();

    return cache_get(m_meshes, name, [&]
    {
        string filename = dirname(name);

        // TODO: temporary
        // TODO: mesh config files, rather than geometry assets
        auto& raw_objects = m_asset.load_geometry(filename)->get_objects();
        auto raw_obj = std::find_if(
            raw_objects.begin(), raw_objects.end(),
            [&](auto& m) { return m.name == name; }
        );

        if (raw_obj == raw_objects.end())
            throw exception(print_fmt("mesh not found, name = %s", name.c_str()).c_str());

        return std::make_unique<Mesh>(*raw_obj, m_render);
    });
}

shared_ptr<Model> RenderCache::get_model(const std::string& name)
{
    flog();

    return cache_get(m_models, name, [&]
    {
        auto geometry = m_asset.load_geometry(name);
        return std::make_unique<Model>(*geometry.get(), m_render);
    });
}

void RenderCache::clear()
{
    flog();

    m_meshes.clear();
    m_materials.clear();
    m_models.clear();
}
