#pragma once

#include "render_buffers.h"

namespace detail
{
    class BufferStorage
    {
    public:
        BufferStorage(size_t size);
        ~BufferStorage() = default;

        const uint8_t* data() const;

    protected:
        std::vector<uint8_t> m_data;
    };
}

class SoftwareVertexBuffer : public detail::BufferStorage, public VertexBuffer
{
public:
    SoftwareVertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count);

    // TODO: figure out how to remove this dup code
    uint8_t* lock() final;
    void unlock() final;
};

class SoftwareIndexBuffer : public detail::BufferStorage, public IndexBuffer
{
public:
    SoftwareIndexBuffer(size_t count);

    uint8_t* lock() final;
    void unlock() final;
};

///////////////////////////////////////////////////////////////////////////////
// Impl
///////////////////////////////////////////////////////////////////////////////
inline detail::BufferStorage::BufferStorage(size_t size)
{
    m_data.resize(size);
}

inline const uint8_t* detail::BufferStorage::data() const
{
    return m_data.data();
}

inline SoftwareVertexBuffer::SoftwareVertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count) :
    BufferStorage(decl->get_vertex_size() * count),
    VertexBuffer(std::move(decl), count)
{}

inline uint8_t* SoftwareVertexBuffer::lock()
{
    return m_data.data();
}

inline void SoftwareVertexBuffer::unlock()
{}

inline SoftwareIndexBuffer::SoftwareIndexBuffer(size_t count) :
    BufferStorage(sizeof(uint16_t) * count),
    IndexBuffer(count)
{}

inline uint8_t* SoftwareIndexBuffer::lock()
{
    return m_data.data();
}

inline void SoftwareIndexBuffer::unlock()
{}