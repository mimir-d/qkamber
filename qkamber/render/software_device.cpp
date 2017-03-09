
#include "precompiled.h"
#include "software_device.h"

#include "render_primitive.h"
#include "software_buffers.h"

using namespace std;

namespace
{
    class NullTarget : public RenderTarget
    {
    public:
        ColorBuffer& get_color_buffer() final;
        DepthBuffer& get_depth_buffer() final;

        int get_width() const final;
        int get_height() const final;
    };

    inline ColorBuffer& NullTarget::get_color_buffer()
    {
        throw std::exception("Attempted to draw in null target");
    }

    inline DepthBuffer& NullTarget::get_depth_buffer()
    {
        throw std::exception("Attempted to draw in null target");
    }

    inline int NullTarget::get_width() const
    {
        return 0;
    }

    inline int NullTarget::get_height() const
    {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
SoftwareDevice::SoftwareDevice() :
    m_null_target(new NullTarget)
{
    set_render_target(nullptr);
}

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
    size_t position_offset, color_offset, texcoord_offset;
    for (auto& di : vb.get_declaration())
    {
        switch (di.semantic)
        {
            case VDES_POSITION: position_offset = di.offset; break;
            case VDES_COLOR:    color_offset = di.offset;    break;
            case VDES_TEXCOORD:  texcoord_offset = di.offset; break;
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
        // vertex color computations
        const float* p_c0 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[0] * vertex_size);
        const float* p_c1 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[1] * vertex_size);
        const float* p_c2 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[2] * vertex_size);

        // TODO: make a ptr-based vec3
        vec3 c0{ p_c0[0], p_c0[1], p_c0[2] };
        vec3 c1{ p_c1[0], p_c1[1], p_c1[2] };
        vec3 c2{ p_c2[0], p_c2[1], p_c2[2] };

        // vertex texcoord computations
        const float* p_uv0 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[0] * vertex_size);
        const float* p_uv1 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[1] * vertex_size);
        const float* p_uv2 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[2] * vertex_size);

        // TODO: make a ptr-based vec3
        vec2 uv0{ p_uv0[0], p_uv0[1] };
        vec2 uv1{ p_uv1[0], p_uv1[1] };
        vec2 uv2{ p_uv2[0], p_uv2[1] };

        // vertex position computations
        const float* p_p0 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[0] * vertex_size);
        const float* p_p1 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[1] * vertex_size);
        const float* p_p2 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[2] * vertex_size);

        // TODO: make a ptr-based vec3
        // transform to view-space
        vec4 v0v = mv * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        vec4 v1v = mv * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        vec4 v2v = mv * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

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
        vec4 v0c = mvp * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        vec4 v1c = mvp * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        vec4 v2c = mvp * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

        // infinity transition when any vertices of the triangles are on +plane and the other on -plane
        const int s0 = sgn(v0c.w());
        const int s1 = sgn(v1c.w());
        const int s2 = sgn(v2c.w());
        if (!(s0 == s1 && s1 == s2))
            continue;

        const float wi0 = 1.0f / v0c.w();
        const float wi1 = 1.0f / v1c.w();
        const float wi2 = 1.0f / v2c.w();

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
        const vec3 v0d = m_clip_matrix * v0c;
        const vec3 v1d = m_clip_matrix * v1c;
        const vec3 v2d = m_clip_matrix * v2c;

        const DevicePoint dp0{ { v0d.x(), v0d.y(), v0c.z() }, c0 * wi0, uv0 * wi0, wi0 };
        const DevicePoint dp1{ { v1d.x(), v1d.y(), v1c.z() }, c1 * wi1, uv1 * wi1, wi1 };
        const DevicePoint dp2{ { v2d.x(), v2d.y(), v2c.z() }, c2 * wi2, uv2 * wi2, wi2 };

        draw_tri(dp0, dp1, dp2);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
unique_ptr<VertexBuffer> SoftwareDevice::create_vertex_buffer(unique_ptr<VertexDecl> decl, size_t count)
{
    auto ret = unique_ptr<VertexBuffer>{ new SoftwareVertexBuffer{ move(decl), count } };
    dlog("Created vertex buffer %#x", ret.get());
    return ret;
}

unique_ptr<IndexBuffer> SoftwareDevice::create_index_buffer(size_t count)
{
    auto ret = unique_ptr<IndexBuffer>{ new SoftwareIndexBuffer{ count } };
    dlog("Created index buffer %#x", ret.get());
    return ret;
}

unique_ptr<Texture> SoftwareDevice::create_texture(Image* image)
{
    auto ret = unique_ptr<Texture>{ new SoftwareTexture{ image } };
    dlog("Created texture %#x", ret.get());
    return ret;
}