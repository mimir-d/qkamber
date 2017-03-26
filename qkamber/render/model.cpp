
#include "precompiled.h"
#include "model.h"

#include "render/render_system.h"
#include "asset/asset_system.h"
#include "mesh.h"
#include "material.h"

using namespace std;

Model::Model(const GeometryAsset& geometry, RenderSystem& render)
{
    flog("id = %#x", this);

    const auto& raw_objects = geometry.get_objects();
    m_units.reserve(raw_objects.size());

    auto& cache = render.get_cache();
    for (const auto& raw_obj : raw_objects)
    {
        m_units.emplace_back(
            cache.get_mesh(raw_obj.name),
            cache.get_material(raw_obj.material_name)
        );
    }

    log_info("Created model name = %s, id = %#x", geometry.get_name().c_str(), this);
}

Model::~Model()
{
    flog();
    log_info("Destroyed model %#x", this);
}
