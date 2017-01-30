#pragma once

// TODO: forward decl not working?
#include "render_buffers.h"
#include "render_primitive.h"

class VertexBuffer;
class IndexBuffer;

class Mesh
{
public:
    Mesh();

    RenderPrimitive get_primitive() const;

private:
    std::unique_ptr<VertexBuffer> m_vertices;
    std::unique_ptr<IndexBuffer> m_indices;
};