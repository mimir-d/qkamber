#pragma once

#include "math3.h"

// TODO: impl spot lights
class Light
{
public:
    // position is [x, y, z, w]
    // if w == 0 this is a directional light
    void set_position(const vec4& position);
    const vec4& get_position() const;

    // attenuation values are [range, const, linear, quadratic]
    void set_attenuation(const vec4& attenuation);
    const vec4& get_attenuation() const;

    void set_ambient(const Color& ambient);
    const Color& get_ambient() const;

    void set_diffuse(const Color& diffuse);
    const Color& get_diffuse() const;

    void set_specular(const Color& specular);
    const Color& get_specular() const;

private:
    vec4 m_position;
    vec4 m_attenuation = { 0.0f, 1.0f, 0.0f, 0.0f };
    Color m_ambient, m_diffuse, m_specular;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline void Light::set_position(const vec4& position)
{
    m_position = position;
}

inline const vec4& Light::get_position() const
{
    return m_position;
}

inline void Light::set_attenuation(const vec4& attenuation)
{
    m_attenuation = attenuation;
}

inline const vec4& Light::get_attenuation() const
{
    return m_attenuation;
}

inline void Light::set_ambient(const Color& ambient)
{
    m_ambient = ambient;
}

inline const Color& Light::get_ambient() const
{
    return m_ambient;
}

inline void Light::set_diffuse(const Color& diffuse)
{
    m_diffuse = diffuse;
}

inline const Color& Light::get_diffuse() const
{
    return m_diffuse;
}

inline void Light::set_specular(const Color& specular)
{
    m_specular = specular;
}

inline const Color& Light::get_specular() const
{
    return m_specular;
}
