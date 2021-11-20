#pragma once

#include "subsystem.h"
#include "engine.h"
#include "component_store.h"

class EntityConfig;

class EntitySystem : public Subsystem
{
    using eid_t = uintptr_t;

    template <bool is_const, typename... Components>
    class filter_t
    {
        using es_t = typename std::conditional<is_const, const EntitySystem&, EntitySystem&>::type;

    public:
        class iter_t
        {
            using value_t = typename std::conditional<
                is_const,
                std::tuple<const Components&...>,
                std::tuple<Components&...>
            >::type;

        public:
            iter_t(es_t es, uint32_t mask, size_t index, size_t size);
            ~iter_t() = default;

            bool operator==(const iter_t& rhs);
            bool operator!=(const iter_t& rhs);

            iter_t& operator++();
            value_t operator*();

        private:
            void advance(uint32_t offset = 1);

            es_t m_es;
            uint32_t m_mask;
            size_t m_index, m_size;
        };

    public:
        filter_t(es_t es);
        ~filter_t() = default;

        iter_t begin();
        iter_t end();

    private:
        es_t m_es;
        uint32_t m_mask;
    };

    template <typename... Components>
    using filter = filter_t<false, Components...>;

    template <typename... Components>
    using const_filter = filter_t<true, Components...>;

    struct private_tag {};

public:
    class Entity
    {
    public:
        // NOTE: only allow EntitySystem to create these objects
        Entity(EntitySystem& parent, eid_t id, private_tag);
        ~Entity() = default;

        template <typename Component>
        Component& add_component();

        template <typename Component>
        Component& get_component();

        template <typename Component>
        const Component& get_component() const;

    private:
        eid_t m_id;
        EntitySystem& m_parent;
    };

public:
    EntitySystem(QkEngine::Context& context);
    ~EntitySystem();

    void process() final {}

    // TODO: create delete + freelist
    std::unique_ptr<Entity> create_entity(const std::string& name);

    template <typename... Components>
    filter<Components...> filter_comp();

    template <typename... Components>
    const_filter<Components...> filter_comp() const;

private:
    template <typename Component>
    Component& add_component(eid_t id);

    template <typename Component>
    Component& get_component(eid_t id);

    template <typename Component>
    const Component& get_component(eid_t id) const;

    eid_t get_next_id();

private:
    ComponentStore m_store;
    ComponentMask m_mask;

    std::unique_ptr<EntityConfig> m_config;
    eid_t m_last_id = 0;
};

///////////////////////////////////////////////////////////////////////////////
// EntitySystem::filter_t::iter_t impl
///////////////////////////////////////////////////////////////////////////////
template <bool is_const, typename... Components>
inline EntitySystem::filter_t<is_const, Components...>::iter_t::iter_t(
    es_t es, uint32_t mask,
    size_t index, size_t size
) :
    m_es(es),
    m_mask(mask),
    m_index(index), m_size(size)
{
    // advance iterator until first mask match or end
    advance(0);
}

template <bool is_const, typename... Components>
inline bool EntitySystem::filter_t<is_const, Components...>::iter_t::operator==(const iter_t& rhs)
{
    return m_index == rhs.m_index && m_size == rhs.m_size;
}

template <bool is_const, typename... Components>
inline bool EntitySystem::filter_t<is_const, Components...>::iter_t::operator!=(const iter_t& rhs)
{
    return m_index != rhs.m_index || m_size != rhs.m_size;
}

template <bool is_const, typename... Components>
inline typename EntitySystem::filter_t<is_const, Components...>::iter_t&
EntitySystem::filter_t<is_const, Components...>::iter_t::operator++()
{
    advance(1);
    return *this;
}

template <bool is_const, typename... Components>
inline typename EntitySystem::filter_t<is_const, Components...>::iter_t::value_t
EntitySystem::filter_t<is_const, Components...>::iter_t::operator*()
{
    return make_tuple(std::ref(m_es.m_store.template get<Components>()[m_index])...);
}

template <bool is_const, typename... Components>
void EntitySystem::filter_t<is_const, Components...>::iter_t::advance(uint32_t offset)
{
    for (m_index += offset; m_index < m_size && (m_es.m_mask[m_index] & m_mask) != m_mask; m_index++);
}

///////////////////////////////////////////////////////////////////////////////
// EntitySystem::filter_t impl
///////////////////////////////////////////////////////////////////////////////
template <bool is_const, typename... Components>
inline EntitySystem::filter_t<is_const, Components...>::filter_t(es_t es) :
    m_es(es)
{
    using swallow = int[];

    m_mask = 0;
    (void)swallow{ (m_mask |= es.m_mask.template get_mask<Components>(), 0)... };
}

template <bool is_const, typename... Components>
inline typename EntitySystem::filter_t<is_const, Components...>::iter_t
EntitySystem::filter_t<is_const, Components...>::begin()
{
    return iter_t{ m_es, m_mask, 0, m_es.m_mask.size() };
}

template <bool is_const, typename... Components>
inline typename EntitySystem::filter_t<is_const, Components...>::iter_t
EntitySystem::filter_t<is_const, Components...>::end()
{
    return iter_t{ m_es, m_mask, m_es.m_mask.size(), m_es.m_mask.size() };
}

///////////////////////////////////////////////////////////////////////////////
// Entity impl
///////////////////////////////////////////////////////////////////////////////
inline EntitySystem::Entity::Entity(EntitySystem& parent, eid_t id, private_tag) :
    m_parent(parent),
    m_id(id)
{}

template <typename Component>
inline Component& EntitySystem::Entity::add_component()
{
    return m_parent.add_component<Component>(m_id);
}

template <typename Component>
inline Component& EntitySystem::Entity::get_component()
{
    return m_parent.get_component<Component>(m_id);
}

template <typename Component>
inline const Component& EntitySystem::Entity::get_component() const
{
    return m_parent.get_component<Component>(m_id);
}

///////////////////////////////////////////////////////////////////////////////
// EntitySystem impl
///////////////////////////////////////////////////////////////////////////////
template <typename... Components>
inline EntitySystem::filter<Components...> EntitySystem::filter_comp()
{
    return filter<Components...>{ *this };
}

template <typename... Components>
inline EntitySystem::const_filter<Components...> EntitySystem::filter_comp() const
{
    return const_filter<Components...>{ *this };
}

template <typename Component>
inline Component& EntitySystem::add_component(eid_t id)
{
    m_mask[id] |= m_mask.get_mask<Component>();
    return m_store.get<Component>()[static_cast<int>(id)];
}

template <typename Component>
inline Component& EntitySystem::get_component(eid_t id)
{
    if (!(m_mask[id] & m_mask.get_mask<Component>()))
        throw std::runtime_error("no such component");

    return m_store.get<Component>()[static_cast<int>(id)];
}

template <typename Component>
inline const Component& EntitySystem::get_component(eid_t id) const
{
    if (!(m_mask[id] & m_mask.get_mask<Component>()))
        throw std::runtime_error("no such component");

    return m_store.get<Component>()[static_cast<int>(id)];
}

inline EntitySystem::eid_t EntitySystem::get_next_id()
{
    return m_last_id ++;
}
