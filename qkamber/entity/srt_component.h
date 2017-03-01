#pragma once

#include "misc.h"
#include "math3.h"

namespace detail
{
    struct make_world
    {
        mat4 operator()(const vec3& s, const vec3& r, const vec3& t) const
        {
            const mat4 translate = mat4::translate(t.x(), t.y(), t.z());
            const mat4 rotate = mat4::rotate(r.x(), r.y(), r.z());
            const mat4 scale = mat4::scale(s.x(), s.y(), s.z());
            return translate * rotate * scale;
        }
    };
};

class SrtComponent
{
public:
    SrtComponent() = default;
    SrtComponent(const SrtComponent& rhs);
    SrtComponent(SrtComponent&&) = default;
    ~SrtComponent() = default;

    void set_scale(const vec3& scale);
    const vec3& get_scale() const;

    void set_rotation(const vec3& rotation);
    const vec3& get_rotation() const;

    void set_position(const vec3& position);
    const vec3& get_position() const;

    const mat4& get_world();

private:
    vec3 m_scale = { 1, 1, 1 };
    vec3 m_rotation = { 0, 0, 0 };
    vec3 m_position = { 0, 0, 0 };
    dirty_t<mat4, detail::make_world> m_world = { m_scale, m_rotation, m_position };
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline SrtComponent::SrtComponent(const SrtComponent& rhs)
{
    // TODO: cant figure out how to copy an object and skip the dirty_t such that it's recreated with the new refs
    m_scale = rhs.m_scale;
    m_rotation = rhs.m_rotation;
    m_position = rhs.m_position;
}

inline void SrtComponent::set_scale(const vec3& scale)
{
    m_scale = scale;
    m_world.set_dirty();
}

inline const vec3& SrtComponent::get_scale() const
{
    return m_scale;
}

inline void SrtComponent::set_rotation(const vec3& rotation)
{
    m_rotation = rotation;
    m_world.set_dirty();
}

inline const vec3& SrtComponent::get_rotation() const
{
    return m_rotation;
}

inline void SrtComponent::set_position(const vec3& position)
{
    m_position = position;
    m_world.set_dirty();
}

inline const vec3& SrtComponent::get_position() const
{
    return m_position;
}

inline const mat4& SrtComponent::get_world()
{
    return m_world.get();
}
