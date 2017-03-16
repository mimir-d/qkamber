
#include "precompiled.h"
#include "mesh.h"

#include "render/render_system.h"
#include "render/render_buffers.h"
#include "math3.h"

Mesh::Mesh(const GeometryAsset::Object& object, RenderDevice& dev)
{
    flog("id = %#x", this);

    bool has_colors = object.colors.size() > 0;
    bool has_texcoords = object.texcoords.size() > 0;

    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    size_t offset = 0;

    decl->add(offset, VDET_FLOAT3, VDES_POSITION);
    offset += VertexDecl::get_elem_size(VDET_FLOAT3);

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

#define MESH_

// TODO: source with dep inj?
#ifdef MESH_CUBE
Mesh::Mesh(RenderDevice& dev)
{
    flog("id = %#x", this);

    // hardcode a cube for now
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);
    decl->add(3 * sizeof(float), VDET_FLOAT3, VDES_COLOR);
    decl->add(6 * sizeof(float), VDET_FLOAT2, VDES_TEXCOORD);

    vec3 vertices[] =
    {
        { -1.0f,  1.0f,  1.0f },
        {  1.0f,  1.0f,  1.0f },
        { -1.0f, -1.0f,  1.0f },
        {  1.0f, -1.0f,  1.0f },

        { -1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f }
    };

    vec3 colors[] =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },

        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.5f },
        { 0.0f, 0.5f, 0.0f },
        { 0.5f, 0.0f, 0.0f },
    };

    vec2 uvs[] =
    {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },

        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f }
    };

    const size_t vertex_count = sizeof(vertices) / sizeof(vertices[0]);

    m_vertices = dev.create_vertex_buffer(std::move(decl), vertex_count);
    lock_buffer(m_vertices.get(), [&vertices, &colors, &uvs, &vertex_count](float* ptr)
    {
        for (size_t i = 0; i < vertex_count; i++)
        {
            ptr[0] = vertices[i].x();
            ptr[1] = vertices[i].y();
            ptr[2] = vertices[i].z();
            ptr += 3;

            ptr[0] = colors[i].x();
            ptr[1] = colors[i].y();
            ptr[2] = colors[i].z();
            ptr += 3;

            ptr[0] = uvs[i].x();
            ptr[1] = uvs[i].y();
            ptr += 2;
        }
    });

    const uint16_t indices[] =
    {
        0, 2, 1, 1, 2, 3, // front
        1, 3, 5, 5, 3, 7, // right
        4, 0, 5, 5, 0, 1, // top
        4, 6, 0, 0, 6, 2, // left
        5, 7, 4, 4, 7, 6, // back
        2, 6, 3, 3, 6, 7  // bottom
    };
    const size_t index_count = sizeof(indices) / sizeof(indices[0]);

    m_indices = dev.create_index_buffer(index_count);
    lock_buffer(m_indices.get(), [&indices, &index_count](uint16_t* ptr)
    {
        for (size_t i = 0; i < index_count; i++)
        {
            *ptr = indices[i];
            ptr ++;
        }
    });

    log_info("Created mesh %#x", this);
}
#elif defined MESH_CUBE3
Mesh::Mesh(RenderDevice& dev)
{
    flog("id = %#x", this);

    // hardcode a cube for now
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);
    decl->add(3 * sizeof(float), VDET_FLOAT3, VDES_COLOR);
    decl->add(6 * sizeof(float), VDET_FLOAT2, VDES_TEXCOORD);

    const size_t face_indices[] =
    {
        0, 1, 2, 3, // front
        1, 5, 3, 7, // right
        5, 4, 7, 6, // back
        4, 0, 6, 2, // left
        4, 5, 0, 1, // top
        2, 3, 6, 7  // bottom
    };

    const vec3 vertices[] =
    {
        { -1.0f,  1.0f,  1.0f },
        { 1.0f,  1.0f,  1.0f },
        { -1.0f, -1.0f,  1.0f },
        { 1.0f, -1.0f,  1.0f },

        { -1.0f,  1.0f, -1.0f },
        { 1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f }
    };

    const vec3 colors[] =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },

        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.5f },
        { 0.0f, 0.5f, 0.0f },
        { 0.5f, 0.0f, 0.0f },
    };

    vec2 uvs[] =
    {
        // all faces have same uv
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f }
    };

    const size_t vertex_count = sizeof(face_indices) / sizeof(face_indices[0]);

    m_vertices = dev.create_vertex_buffer(std::move(decl), vertex_count);
    lock_buffer(m_vertices.get(), [&](float* ptr)
    {
        for (size_t i = 0; i < vertex_count; i++)
        {
            const size_t fi = face_indices[i];
            ptr[0] = vertices[fi].x();
            ptr[1] = vertices[fi].y();
            ptr[2] = vertices[fi].z();
            ptr += 3;

            // if (i % 4 == 0)
            // {
            //     vec3 v1 = vertices[face_indices[i + 1]] - vertices[face_indices[i]];
            //     vec3 v2 = vertices[face_indices[i + 2]] - vertices[face_indices[i]];
            //     vec3 n = (v1 ^ v2).normalize();
            // }

            ptr[0] = colors[fi].x();
            ptr[1] = colors[fi].y();
            ptr[2] = colors[fi].z();
            ptr += 3;

            ptr[0] = uvs[i % 4].x();
            ptr[1] = uvs[i % 4].y();
            ptr += 2;
        }
    });

    const uint16_t indices[] = { 0,  2,  1,  1,  2,  3 };
    const size_t face_index_count = sizeof(indices) / sizeof(indices[0]);
    const size_t index_count = 6 * face_index_count;

    m_indices = dev.create_index_buffer(index_count);
    lock_buffer(m_indices.get(), [&](uint16_t* ptr)
    {
        for (size_t i = 0; i < index_count; i++)
        {
            const size_t fi = i / 6;
            *ptr = static_cast<uint16_t>(4*fi) + indices[i % face_index_count];
            ptr ++;
        }
    });

    log_info("Created mesh %#x", this);
}
#elif defined MESH_TRI
Mesh::Mesh(RenderDevice& dev)
{
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);
    decl->add(3 * sizeof(float), VDET_FLOAT3, VDES_COLOR);

    vec3 vertices[] =
    {
        { -1.0f, -0.5773f, 0.0f },
        { 0.0f,   1.1547f, 0.0f },
        { 1.0f,  -0.5773f, 0.0f }
    };

    vec3 colors[] =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    const size_t vertex_count = sizeof(vertices) / sizeof(vertices[0]);

    m_vertices = dev.create_vertex_buffer(std::move(decl), vertex_count);
    lock_buffer(m_vertices.get(), [&vertices, &colors, &vertex_count](float* ptr)
    {
        for (size_t i = 0; i < vertex_count; i++)
        {
            ptr[0] = vertices[i].x();
            ptr[1] = vertices[i].y();
            ptr[2] = vertices[i].z();
            ptr += 3;

            ptr[0] = colors[i].x();
            ptr[1] = colors[i].y();
            ptr[2] = colors[i].z();
            ptr += 3;
        }
    });

    const uint16_t indices[] = {
        0, 2, 1
    };
    const size_t index_count = sizeof(indices) / sizeof(indices[0]);

    m_indices = dev.create_index_buffer(index_count);
    lock_buffer(m_indices.get(), [&indices, &index_count](uint16_t* ptr)
    {
        for (size_t i = 0; i < index_count; i++)
        {
            *ptr = indices[i];
            ptr ++;
        }
    });
}
#elif defined(MESH_AXIS)
Mesh::Mesh(RenderDevice& dev)
{
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);
    decl->add(3 * sizeof(float), VDET_FLOAT3, VDES_COLOR);
    decl->add(6 * sizeof(float), VDET_FLOAT2, VDES_TEXCOORD);

    vec3 vertices[] =
    {
        { 0.0f, 0.0f, 0.0f },
        { 2.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    };

    vec3 colors[] =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    vec2 uvs[] =
    {
        { 0, 0 },
        { 1, 0 },
        { 0, 1 }
    };

    const size_t vertex_count = sizeof(vertices) / sizeof(vertices[0]);

    m_vertices = dev.create_vertex_buffer(std::move(decl), vertex_count);
    lock_buffer(m_vertices.get(), [&vertices, &colors, &uvs, &vertex_count](float* ptr)
    {
        for (size_t i = 0; i < vertex_count; i++)
        {
            ptr[0] = vertices[i].x();
            ptr[1] = vertices[i].y();
            ptr[2] = vertices[i].z();
            ptr += 3;

            ptr[0] = colors[i].x();
            ptr[1] = colors[i].y();
            ptr[2] = colors[i].z();
            ptr += 3;

            ptr[0] = uvs[i].x();
            ptr[1] = uvs[i].y();
            ptr += 2;
        }
    });

    const uint16_t indices[] = {
        0, 1, 2
    };
    const size_t index_count = sizeof(indices) / sizeof(indices[0]);

    m_indices = dev.create_index_buffer(index_count);
    lock_buffer(m_indices.get(), [&indices, &index_count](uint16_t* ptr)
    {
        for (size_t i = 0; i < index_count; i++)
        {
            *ptr = indices[i];
            ptr ++;
        }
    });
}
#endif

Mesh::~Mesh()
{}

RenderPrimitive Mesh::get_primitive() const
{
    return RenderPrimitive(*m_vertices, *m_indices);
}
