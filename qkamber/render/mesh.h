#pragma once

#include "render/render_primitive.h"

class RenderDevice;
class VertexBuffer;
class IndexBuffer;

class Mesh
{
public:
    Mesh(RenderDevice& dev);
    ~Mesh();

    RenderPrimitive get_primitive() const;

private:
    std::unique_ptr<VertexBuffer> m_vertices;
    std::unique_ptr<IndexBuffer> m_indices;
};