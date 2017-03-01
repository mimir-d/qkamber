#pragma once

// render queue has render groups
// renderer processes queue per frame / iterator
// render group has solids/transparent/etc as render compounds
//

class mat4;
class Mesh;

class RenderQueue
{
public:
    struct Item
    {
        const mat4& world_matrix;
        const Mesh& mesh;
        // material

        Item(const mat4& world_matrix, const Mesh& mesh);
    };

    using iterator = std::vector<Item>::const_iterator;

public:
    iterator begin();
    iterator end();

    template <typename... Args>
    void add(Args&&... args);
    void clear();

private:
    std::vector<Item> m_items;
};

///////////////////////////////////////////////////////////////////////////////
// RenderQueue::Item impl
///////////////////////////////////////////////////////////////////////////////
inline RenderQueue::Item::Item(const mat4& world_matrix, const Mesh& mesh) :
    world_matrix(world_matrix), mesh(mesh)
{}

///////////////////////////////////////////////////////////////////////////////
// RenderQueue impl
///////////////////////////////////////////////////////////////////////////////
inline RenderQueue::iterator RenderQueue::begin()
{
    return m_items.cbegin();
}

inline RenderQueue::iterator RenderQueue::end()
{
    return m_items.cend();
}

template <typename... Args>
inline void RenderQueue::add(Args&&... args)
{
    m_items.emplace_back(std::forward<Args>(args)...);
}

inline void RenderQueue::clear()
{
    m_items.clear();
}
