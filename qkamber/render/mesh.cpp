
#include "precompiled.h"
#include "mesh.h"

#include "render/render_system.h"
#include "render/render_buffers.h"
#include "math3.h"

Mesh::Mesh(const GeometryAsset::Object& raw, RenderSystem& render)
{
    flog("id = %#x", this);
    auto& dev = render.get_device();

    bool has_normals = raw.normals.size() > 0;
    bool has_colors = raw.colors.size() > 0;
    bool has_texcoords = raw.texcoords.size() > 0;

    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    size_t offset = 0;

    decl->add(offset, VDET_FLOAT3, VDES_POSITION);
    offset += VertexDecl::get_elem_size(VDET_FLOAT3);

    if (has_normals)
    {
        decl->add(offset, VDET_FLOAT3, VDES_NORMAL);
        offset += VertexDecl::get_elem_size(VDET_FLOAT3);
    }

    if (has_colors)
    {
        decl->add(offset, VDET_FLOAT4, VDES_COLOR);
        offset += VertexDecl::get_elem_size(VDET_FLOAT4);
    }

    if (has_texcoords)
    {
        decl->add(offset, VDET_FLOAT2, VDES_TEXCOORD);
        offset += VertexDecl::get_elem_size(VDET_FLOAT2);
    }

    m_vertices = dev.create_vertex_buffer(std::move(decl), raw.vertices.size());
    lock_buffer(m_vertices.get(), [&](float* ptr)
    {
        for (size_t i = 0; i < raw.vertices.size(); i++)
        {
            ptr[0] = raw.vertices[i].x();
            ptr[1] = raw.vertices[i].y();
            ptr[2] = raw.vertices[i].z();
            ptr += 3;

            if (has_normals)
            {
                ptr[0] = raw.normals[i].x();
                ptr[1] = raw.normals[i].y();
                ptr[2] = raw.normals[i].z();
                ptr += 3;
            }

            if (has_colors)
            {
                ptr[0] = raw.colors[i].r();
                ptr[1] = raw.colors[i].g();
                ptr[2] = raw.colors[i].b();
                ptr[3] = raw.colors[i].a();
                ptr += 4;
            }

            if (has_texcoords)
            {
                ptr[0] = raw.texcoords[i].x();
                ptr[1] = raw.texcoords[i].y();
                ptr += 2;
            }
        }
    });

    m_indices = dev.create_index_buffer(raw.indices.size());
    lock_buffer(m_indices.get(), [&](uint16_t* ptr)
    {
        std::copy(raw.indices.begin(), raw.indices.end(), ptr);
    });

    log_info("Created mesh name = %s, id = %#x", raw.name.c_str(), this);
}

Mesh::~Mesh()
{}

RenderPrimitive Mesh::get_primitive() const
{
    return RenderPrimitive(*m_vertices, *m_indices);
}
