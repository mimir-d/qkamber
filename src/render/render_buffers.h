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

class RenderTarget
{
public:
    virtual ~RenderTarget() = default;

    // NOTE: this always has these 2 attachments (should provide more in the future)
    virtual ColorBuffer& get_color_buffer() = 0;
    virtual DepthBuffer& get_depth_buffer() = 0;

    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
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
enum class VertexSemantic
{
    Position,
    Normal,
    Color,
    Texcoord
};

enum class VertexType
{
    Float2,
    Float3,
    // rgba color
    Color
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
    void add(VertexType type, VertexSemantic semantic);
    iterator begin() const;
    iterator end() const;

    size_t get_vertex_size() const;

private:
    static size_t get_elem_size(VertexType type);

private:
    std::vector<Element> m_elems;
    size_t m_offset = 0;
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
// Texture
///////////////////////////////////////////////////////////////////////////////
enum class PixelFormat
{
    RgbaU8,
    RgbU8
};

class Texture : public DeviceBuffer
{
public:
    virtual size_t get_width() const = 0;
    virtual size_t get_height() const = 0;
    virtual PixelFormat get_format() const = 0;

    static size_t get_elem_size(PixelFormat format);
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
inline void VertexDecl::add(VertexType type, VertexSemantic semantic)
{
    m_elems.emplace_back(m_offset, type, semantic);
    m_offset += get_elem_size(type);
}

inline VertexDecl::iterator VertexDecl::begin() const
{
    return m_elems.begin();
}

inline VertexDecl::iterator VertexDecl::end() const
{
    return m_elems.end();
}

inline size_t VertexDecl::get_vertex_size() const
{
    return std::accumulate(
        m_elems.begin(), m_elems.end(),
        size_t(0), [](size_t v, auto& e) { return v + get_elem_size(e.type); }
    );
}

inline size_t VertexDecl::get_elem_size(VertexType type)
{
    switch (type)
    {
        case VertexType::Float2:
            return 2 * sizeof(float);

        case VertexType::Float3:
            return 3 * sizeof(float);

        case VertexType::Color:
            return 4 * sizeof(float);
    }
    throw std::runtime_error("unknown vertex decl element");
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

///////////////////////////////////////////////////////////////////////////////
// Texture impl
///////////////////////////////////////////////////////////////////////////////
inline size_t Texture::get_elem_size(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat::RgbaU8: return 4;
        case PixelFormat::RgbU8: return 3;
    }
    throw std::runtime_error("unknown pixel format");
}
