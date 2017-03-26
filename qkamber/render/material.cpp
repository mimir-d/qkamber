
#include "precompiled.h"
#include "material.h"

#include "render_system.h"
#include "asset/asset_system.h"

using namespace std;

Material::Material(const GeometryAsset::Material& raw, RenderDevice& dev, AssetSystem& asset) :
    m_ambient(raw.ambient),
    m_diffuse(raw.diffuse),
    m_specular(raw.specular),
    m_emissive(raw.emissive),
    m_shininess(raw.shininess)
{
    flog("id = %#x", this);

    if (raw.tex_filename.size() > 0)
    {
        auto diffuse_map = asset.load_image(raw.tex_filename);
        size_t width = diffuse_map->get_width();
        size_t height = diffuse_map->get_height();
        const PixelFormat format = [&]
        {
            switch (diffuse_map->get_format())
            {
                case ImageFormat::Rgba8: return PixelFormat::RgbaU8;
                case ImageFormat::Rgb8: return PixelFormat::RgbU8;
            }
            throw exception("unknown image format for material construction");
        }();

        auto tex = dev.create_texture(width, height, format);
        lock_buffer(tex.get(), [&](uint8_t* data)
        {
            const size_t size = height * width * Texture::get_elem_size(format);
            std::copy(diffuse_map->data(), diffuse_map->data() + size, data);
        });

        m_textures.push_back(tex.get());
        m_tex_storage.push_back(std::move(tex));
    }

    log_info("Created material name = %s, id = %#x", raw.name.c_str(), this);
}

Material::~Material()
{
    flog();
    log_info("Destroyed material %#x", this);
}
