
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
    auto& mv_matrix = m_mv_matrix.get();
    auto& mvp_matrix = m_mvp_matrix.get();
    auto& normal_matrix = m_normal_matrix.get();

    auto& vb = static_cast<const SoftwareVertexBuffer&>(primitive.vertices);
    auto& ib = static_cast<const SoftwareIndexBuffer&>(primitive.indices);

    // go thru declaration and figure out the offsets and data size
    int position_offset = -1;
    int normal_offset = -1;
    int color_offset = -1;
    int texcoord_offset = -1;
    for (auto& di : vb.get_declaration())
    {
        switch (di.semantic)
        {
            case VDES_POSITION: position_offset = static_cast<int>(di.offset); break;
            case VDES_NORMAL: normal_offset = static_cast<int>(di.offset); break;
            case VDES_COLOR: color_offset = static_cast<int>(di.offset); break;
            case VDES_TEXCOORD: texcoord_offset = static_cast<int>(di.offset); break;
        }
    }
    size_t vertex_size = vb.get_declaration().get_vertex_size();

    // TODO: this will need to change when index size is != uint16_t
    const uint16_t* ib_ptr = reinterpret_cast<const uint16_t*>(ib.data());

    // TODO: performance, push these to display lists and parallel process
    for (size_t i = 0; i < ib.get_count(); i += 3, ib_ptr += 3)
    {
        // vertex position computations
        const float* p_p0 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[0] * vertex_size);
        const float* p_p1 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[1] * vertex_size);
        const float* p_p2 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[2] * vertex_size);

        // transform to view-space
        const vec4 v0v = mv_matrix * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        const vec4 v1v = mv_matrix * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        const vec4 v2v = mv_matrix * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

        // TODO: make a vecN to vecM ctor
        const vec3 v0v_3 = { v0v[0], v0v[1], v0v[2] };
        const vec3 v1v_3 = { v1v[0], v1v[1], v1v[2] };
        const vec3 v2v_3 = { v2v[0], v2v[1], v2v[2] };

        // compute view-space normal
        const vec3 dv1 = (v2v_3 - v0v_3);
        const vec3 dv2 = (v1v_3 - v0v_3);
        // TODO: fix cross operator to work with temporaries
        const vec3 vnv = (dv1 ^ dv2).normalize();

        if (v0v_3 * vnv < 0)
        {
            // discard back-facing triangle
            continue;
        }

        // transform to clip-space
        vec4 v0c = mvp_matrix * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        vec4 v1c = mvp_matrix * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        vec4 v2c = mvp_matrix * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

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
        v0c *= wi0;
        v1c *= wi1;
        v2c *= wi2;

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

        DevicePoint dp[3];
        dp[0].position = vec4{ v0d.x(), v0d.y(), v0c.z(), wi0 };
        dp[1].position = vec4{ v1d.x(), v1d.y(), v1c.z(), wi1 };
        dp[2].position = vec4{ v2d.x(), v2d.y(), v2c.z(), wi2 };

        dp[0].view_position = v0v_3 * wi0;
        dp[1].view_position = v1v_3 * wi1;
        dp[2].view_position = v2v_3 * wi2;

        if (normal_offset >= 0)
        {
            const float* p_n0 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[0] * vertex_size);
            const float* p_n1 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[1] * vertex_size);
            const float* p_n2 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[2] * vertex_size);

            dp[0].view_normal = (normal_matrix * vec3{ p_n0[0], p_n0[1], p_n0[2] }) * wi0;
            dp[1].view_normal = (normal_matrix * vec3{ p_n1[0], p_n1[1], p_n1[2] }) * wi1;
            dp[2].view_normal = (normal_matrix * vec3{ p_n2[0], p_n2[1], p_n2[2] }) * wi2;
        }

        if (color_offset >= 0)
        {
            const float* p_c0 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[0] * vertex_size);
            const float* p_c1 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[1] * vertex_size);
            const float* p_c2 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[2] * vertex_size);

            // TODO: make a ptr-based vec3
            dp[0].color = Color{ p_c0[0], p_c0[1], p_c0[2], p_c0[3] } * wi0;
            dp[1].color = Color{ p_c1[0], p_c1[1], p_c1[2], p_c1[3] } * wi1;
            dp[2].color = Color{ p_c2[0], p_c2[1], p_c2[2], p_c2[3] } * wi2;
        }

        if (texcoord_offset >= 0)
        {
            const float* p_uv0 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[0] * vertex_size);
            const float* p_uv1 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[1] * vertex_size);
            const float* p_uv2 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[2] * vertex_size);

            dp[0].texcoord = vec2{ p_uv0[0], p_uv0[1] } * wi0;
            dp[1].texcoord = vec2{ p_uv1[0], p_uv1[1] } * wi1;
            dp[2].texcoord = vec2{ p_uv2[0], p_uv2[1] } * wi2;
        }

        if (m_debug_normals)
        {
            for (int i = 0; i < 3; i++)
            {
                // compute screen-space (vertex + normal)
                vec3 v_dn = dp[i].view_position.value() * (1.0f / wi0) + dp[i].view_normal.value().normalize() * 0.5f;
                vec4 v_dnc = m_proj_matrix * vec4{ v_dn.x(), v_dn.y(), v_dn.z(), 1.0f };
                v_dnc *= 1.0f / v_dnc.w();
                vec3 v_dnd = m_clip_matrix * v_dnc;

                DevicePoint n_dp[2];
                n_dp[0].position = dp[i].position;
                n_dp[1].position = vec4{ v_dnd.x(), v_dnd.y(), 0, 0 };
                draw_line(n_dp[0], n_dp[1]);
            }
        }

        draw_tri(dp[0], dp[1], dp[2]);
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
