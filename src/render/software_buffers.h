#pragma once

#include "render_buffers.h"
#include "asset/asset_system.h"
#include "math3.h"

namespace detail
{
    template <typename Buffer, typename T>
    class BufferStorage : public Buffer
    {
    public:
        T* lock() override;
        void unlock() override;

        const T* data() const;

    protected:
        template <typename... Args>
        BufferStorage(size_t size, Args&&... args);
        ~BufferStorage() = default;

        std::unique_ptr<T[]> m_data;
    };
}

class SoftwareDepthBuffer : public detail::BufferStorage<DepthBuffer, float>
{
public:
    SoftwareDepthBuffer(int width, int height);
    ~SoftwareDepthBuffer() = default;

    size_t get_stride() final;

    void resize(int width, int height);
    void clear();

private:
    size_t m_width = 0;
    size_t m_height = 0;
};

class SoftwareVertexBuffer : public detail::BufferStorage<VertexBuffer, uint8_t>
{
public:
    SoftwareVertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count);
    ~SoftwareVertexBuffer() = default;
};

class SoftwareIndexBuffer : public detail::BufferStorage<IndexBuffer, uint8_t>
{
public:
    SoftwareIndexBuffer(size_t count);
    ~SoftwareIndexBuffer() = default;
};

class SoftwareTexture : public detail::BufferStorage<Texture, uint8_t>
{
public:
    SoftwareTexture(size_t width, size_t height, PixelFormat format);
    ~SoftwareTexture() = default;

    uint8_t* lock() final;
    void unlock() final;

    size_t get_width() const final;
    size_t get_height() const final;
    PixelFormat get_format() const final;

    Color sample(float u, float v) const;

private:
    size_t m_width, m_height;
    PixelFormat m_format;
    std::vector<uint8_t> m_rgb8u_data;
};

///////////////////////////////////////////////////////////////////////////////
// detail::BufferStorage impl
///////////////////////////////////////////////////////////////////////////////
template <typename Buffer, typename T>
template <typename... Args>
inline detail::BufferStorage<Buffer, T>::BufferStorage(size_t size, Args&&... args) :
    Buffer{ std::forward<Args>(args)... }
{
    m_data.reset(new T[size]);
}

template <typename Buffer, typename T>
inline T* detail::BufferStorage<Buffer, T>::lock()
{
    return m_data.get();
}

template <typename Buffer, typename T>
inline void detail::BufferStorage<Buffer, T>::unlock()
{}

template <typename Buffer, typename T>
inline const T* detail::BufferStorage<Buffer, T>::data() const
{
    return m_data.get();
}

///////////////////////////////////////////////////////////////////////////////
// SoftwareDepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline size_t SoftwareDepthBuffer::get_stride()
{
    return m_width;
}

///////////////////////////////////////////////////////////////////////////////
// SoftwareVertexBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline SoftwareVertexBuffer::SoftwareVertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count) :
    BufferStorage(decl->get_vertex_size() * count, std::move(decl), count)
{}

///////////////////////////////////////////////////////////////////////////////
// SoftwareIndexBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline SoftwareIndexBuffer::SoftwareIndexBuffer(size_t count) :
    BufferStorage(sizeof(uint16_t) * count, count)
{}

///////////////////////////////////////////////////////////////////////////////
// SoftwareTexture impl
///////////////////////////////////////////////////////////////////////////////
inline size_t SoftwareTexture::get_width() const
{
    return m_width;
}

inline size_t SoftwareTexture::get_height() const
{
    return m_height;
}

inline PixelFormat SoftwareTexture::get_format() const
{
    return m_format;
}

inline Color SoftwareTexture::sample(float u, float v) const
{
    const size_t x = clamp(static_cast<size_t>(u * m_width), size_t(0), m_width - 1);
    const size_t y = clamp(static_cast<size_t>(v * m_height), size_t(0), m_height - 1);
    const uint8_t* data = m_data.get() + (y * m_width + x) * 4;
    return Color{ data[0] / 255.0f, data[1] / 255.0f, data[2] / 255.0f, data[3] / 255.0f };
}
