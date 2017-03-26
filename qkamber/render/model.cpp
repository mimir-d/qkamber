
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

    for (auto& raw_obj : geometry.get_objects())
    {
        auto& raw_mat = geometry.get_materials()[raw_obj.material_index];

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
