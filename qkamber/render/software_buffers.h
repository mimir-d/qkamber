#pragma once

#include "render_buffers.h"
#include "asset/asset_system.h"

namespace detail
{
    template <typename Buffer>
    class BufferStorage : public Buffer
    {
    public:
        uint8_t* lock() final;
        void unlock() final;

        const uint8_t* data() const;

    protected:
        template <typename... Args>
        BufferStorage(size_t size, Args&&... args);
        ~BufferStorage() = default;

        // TODO: a const-size vector after init
        std::vector<uint8_t> m_data;
    };
}

class SoftwareVertexBuffer : public detail::BufferStorage<VertexBuffer>
{
public:
    SoftwareVertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count);
    ~SoftwareVertexBuffer() = default;
};

class SoftwareIndexBuffer : public detail::BufferStorage<IndexBuffer>
{
public:
    SoftwareIndexBuffer(size_t count);
    ~SoftwareIndexBuffer() = default;
};

class SoftwareTexture : public detail::BufferStorage<Texture>
{
public:
    SoftwareTexture(Image* image);
    ~SoftwareTexture() = default;

    size_t get_width() const final;
    size_t get_height() const final;
    PixelFormat get_format() const final;

    const uint8_t* sample(float u, float v) const;

private:
    size_t m_width, m_height;
    PixelFormat m_format;
};

///////////////////////////////////////////////////////////////////////////////
// detail::BufferStorage impl
///////////////////////////////////////////////////////////////////////////////
template <typename Buffer>
template <typename... Args>
inline detail::BufferStorage<Buffer>::BufferStorage(size_t size, Args&&... args) :
    Buffer{ std::forward<Args>(args)... }
{
    m_data.resize(size);
}

template <typename Buffer>
inline uint8_t* detail::BufferStorage<Buffer>::lock()
{
    return m_data.data();
}

template <typename Buffer>
inline void detail::BufferStorage<Buffer>::unlock()
{}

template <typename Buffer>
inline const uint8_t* detail::BufferStorage<Buffer>::data() const
{
    return m_data.data();
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
inline SoftwareTexture::SoftwareTexture(Image* image) :
    BufferStorage(image->get_width() * image->get_height() * 4),
    m_width(image->get_width()),
    m_height(image->get_height())
{
    // NOTE: simplification for software rasterizer, always convert to RgbaU8
    m_format = PixelFormat::RgbaU8;
    if (image->get_format() == ImageFormat::Rgba8)
    {
        std::copy(image->data(), image->data() + m_data.size(), m_data.data());
        return;
    }

    // this is Rgb8, add in a check for the hell of it
    if (image->get_format() != ImageFormat::Rgb8)
        throw std::exception("invalid image format while trying to create software texture");

    uint8_t* pi = image->data();
    uint8_t* po = m_data.data();
    for (; pi != image->data() + m_height * m_width * 3; pi += 3, po += 4)
    {
        po[0] = pi[0];
        po[1] = pi[1];
        po[2] = pi[2];
        po[3] = 0xff;
    }
}

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

inline const uint8_t* SoftwareTexture::sample(float u, float v) const
{
    const size_t x = clamp(static_cast<size_t>(u * m_width), size_t(0), m_width - 1);
    const size_t y = clamp(static_cast<size_t>(v * m_height), size_t(0), m_height - 1);
    return m_data.data() + (y * m_width + x) * 4;
}
