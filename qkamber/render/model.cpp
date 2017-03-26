
#include "precompiled.h"
#include "model.h"

#include "render/render_system.h"
#include "asset/asset_system.h"
#include "mesh.h"
#include "material.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Model impl
///////////////////////////////////////////////////////////////////////////////
Model::Model(GeometryAsset& geometry, RenderDevice& dev, AssetSystem& asset)
{
    flog("id = %#x", this);

    size_t unit_count = geometry.get_objects().size();
    m_meshes.reserve(unit_count);
    m_materials.reserve(unit_count);
    m_units.reserve(unit_count);

    auto& materials = geometry.get_materials();
    for (auto& raw_obj : geometry.get_objects())
    {
        auto search = std::find_if(
            materials.begin(), materials.end(),
            [&](auto& m) { return m.name == raw_obj.material_name; }
        );
        if (search == materials.end())
            throw exception("material not found as defined");
        auto& raw_mat = *search;

        m_meshes.emplace_back(new Mesh{ raw_obj, dev });
        m_materials.emplace_back(new Material{ raw_mat, dev, asset });

        Mesh* mesh = m_meshes[m_meshes.size() - 1].get();
        Material* mat = m_materials[m_materials.size() - 1].get();

        m_units.emplace_back(mesh, mat);
    }

    log_info("Created model %#x", this);
}

Model::~Model()
{
    flog();
    log_info("Destroyed model %#x", this);
}
