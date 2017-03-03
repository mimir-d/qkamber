#pragma once

#include "misc.h"

///////////////////////////////////////////////////////////////////////////////
// Framebuffer objects
///////////////////////////////////////////////////////////////////////////////
class ColorBuffer
{
public:
    virtual ~ColorBuffer() = default;
};

class DepthBuffer
{
public:
    virtual ~DepthBuffer() = default;
};

///////////////////////////////////////////////////////////////////////////////
// DeviceBuffer
///////////////////////////////////////////////////////////////////////////////
class DeviceBuffer
{
public:
    virtual ~DeviceBuffer() = default;

    virtual uint8_t* lock() = 0;
    virtual void unlock() = 0;
};

template <typename T, typename Func>
inline void lock_buffer(DeviceBuffer* buffer, Func fun);

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer
///////////////////////////////////////////////////////////////////////////////
enum VertexSemantic
{
    VDES_POSITION,
    VDES_COLOR,
    VDES_TEXCOORD
};

enum VertexType
{
    VDET_FLOAT2,
    VDET_FLOAT3
};

class VertexDecl
{
public:
    struct Element
    {
        size_t offset;
        VertexType type;
        VertexSemantic semantic;

        Element(size_t offset, VertexType type, VertexSemantic semantic) :
            offset(offset), type(type), semantic(semantic)
        {}
    };

    typedef std::vector<Element>::const_iterator iterator;

public:
    void add(size_t offset, VertexType type, VertexSemantic semantic);
    iterator begin() const;
    iterator end() const;

    size_t get_vertex_size() const;

    static size_t get_elem_size(VertexType type);

private:
    std::vector<Element> m_elems;
};

class VertexBuffer : public DeviceBuffer
{
public:
    VertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count);

    const VertexDecl& get_declaration() const;
    size_t get_count() const;

private:
    std::unique_ptr<VertexDecl> m_decl;
    size_t m_count;
};

///////////////////////////////////////////////////////////////////////////////
// IndexBuffer
///////////////////////////////////////////////////////////////////////////////
class IndexBuffer : public DeviceBuffer
{
public:
    IndexBuffer(size_t size);

    size_t get_count() const;

protected:
    size_t m_count;
};

///////////////////////////////////////////////////////////////////////////////
// DeviceBuffer impl
///////////////////////////////////////////////////////////////////////////////
template <typename Func>
inline void lock_buffer(DeviceBuffer* buffer, Func fun)
{
    fun(reinterpret_cast<first_arg_t<Func>>(buffer->lock()));
    buffer->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// VertexDecl impl
///////////////////////////////////////////////////////////////////////////////
inline void VertexDecl::add(size_t offset, VertexType type, VertexSemantic semantic)
{
    m_elems.emplace_back(offset, type, semantic);
}

inline VertexDecl::iterator VertexDecl::begin() const
{
    return m_elems.begin();
}

inline VertexDecl::iterator VertexDecl::end() const
{
    return m_elems.end();
}

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline VertexBuffer::VertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count) :
    m_decl(std::move(decl)),
    m_count(count)
{}

inline const VertexDecl& VertexBuffer::get_declaration() const
{
    return *m_decl;
}

inline size_t VertexBuffer::get_count() const
{
    return m_count;
}

///////////////////////////////////////////////////////////////////////////////
// IndexBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline IndexBuffer::IndexBuffer(size_t count) :
    m_count(count)
{}

inline size_t IndexBuffer::get_count() const
{
    return m_count;
}