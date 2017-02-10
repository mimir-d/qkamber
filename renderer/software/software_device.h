#pragma once

#include "renderer.h"
#include "math3.h"
#include "misc.h"

struct RenderPrimitive;
class VertexDecl;
class VertexBuffer;
class IndexBuffer;

namespace detail
{
    // TODO: would like to use a lambda here, but no idea how atm
    struct make_mvp
    {
        mat4 operator()(const mat4& world, const mat4& view, const mat4& proj) const
        {
            return proj * view * world;
        }
    };
}

class SoftwareDevice : public RenderDevice
{
public:
    // drawing methods
    void draw_primitive(const RenderPrimitive& primitive) final;

    // device state methods
    void set_world_matrix(mat4 world_matrix) final;
    void set_view_matrix(mat4 view_matrix) final;
    void set_proj_matrix(mat4 proj_matrix) final;
    void set_clip_matrix(mat3x4 clip_matrix) final;

    void set_polygon_mode(PolygonMode mode) final;

    // resource management methods
    std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) final;
    std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) final;

protected:
    virtual void draw_tri_point(float x0, float y0, float x1, float y1, float x2, float y2) = 0;
    virtual void draw_tri_line(float x0, float y0, float x1, float y1, float x2, float y2) = 0;
    virtual void draw_tri_fill(float x0, float y0, float x1, float y1, float x2, float y2) = 0;

private:
    void draw_tri(float x0, float y0, float x1, float y1, float x2, float y2);

private:
    mat4 m_world_matrix, m_view_matrix, m_proj_matrix;
    mat3x4 m_clip_matrix;

    // computed stuff
    dirty_t<mat4, detail::make_mvp> m_mvp_matrix = { m_world_matrix, m_view_matrix, m_proj_matrix };

    PolygonMode m_poly_mode = PolygonMode::Fill;
};

inline void SoftwareDevice::set_world_matrix(mat4 world_matrix)
{
    m_world_matrix = world_matrix;
    m_mvp_matrix.set_dirty();
}

inline void SoftwareDevice::set_view_matrix(mat4 view_matrix)
{
    m_view_matrix = view_matrix;
    m_mvp_matrix.set_dirty();
}

inline void SoftwareDevice::set_proj_matrix(mat4 proj_matrix)
{
    m_proj_matrix = proj_matrix;
    m_mvp_matrix.set_dirty();
}

inline void SoftwareDevice::set_clip_matrix(mat3x4 clip_matrix)
{
    m_clip_matrix = clip_matrix;
}

inline void SoftwareDevice::set_polygon_mode(PolygonMode mode)
{
    m_poly_mode = mode;
}