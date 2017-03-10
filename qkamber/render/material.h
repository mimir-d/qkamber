#pragma once

#include "math3.h"

class Texture;

// TODO: this might need to stay in some other place
class Color : public vec<float, 4>
{
    using Base = vec<float, 4>;
public:
    using Base::Base;

    Color() : Base() {}
    Color(const Color& rhs) : Base(rhs) {}
    Color(const Base& rhs) : Base(rhs) {}

    float& r() { return m_data[0]; }
    float& g() { return m_data[1]; }
    float& b() { return m_data[2]; }
    float& a() { return m_data[3]; }

    float r() const { return m_data[0]; }
    float g() const { return m_data[1]; }
    float b() const { return m_data[2]; }
    float a() const { return m_data[3]; }
};

class Material
{
public:
    using Textures = std::vector<Texture*>;

public:
    Material();
    ~Material();

    // colors
    void set_ambient(const Color& color);
    void set_diffuse(const Color& color);
    void set_specular(const Color& color, float shininess = 1.0f);
    void set_emissive(const Color& color);

    const Color& get_ambient() const;
    const Color& get_diffuse() const;
    const Color& get_specular() const;
    float get_specular_shininess() const;
    const Color& get_emissive() const;

    // textures
    // TODO: support multiple textures
    void set_texture(Texture* texture);
    const Textures& get_textures() const;

private:
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    float m_specular_shininess = 1.0f;
    Color m_emissive;

    Textures m_textures;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline Material::Material()
{
    flog("id = %#x", this);
    log_info("Created material %#x", this);
}

inline Material::~Material()
{
    flog();
    log_info("Destroyed material %#x", this);
}

inline void Material::set_ambient(const Color& color)
{
    m_ambient = color;
}

inline void Material::set_diffuse(const Color& color)
{
    m_diffuse = color;
}

inline void Material::set_specular(const Color& color, float shininess)
{
    m_specular = color;
    m_specular_shininess = shininess;
}

inline void Material::set_emissive(const Color& color)
{
    m_emissive = color;
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

inline float Material::get_specular_shininess() const
{
    return m_specular_shininess;
}

inline const Color& Material::get_emissive() const
{
    return m_emissive;
}

inline void Material::set_texture(Texture* texture)
{
    if (m_textures.size() < 1)
        m_textures.resize(1);
    m_textures[0] = texture;
}

inline const Material::Textures& Material::get_textures() const
{
    return m_textures;
}
