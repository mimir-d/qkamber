#pragma once

#include "render_system.h"
#include "material.h"
#include "scene/light.h"
#include "math3.h"
#include "misc.h"

struct RenderPrimitive;
class VertexDecl;
class VertexBuffer;
class IndexBuffer;

namespace detail
{
    struct make_mvp
    {
        mat4 operator()(const mat4& world, const mat4& view, const mat4& proj) const
        {
            return proj * view * world;
        }
    };

    struct make_mv
    {
        mat4 operator()(const mat4& world, const mat4& view) const
        {
            return view * world;
        }
    };

    struct make_normal
    {
        mat3 operator()(const mat4& view_inv, const mat4& world_inv) const
        {
            return mat3{ (world_inv * view_inv).transpose() };
        }
    };

    constexpr size_t SOFTWARE_TEXTURE_COUNT = 2;
    constexpr size_t SOFTWARE_LIGHT_COUNT = 2;
}

class SoftwareDevice : public RenderDevice
{
protected:
    class SoftwareParams : public Params
    {
    public:
        SoftwareParams() = default;
        SoftwareParams(const SoftwareParams&) = delete;
        ~SoftwareParams() = default;

    public:
        void set_world_matrix(mat4 world_matrix) final;
        void set_view_matrix(mat4 view_matrix) final;
        void set_proj_matrix(mat4 proj_matrix) final;
        void set_clip_matrix(mat3x4 clip_matrix) final;

        void set_world_inv_matrix(mat4 world_inv_matrix) final;
        void set_view_inv_matrix(mat4 view_inv_matrix) final;

        void set_material(const Material& material) final;

    public:
        const mat4& get_world_matrix() const;
        const mat4& get_view_matrix() const;
        const mat4& get_proj_matrix() const;
        const mat3x4& get_clip_matrix() const;

        const mat4& get_mv_matrix();
        const mat4& get_mvp_matrix();
        const mat3& get_normal_matrix();

        const mat4& get_world_inv_matrix() const;
        const mat4& get_view_inv_matrix() const;

        const Color& get_material_ambient() const;
        const Color& get_material_diffuse() const;
        const Color& get_material_specular() const;
        const Color& get_material_emissive() const;
        float get_material_shininess() const;
        bool get_material_lighting() const;

    private:
        mat4 m_world_matrix, m_world_inv_matrix;
        mat4 m_view_matrix, m_view_inv_matrix;
        mat4 m_proj_matrix;
        mat3x4 m_clip_matrix;

        Color m_material_ambient;
        Color m_material_diffuse;
        Color m_material_specular;
        Color m_material_emissive;
        float m_material_shininess = 0.0f;
        bool m_material_lighting = false;

        // computed stuff
        dirty_t<mat4, detail::make_mv> m_mv_matrix = { m_world_matrix, m_view_matrix };
        dirty_t<mat4, detail::make_mvp> m_mvp_matrix = { m_world_matrix, m_view_matrix, m_proj_matrix };
        dirty_t<mat3, detail::make_normal> m_normal_matrix = { m_view_inv_matrix, m_world_inv_matrix };
    };

    struct DevicePoint
    {
        vec4 position;
        optional_t<vec3> view_position;
        optional_t<vec3> view_normal;
        optional_t<Color> color;
        optional_t<vec2> texcoord;
    };

public:
    SoftwareDevice();
    ~SoftwareDevice() = default;

public:
    // drawing methods
    void draw_primitive(const RenderPrimitive& primitive) final;

    // device state methods
    void set_polygon_mode(PolygonMode mode) final;
    void set_render_target(RenderTarget* target) override;
    void set_texture_unit(size_t index, const Texture* texture) final;
    void set_light_unit(size_t index, const Light* light) final;
    Params& get_params() final;

    // resource management methods
    std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) final;
    std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) final;
    std::unique_ptr<Texture> create_texture(size_t width, size_t height, PixelFormat format) final;

    // capabilities methods
    size_t get_texture_unit_count() const final;
    size_t get_light_unit_count() const final;

    // debug
    void debug_normals(bool enable);

protected:
    void draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2);

    // TODO: these 2 should also be software rendered
    virtual void draw_points(const std::vector<DevicePoint>& points) = 0;
    virtual void draw_lines(const std::vector<DevicePoint>& points) = 0;
    void draw_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2);

protected:
    SoftwareParams m_params;

    PolygonMode m_poly_mode = PolygonMode::Fill;
    RenderTarget* m_render_target;
    std::array<const Texture*, detail::SOFTWARE_TEXTURE_COUNT> m_texture_units;

    std::array<vec3, detail::SOFTWARE_LIGHT_COUNT> m_light_view_positions;
    std::array<const Light*, detail::SOFTWARE_LIGHT_COUNT> m_light_units;

    std::unique_ptr<RenderTarget> m_null_target;
    bool m_debug_normals = false;
};

///////////////////////////////////////////////////////////////////////////////
// SoftwareDevice::SoftwareState impl
///////////////////////////////////////////////////////////////////////////////
inline void SoftwareDevice::SoftwareParams::set_world_matrix(mat4 world_matrix)
{
    m_world_matrix = world_matrix;
    // TODO: encode the set_dirty inside operator=
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::SoftwareParams::set_view_matrix(mat4 view_matrix)
{
    m_view_matrix = view_matrix;
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::SoftwareParams::set_proj_matrix(mat4 proj_matrix)
{
    m_proj_matrix = proj_matrix;
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::SoftwareParams::set_clip_matrix(mat3x4 clip_matrix)
{
    m_clip_matrix = clip_matrix;
}

inline void SoftwareDevice::SoftwareParams::set_world_inv_matrix(mat4 world_inv_matrix)
{
    m_world_inv_matrix = world_inv_matrix;
    m_normal_matrix.set_dirty();
}

inline void SoftwareDevice::SoftwareParams::set_view_inv_matrix(mat4 view_inv_matrix)
{
    m_view_inv_matrix = view_inv_matrix;
    m_normal_matrix.set_dirty();
}

inline void SoftwareDevice::SoftwareParams::set_material(const Material& material)
{
    m_material_ambient = material.get_ambient();
    m_material_diffuse = material.get_diffuse();
    m_material_specular = material.get_specular();
    m_material_emissive = material.get_emissive();
    m_material_shininess = material.get_shininess();
    m_material_lighting = material.get_lighting_enable();
}

inline const mat4& SoftwareDevice::SoftwareParams::get_world_matrix() const
{
    return m_world_matrix;
}

inline const mat4& SoftwareDevice::SoftwareParams::get_view_matrix() const
{
    return m_view_matrix;
}

inline const mat4& SoftwareDevice::SoftwareParams::get_proj_matrix() const
{
    return m_proj_matrix;
}

inline const mat3x4& SoftwareDevice::SoftwareParams::get_clip_matrix() const
{
    return m_clip_matrix;
}

inline const mat4& SoftwareDevice::SoftwareParams::get_mv_matrix()
{
    return m_mv_matrix.get();
}

inline const mat4& SoftwareDevice::SoftwareParams::get_mvp_matrix()
{
    return m_mvp_matrix.get();
}

inline const mat3& SoftwareDevice::SoftwareParams::get_normal_matrix()
{
    return m_normal_matrix.get();
}

inline const mat4& SoftwareDevice::SoftwareParams::get_world_inv_matrix() const
{
    return m_world_inv_matrix;
}

inline const mat4& SoftwareDevice::SoftwareParams::get_view_inv_matrix() const
{
    return m_view_inv_matrix;
}

inline const Color& SoftwareDevice::SoftwareParams::get_material_ambient() const
{
    return m_material_ambient;
}

inline const Color& SoftwareDevice::SoftwareParams::get_material_diffuse() const
{
    return m_material_diffuse;
}

inline const Color& SoftwareDevice::SoftwareParams::get_material_specular() const
{
    return m_material_specular;
}

inline const Color& SoftwareDevice::SoftwareParams::get_material_emissive() const
{
    return m_material_emissive;
}

inline float SoftwareDevice::SoftwareParams::get_material_shininess() const
{
    return m_material_shininess;
}

inline bool SoftwareDevice::SoftwareParams::get_material_lighting() const
{
    return m_material_lighting;
}

///////////////////////////////////////////////////////////////////////////////
// SoftwareDevice impl
///////////////////////////////////////////////////////////////////////////////
inline void SoftwareDevice::set_polygon_mode(PolygonMode mode)
{
    m_poly_mode = mode;
}

inline void SoftwareDevice::set_render_target(RenderTarget* target)
{
    flog();

    if (!target)
    {
        m_render_target = m_null_target.get();
        log_info("Set render target to null");
        return;
    }

    m_render_target = target;
    log_info("Set render target %#x", target);
}

inline void SoftwareDevice::set_texture_unit(size_t index, const Texture* texture)
{
    if (index >= detail::SOFTWARE_TEXTURE_COUNT)
        throw std::runtime_error("invalid texture unit index");
    m_texture_units[index] = texture;
}

inline void SoftwareDevice::set_light_unit(size_t index, const Light* light)
{
    if (index >= detail::SOFTWARE_LIGHT_COUNT)
        throw std::runtime_error("invalid light unit index");

    m_light_units[index] = light;
    if (light)
        m_light_view_positions[index] = vec3{ m_params.get_view_matrix() * light->get_position() };
}

inline RenderDevice::Params& SoftwareDevice::get_params()
{
    return m_params;
}

inline size_t SoftwareDevice::get_texture_unit_count() const
{
    return detail::SOFTWARE_TEXTURE_COUNT;
}

inline size_t SoftwareDevice::get_light_unit_count() const
{
    return detail::SOFTWARE_LIGHT_COUNT;
}

inline void SoftwareDevice::debug_normals(bool enable)
{
    m_debug_normals = enable;
}
