
#include "stdafx.h"
#include "render_buffers.h"

void VertexDecl::add(size_t offset, VertexType type, VertexSemantic semantic)
{
    m_elems.emplace_back(offset, type, semantic);
}

VertexDecl::iterator VertexDecl::begin()
{
    return m_elems.begin();
}

VertexDecl::iterator VertexDecl::end()
{
    return m_elems.end();
}

size_t VertexDecl::get_vertex_size() const
{
    size_t ret = 0;
    for (auto& elem : m_elems)
        ret += get_elem_size(elem.type);
    return ret;
}

size_t VertexDecl::get_elem_size(VertexType type) const
{
    switch (type)
    {
        case VDET_FLOAT3:
            return 3 * sizeof(float);
    }
    // TODO: this should fail somehow at compile time because reasons
    return 0;
}