
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

    for (auto& obj : geometry.get_objects())
    {
        m_meshes.emplace_back(new Mesh{ obj, dev });
        m_materials.emplace_back(new Material);

        Mesh* mesh = m_meshes[m_meshes.size() - 1].get();
        Material* mat = m_materials[m_materials.size() - 1].get();

        auto& raw_mat = geometry.get_materials()[obj.material_index];

        mat->set_ambient(raw_mat.ambient);
        mat->set_diffuse(raw_mat.diffuse);
        mat->set_specular(raw_mat.specular, raw_mat.specular_shininess);
        mat->set_emissive(raw_mat.emissive);

        if (raw_mat.tex_filename.size() > 0)
        {
            auto im = asset.load_image(raw_mat.tex_filename);
            const PixelFormat format = [&]
            {
                switch (im->get_format())
                {
                    case ImageFormat::Rgba8: return PixelFormat::RgbaU8;
                    case ImageFormat::Rgb8: return PixelFormat::RgbU8;
                }
                throw exception("unknown image format for material construction");
            }();

            auto tex = dev.create_texture(im->get_width(), im->get_height(), format);
            lock_buffer(tex.get(), [&](uint8_t* data)
            {
                const size_t size = im->get_height() * im->get_width() * Texture::get_elem_size(format);
                std::copy(im->data(), im->data() + size, data);
            });

            m_textures.push_back(std::move(tex));
            mat->set_texture(m_textures[m_textures.size() - 1].get());
        }

        m_units.emplace_back(mesh, mat);
    }

    log_info("Created model %#x", this);
}

Model::~Model()
{
    flog();
    log_info("Destroyed model %#x", this);
}
