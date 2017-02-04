
#include "stdafx.h"
#include "software_device.h"

#include "render_primitive.h"
#include "software_buffers.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void SoftwareDevice::draw_primitive(const RenderPrimitive& primitive)
{
    auto& mvp = m_mvp_matrix.get();
    auto& vb = static_cast<const SoftwareVertexBuffer&>(primitive.vertices);
    auto& ib = static_cast<const SoftwareIndexBuffer&>(primitive.indices);

    size_t pos_offset, pos_size;
    size_t color_offset, color_incr;

    // go thru declaration and figure out the offsets and data size
    for (auto& di : vb.get_declaration())
    {
        switch (di.semantic)
        {
            case VDES_POSITION:
                pos_offset = di.offset;
                pos_size = VertexDecl::get_elem_size(di.type);
                break;

            case VDES_COLOR:
                color_offset = di.offset;
                color_incr = VertexDecl::get_elem_size(di.type);
                break;
        }
    }
    size_t vertex_size = vb.get_declaration().get_vertex_size();

    // TODO: this will need to change when index size is != uint16_t
    const uint16_t* ib_ptr = reinterpret_cast<const uint16_t*>(ib.data());

    for (size_t i = 0; i < ib.get_count(); i += 3)
    {
        const float* p0 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[0] * vertex_size);
        const float* p1 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[1] * vertex_size);
        const float* p2 = reinterpret_cast<const float*>(vb.data() + pos_offset + ib_ptr[2] * vertex_size);

        vec4 v0 { p0[0], p0[1], p0[2], 1.0f };
        vec4 v1 { p1[0], p1[1], p1[2], 1.0f };
        vec4 v2 { p2[0], p2[1], p2[2], 1.0f };

        v0 = mvp * v0;
        v0 *= 1.0f / v0.w();

        v1 = mvp * v1;
        v1 *= 1.0f / v1.w();

        v2 = mvp * v2;
        v2 *= 1.0f / v2.w();

        vec3 sv0 = m_clip_matrix * v0;
        vec3 sv1 = m_clip_matrix * v1;
        vec3 sv2 = m_clip_matrix * v2;

        draw_tri(sv0.x(), sv0.y(), sv1.x(), sv1.y(), sv2.x(), sv2.y());
        //draw_tri_wireframe(sv0.x(), sv0.y(), sv1.x(), sv1.y(), sv2.x(), sv2.y());

        ib_ptr += 3;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
unique_ptr<VertexBuffer> SoftwareDevice::create_vertex_buffer(unique_ptr<VertexDecl> decl, size_t count)
{
    return unique_ptr<VertexBuffer>(new SoftwareVertexBuffer(move(decl), count));
}

unique_ptr<IndexBuffer> SoftwareDevice::create_index_buffer(size_t count)
{
    return unique_ptr<IndexBuffer>(new SoftwareIndexBuffer(count));
}