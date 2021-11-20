
#include "precompiled.h"
#include "material.h"

#include "render_system.h"
#include "asset/asset_system.h"

using namespace std;

Material::Material(const GeometryAsset::Material& raw, RenderSystem& render) :
    m_ambient(raw.ambient),
    m_diffuse(raw.diffuse),
    m_specular(raw.specular),
    m_emissive(raw.emissive),
    m_shininess(raw.shininess)
{
    flog("id = %#x", this);

    if (raw.diffuse_map)
    {
        size_t width = raw.diffuse_map->get_width();
        size_t height = raw.diffuse_map->get_height();
        const PixelFormat format = [&]
        {
            switch (raw.diffuse_map->get_format())
            {
                case ImageFormat::Rgba8: return PixelFormat::RgbaU8;
                case ImageFormat::Rgb8: return PixelFormat::RgbU8;
            }
            throw std::runtime_error("unknown image format for material construction");
        }();

        auto tex = render.get_device().create_texture(width, height, format);
        lock_buffer(tex.get(), [&](uint8_t* data)
        {
            const size_t size = height * width * Texture::get_elem_size(format);
            std::copy(raw.diffuse_map->data(), raw.diffuse_map->data() + size, data);
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
