#pragma once

class DeviceBuffer
{
public:
    DeviceBuffer(size_t size);

    uint8_t* lock();
    void unlock();

protected:
    std::vector<uint8_t> m_data;
};

template <typename T, typename Func>
inline void lock_buffer(DeviceBuffer* buffer, Func fun);

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer
///////////////////////////////////////////////////////////////////////////////
enum VertexSemantic
{
    VDES_POSITION,
    VDES_COLOR
};

enum VertexType
{
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
    VertexDecl() {}

    void add(size_t offset, VertexType type, VertexSemantic semantic);
    iterator begin();
    iterator end();

    size_t get_vertex_size() const;

private:
    size_t get_elem_size(VertexType type) const;

private:
    std::vector<Element> m_elems;
};

class VertexBuffer : public DeviceBuffer
{
public:
    VertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count);

    VertexDecl& get_declaration();
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

private:
    size_t m_count;
};

///////////////////////////////////////////////////////////////////////////////
// DeviceBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline DeviceBuffer::DeviceBuffer(size_t size)
{
    m_data.resize(size);
}

inline uint8_t* DeviceBuffer::lock()
{
    return m_data.data();
}

inline void DeviceBuffer::unlock()
{}

namespace detail
{
    // TODO: might need to extract this somewher else
    template<typename Func, typename R, typename Arg, typename... Rest>
    Arg first_arg_helper(R (Func::*)(Arg, Rest...));

    template<typename Func, typename R, typename Arg, typename... Rest>
    Arg first_arg_helper(R (Func::*)(Arg, Rest...) const);

    template <typename Func>
    using first_arg_t = decltype(first_arg_helper(&Func::operator()));
}

template <typename Func>
inline void lock_buffer(DeviceBuffer* buffer, Func fun)
{
    fun(reinterpret_cast<detail::first_arg_t<Func>>(buffer->lock()));
    buffer->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline VertexBuffer::VertexBuffer(std::unique_ptr<VertexDecl> decl, size_t count) :
    DeviceBuffer(decl->get_vertex_size() * count),
    m_decl(std::move(decl)),
    m_count(count)
{}

inline VertexDecl& VertexBuffer::get_declaration()
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
    DeviceBuffer(count * sizeof(uint16_t)),
    m_count(count)
{}

inline size_t IndexBuffer::get_count() const
{
    return m_count;
}