#pragma once

#include "srt_component.h"
#include "model_component.h"
#include "misc.h"

namespace detail
{
    template <typename Component>
    struct ComponentId : typelist_index<
        Component,
        SrtComponent,
        ModelComponent
    >{};

    template <typename... Components>
    class ComponentStoreImpl
    {
    public:
        template <typename Component>
        std::vector<Component>& get();

        template <typename Component>
        const std::vector<Component>& get() const;

        void resize(size_t size);

    private:
        template <size_t... I>
        void resize(size_t size, std::index_sequence<I...>);

        std::tuple<std::vector<Components>...> m_data;
    };
}

using ComponentStore = detail::ComponentStoreImpl<
    SrtComponent,
    ModelComponent
>;

// store the bit-or component id to show which components are valid for each entity
class ComponentMask : public std::vector<uint32_t>
{
public:
    template <typename Component>
    constexpr static uint32_t get_mask();
};

///////////////////////////////////////////////////////////////////////////////
// ComponentStoreImpl impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename... Components>
    template <typename Component>
    inline std::vector<Component>& ComponentStoreImpl<Components...>::get()
    {
        constexpr int comp_id = ComponentId<Component>::value;
        return std::get<comp_id>(m_data);
    }

    template <typename... Components>
    template <typename Component>
    inline const std::vector<Component>& ComponentStoreImpl<Components...>::get() const
    {
        constexpr int comp_id = ComponentId<Component>::value;
        return std::get<comp_id>(m_data);
    }

    template <typename... Components>
    inline void ComponentStoreImpl<Components...>::resize(size_t size)
    {
        resize(size, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <size_t... I>
    inline void ComponentStoreImpl<Components...>::resize(size_t size, std::index_sequence<I...>)
    {
        using swallow = int[];
        (void)swallow{ (std::get<I>(m_data).resize(size), 0)... };
    }
}

///////////////////////////////////////////////////////////////////////////////
// ComponentMask impl
///////////////////////////////////////////////////////////////////////////////
template <typename Component>
constexpr uint32_t ComponentMask::get_mask()
{
    return 1 << detail::ComponentId<Component>::value;
}
