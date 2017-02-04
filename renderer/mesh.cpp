
#include "stdafx.h"
#include "mesh.h"

#include "renderer.h"
#include "render_buffers.h"
#include "math3.h"

// TODO: source with dep inj?

Mesh::Mesh(RenderDevice& dev)
{
    // hardcode a cube for now
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);
    decl->add(3 * sizeof(float), VDET_FLOAT3, VDES_COLOR);

    vec3 vertices[] =
    {
        vec3(-1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3(-1.0f, -1.0f,  1.0f),
        vec3( 1.0f, -1.0f,  1.0f),

        vec3(-1.0f,  1.0f, -1.0f),
        vec3( 1.0f,  1.0f, -1.0f),
        vec3(-1.0f, -1.0f, -1.0f),
        vec3( 1.0f, -1.0f, -1.0f)
    };

    vec3 colors[] =
    {
        vec3(0.5f, 0.0f, 0.0f),
        vec3(0.0f, 0.5f, 0.0f),
        vec3(0.0f, 0.0f, 0.5f),
        vec3(0.0f, 0.0f, 0.0f),

        vec3(0.8f, 0.0f, 0.0f),
        vec3(0.0f, 0.8f, 0.0f),
        vec3(0.0f, 0.0f, 0.8f),
        vec3(0.3f, 0.3f, 0.3f)
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
}
/*
Mesh::Mesh(RenderDevice& dev)
{
    // hardcode a cube for now
    std::unique_ptr<VertexDecl> decl(new VertexDecl);
    decl->add(0, VDET_FLOAT3, VDES_POSITION);

    vec3 vertices[] =
    {
        {  1, -0.5, 0 },
        {  0,  0.5, 0 },
        { -1, -0.5, 0 }
    };

    const size_t vertex_count = sizeof(vertices) / sizeof(vertices[0]);

    m_vertices = dev.create_vertex_buffer(std::move(decl), vertex_count);
    lock_buffer(m_vertices.get(), [&vertices, &vertex_count](float* ptr)
    {
        for (size_t i = 0; i < vertex_count; i++)
        {
            ptr[0] = vertices[i].x();
            ptr[1] = vertices[i].y();
            ptr[2] = vertices[i].z();
            ptr += 3;
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
*/
RenderPrimitive Mesh::get_primitive() const
{
    return RenderPrimitive(*m_vertices, *m_indices);
}