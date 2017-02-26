#pragma once

#include "render_system.h"
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
}

class SoftwareDevice : public RenderDevice
{
protected:
    struct DevicePoint
    {
        vec3 position;
        vec3 color;
        //vec2 tex_coords;
    };

public:
    SoftwareDevice();
    ~SoftwareDevice() = default;

public:
    // drawing methods
    void draw_primitive(const RenderPrimitive& primitive) final;

    // device state methods
    void set_world_matrix(mat4 world_matrix) final;
    void set_view_matrix(mat4 view_matrix) final;
    void set_proj_matrix(mat4 proj_matrix) final;
    void set_clip_matrix(mat3x4 clip_matrix) final;

    void set_polygon_mode(PolygonMode mode) final;
    void set_render_target(RenderTarget* target);

    // resource management methods
    std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) final;
    std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) final;

protected:
    virtual void draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2) = 0;

protected:
    PolygonMode m_poly_mode = PolygonMode::Fill;
    RenderTarget* m_render_target;
    std::unique_ptr<RenderTarget> m_null_target;

private:
    mat4 m_world_matrix, m_view_matrix, m_proj_matrix;
    mat3x4 m_clip_matrix;

    // computed stuff
    dirty_t<mat4, detail::make_mv> m_mv_matrix = { m_world_matrix, m_view_matrix };
    dirty_t<mat4, detail::make_mvp> m_mvp_matrix = { m_world_matrix, m_view_matrix, m_proj_matrix };
};

// TODO: move this to a RenderContext
inline void SoftwareDevice::set_world_matrix(mat4 world_matrix)
{
    m_world_matrix = world_matrix;
    // TODO: encode the set_dirty inside operator=
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::set_view_matrix(mat4 view_matrix)
{
    m_view_matrix = view_matrix;
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::set_proj_matrix(mat4 proj_matrix)
{
    m_proj_matrix = proj_matrix;
    m_mvp_matrix.set_dirty();
    m_mv_matrix.set_dirty();
}

inline void SoftwareDevice::set_clip_matrix(mat3x4 clip_matrix)
{
    m_clip_matrix = clip_matrix;
}

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