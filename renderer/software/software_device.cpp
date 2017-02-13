
#include "stdafx.h"
#include "software_device.h"

#include "render_primitive.h"
#include "software_buffers.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

void SoftwareDevice::draw_primitive(const RenderPrimitive& primitive)
{
    auto& mv = m_mv_matrix.get();
    auto& mvp = m_mvp_matrix.get();

    auto& vb = static_cast<const SoftwareVertexBuffer&>(primitive.vertices);
    auto& ib = static_cast<const SoftwareIndexBuffer&>(primitive.indices);

    // go thru declaration and figure out the offsets and data size
    size_t pos_offset, color_offset;
    for (auto& di : vb.get_declaration())
    {
        switch (di.semantic)
        {
            case VDES_POSITION: pos_offset = di.offset;     break;
            case VDES_COLOR:    color_offset = di.offset;   break;
        }
    }
    size_t vertex_size = vb.get_declaration().get_vertex_size();

    // TODO: this will need to change when index size is != uint16_t
    const uint16_t* ib_ptr = reinterpret_cast<const uint16_t*>(ib.data());
//
//     mat4 n_mat4 = mv.transpose().invert();
//     mat3 nmat = {
//         n_mat4[0][0], n_mat4[0][1], n_mat4[0][2],
//         n_mat4[1][0], n_mat4[1][1], n_mat4[1][2],
//         n_mat4[2][0], n_mat4[2][1], n_mat4[2][2]
//     };

    // TODO: performance, push these to display lists and parallel process
    for (size_t i = 0; i < ib.get_count(); i += 3, ib_ptr += 3)
    {
        const float* p0 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[0] * vertex_size);
        const float* p1 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[1] * vertex_size);
        const float* p2 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[2] * vertex_size);

        // TODO: make a ptr-based vec3
        // transform to view-space
        vec4 v0v = mv * vec4 { p0[0], p0[1], p0[2], 1.0f };
        vec4 v1v = mv * vec4 { p1[0], p1[1], p1[2], 1.0f };
        vec4 v2v = mv * vec4 { p2[0], p2[1], p2[2], 1.0f };

        // TODO: make a vecN to vecM ctor
        vec3 v0v_3 = { v0v[0], v0v[1], v0v[2] };
        vec3 v1v_3 = { v1v[0], v1v[1], v1v[2] };
        vec3 v2v_3 = { v2v[0], v2v[1], v2v[2] };

        // compute view-space normal
        vec3 dv1 = (v2v_3 - v0v_3);
        vec3 dv2 = (v1v_3 - v0v_3);
        // TODO: fix cross operator to work with temporaries
        vec3 vnv = (dv1 ^ dv2).normalize();

        if (v0v_3 * vnv < 0)
        {
            // discard back-facing triangle
            continue;
        }

        // transform to clip-space
        vec4 v0c = mvp * vec4 { p0[0], p0[1], p0[2], 1.0f };
        vec4 v1c = mvp * vec4 { p1[0], p1[1], p1[2], 1.0f };
        vec4 v2c = mvp * vec4 { p2[0], p2[1], p2[2], 1.0f };

        // infinity transition when any vertices of the triangles are on +plane and the other on -plane
        int s0 = sgn(v0c.w());
        int s1 = sgn(v1c.w());
        int s2 = sgn(v2c.w());
        if (!(s0 == s1 && s1 == s2))
            continue;

        // perspective division
        v0c *= 1.0f / v0c.w();
        v1c *= 1.0f / v1c.w();
        v2c *= 1.0f / v2c.w();

        // frustrum culling - left, right view planes
        if (v0c.x() < -1.0f && v1c.x() < -1.0f && v2c.x() < -1.0f)
            continue;
        if (v0c.x() > 1.0f && v1c.x() > 1.0f && v2c.x() > 1.0f)
            continue;

        // frustrum culling - top, down view planes
        if (v0c.y() < -1.0f && v1c.y() < -1.0f && v2c.y() < -1.0f)
            continue;
        if (v0c.y() > 1.0f && v1c.y() > 1.0f && v2c.y() > 1.0f)
            continue;

        // frustrum culling - near, far view planes
        if (v0c.z() < 0.0f && v1c.z() < 0.0f && v2c.z() < 0.0f)
            continue;
        if (v0c.z() > 1.0f && v1c.z() > 1.0f && v2c.z() > 1.0f)
            continue;

        // transform to device space
        vec3 v0d = m_clip_matrix * v0c;
        vec3 v1d = m_clip_matrix * v1c;
        vec3 v2d = m_clip_matrix * v2c;

        draw_tri(v0d.x(), v0d.y(), v1d.x(), v1d.y(), v2d.x(), v2d.y());
    }
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
unique_ptr<VertexBuffer> SoftwareDevice::create_vertex_buffer(unique_ptr<VertexDecl> decl, size_t count)
{
    auto ret = unique_ptr<VertexBuffer>(new SoftwareVertexBuffer(move(decl), count));
    dlog("Created vertex buffer %#x", ret.get());
    return ret;
}

unique_ptr<IndexBuffer> SoftwareDevice::create_index_buffer(size_t count)
{
    auto ret = unique_ptr<IndexBuffer>(new SoftwareIndexBuffer(count));
    dlog("Created index buffer %#x", ret.get());
    return ret;
}

void SoftwareDevice::draw_tri(float x0, float y0, float x1, float y1, float x2, float y2)
{
    // TODO: refactor this for speed
    switch (m_poly_mode)
    {
        case PolygonMode::Point:
            draw_tri_point(x0, y0, x1, y1, x2, y2);
            break;

        case PolygonMode::Line:
            draw_tri_line(x0, y0, x1, y1, x2, y2);
            break;

        case PolygonMode::Fill:
            draw_tri_fill(x0, y0, x1, y1, x2, y2);
            break;
    }
}
