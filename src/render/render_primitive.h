#pragma once

class VertexBuffer;
class IndexBuffer;

struct RenderPrimitive
{
    VertexBuffer& vertices;
    IndexBuffer& indices;

    RenderPrimitive(VertexBuffer& vertices, IndexBuffer& indices) :
        vertices(vertices), indices(indices)
    {}
};
