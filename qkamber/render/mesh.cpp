
#include "precompiled.h"
#include "mesh.h"

#include "render/render_system.h"
#include "render/render_buffers.h"
#include "math3.h"

Mesh::Mesh(const GeometryAsset::Object& object, RenderDevice& dev)
{
    flog("id = %#x", this);

    bool has_normals = object.normals.size() > 0;
    bool has_colors = object.colors.size() > 0;
    bool has_texcoords = object.texcoords.size() > 0;

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

    m_vertices = dev.create_vertex_buffer(std::move(decl), object.vertices.size());
    lock_buffer(m_vertices.get(), [&](float* ptr)
    {
        for (size_t i = 0; i < object.vertices.size(); i++)
        {
            ptr[0] = object.vertices[i].x();
            ptr[1] = object.vertices[i].y();
            ptr[2] = object.vertices[i].z();
            ptr += 3;

            if (has_normals)
            {
                ptr[0] = object.normals[i].x();
                ptr[1] = object.normals[i].y();
                ptr[2] = object.normals[i].z();
                ptr += 3;
            }

            if (has_colors)
            {
                ptr[0] = object.colors[i].r();
                ptr[1] = object.colors[i].g();
                ptr[2] = object.colors[i].b();
                ptr[3] = object.colors[i].a();
                ptr += 4;
            }

            if (has_texcoords)
            {
                ptr[0] = object.texcoords[i].x();
                ptr[1] = object.texcoords[i].y();
                ptr += 2;
            }
        }
    });

    m_indices = dev.create_index_buffer(object.indices.size());
    lock_buffer(m_indices.get(), [&](uint16_t* ptr)
    {
        std::copy(object.indices.begin(), object.indices.end(), ptr);
    });

    log_info("Created mesh %#x", this);
}

Mesh::~Mesh()
{}

RenderPrimitive Mesh::get_primitive() const
{
    return RenderPrimitive(*m_vertices, *m_indices);
}
