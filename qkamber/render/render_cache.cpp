
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

    auto ret = cache_get(m_materials, name, [&]
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
        {
            log_warn("Material not found [name = %s], using default material...", name.c_str());
            // TODO: this needs to stay somewhere else
            GeometryAsset::Material default_mat
            {
                "default_material",
                Color{ 0.2f, 0.2f, 0.2f, 1.0f },
                Color{ 0.8f, 0.8f, 0.8f, 1.0f },
                Color{ 1.0f, 1.0f, 1.0f, 1.0f },
                Color{ 0.0f, 0.0f, 0.0f, 0.0f },
                1.0f
            };
            default_mat.diffuse_map = m_asset.load_image("default_tex.bmp");
            return make_unique<Material>(default_mat, m_render);
        }
        return make_unique<Material>(*raw_mat, m_render);
    });

    // TODO: some factory customization here (and COW materials)
    if (name.find("axis.3ds") != string::npos)
        ret->set_lighting_enable(false);
    return ret;
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
            throw exception(print_fmt("Mesh not found [name = %s]", name.c_str()).c_str());

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
