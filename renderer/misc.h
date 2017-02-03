#pragma once

namespace detail
{
    template <typename Func, typename R, typename Arg, typename... Rest>
    Arg first_arg_helper(R (Func::*)(Arg, Rest...));

    template <typename Func, typename R, typename Arg, typename... Rest>
    Arg first_arg_helper(R (Func::*)(Arg, Rest...) const);

    template <typename Func, typename R, typename... Args>
    std::tuple<Args...> all_arg_helper(R (Func::*)(Args...));

    template <typename Func, typename R, typename... Args>
    std::tuple<Args...> all_arg_helper(R (Func::*)(Args...) const);
}

template <typename Func>
using first_arg_t = decltype(detail::first_arg_helper(&Func::operator()));

template <typename Func>
using all_arg_t = decltype(detail::all_arg_helper(&Func::operator()));

// TODO: find a better name for this
template <typename T, typename Func, typename FuncArgs = all_arg_t<Func>>
class dirty_t
{
public:
    // TODO: check neg case
    template <
        typename... Args,
        typename = std::enable_if_t<
            std::is_convertible<std::tuple<Args...>, FuncArgs>::value
        >
    >
    dirty_t(const Args&... args);

    void set_dirty();
    const T& get();

private:
    template <size_t... I>
    void refresh(std::index_sequence<I...>);

private:
    bool m_dirty;
    T m_data;
    FuncArgs m_args;
};

template <typename T, typename Func, typename FuncArgs>
template <typename... Args, typename>
inline dirty_t<T, Func, FuncArgs>::dirty_t(const Args&... args) :
    m_dirty(false),
    m_args { args... }
{}

template <typename T, typename Func, typename FuncArgs>
inline void dirty_t<T, Func, FuncArgs>::set_dirty()
{
    m_dirty = true;
}

template <typename T, typename Func, typename FuncArgs>
inline const T& dirty_t<T, Func, FuncArgs>::get()
{
    if (m_dirty)
    {
        constexpr size_t size = std::tuple_size<FuncArgs>::value;
        refresh(std::make_index_sequence<size>{});
    }
    return m_data;
}

template <typename T, typename Func, typename FuncArgs>
template <size_t... I>
inline void dirty_t<T, Func, FuncArgs>::refresh(std::index_sequence<I...>)
{
    m_data = Func {}(std::get<I>(m_args)...);
    m_dirty = false;
}
