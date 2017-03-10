#pragma once

// render queue has render groups
// renderer processes queue per frame / iterator
// render group has solids/transparent/etc as render compounds
//

#include "model.h"

class mat4;

class RenderQueue
{
public:
    struct Item
    {
        const mat4& world_matrix;
        const Model::Unit& model_unit;

        Item(const mat4& world_matrix, const Model::Unit& model_unit);
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
inline RenderQueue::Item::Item(const mat4& world_matrix, const Model::Unit& model_unit) :
    world_matrix(world_matrix), model_unit(model_unit)
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
