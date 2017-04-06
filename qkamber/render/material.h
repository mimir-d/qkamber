#pragma once

#include "asset/asset_system.h"
#include "math3.h"

class RenderSystem;
class Texture;

class Material
{
public:
    using Textures = std::vector<const Texture*>;

public:
    Material(const GeometryAsset::Material& raw, RenderSystem& render);
    ~Material();

    // lighting
    // TODO: this would come from config file
    void set_lighting_enable(bool enable);
    bool get_lighting_enable() const;

    // colors
    const Color& get_ambient() const;
    const Color& get_diffuse() const;
    const Color& get_specular() const;
    const Color& get_emissive() const;
    float get_shininess() const;

    // textures
    // TODO: support multiple textures
    const Textures& get_textures() const;

private:
    bool m_lighting_enabled = true;
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    Color m_emissive;
    float m_shininess = 1.0f;

    std::vector<std::unique_ptr<Texture>> m_tex_storage;
    Textures m_textures;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void Material::set_lighting_enable(bool enable)
{
    m_lighting_enabled = enable;
}

inline bool Material::get_lighting_enable() const
{
    return m_lighting_enabled;
}

inline const Color& Material::get_ambient() const
{
    return m_ambient;
}

inline const Color& Material::get_diffuse() const
{
    return m_diffuse;
}

inline const Color& Material::get_specular() const
{
    return m_specular;
}

inline const Color& Material::get_emissive() const
{
    return m_emissive;
}

inline float Material::get_shininess() const
{
    return m_shininess;
}

inline const Material::Textures& Material::get_textures() const
{
    return m_textures;
}
