
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
namespace
{
    template <typename T>
    T& last(vector<T>& v)
    {
        return v[v.size() - 1];
    }
}

Model::Model(GeometryAsset& geometry, RenderDevice& dev, AssetSystem& asset)
{
    flog("id = %#x", this);

    for (auto& obj : geometry.get_objects())
    {
        m_meshes.emplace_back(new Mesh{ obj, dev });
        m_materials.emplace_back(new Material);

        auto& mat = geometry.get_materials()[obj.material_index];
        if (mat.tex_filename.size() > 0)
        {
            auto im = asset.load_image(mat.tex_filename);
            m_textures.push_back(dev.create_texture(im.get()));
            last(m_materials)->set_texture(last(m_textures).get());
        }

        m_units.emplace_back(last(m_meshes).get(), last(m_materials).get());
    }

    log_info("Created model %#x", this);
}

Model::~Model()
{
    flog();
    log_info("Destroyed model %#x", this);
}
