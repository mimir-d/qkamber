#pragma once

#define __FILE_SHORT__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __CONCAT(x, y) x ## y
#define __LINE_VARNAME(prefix) __CONCAT(prefix, __LINE__)

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

///////////////////////////////////////////////////////////////////////////////
// dirty_t
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename Func, typename FuncArgs = all_arg_t<Func>>
class dirty_t
{
public:
    template <
        typename... Args,
        typename = std::enable_if_t<
            std::is_convertible<std::tuple<Args...>, FuncArgs>::value
        >
    >
    dirty_t(const Args&... args);
    ~dirty_t() = default;

    // non-copyable and non-assignable
    dirty_t(const dirty_t&) = delete;
    dirty_t& operator=(const dirty_t&) = delete;

    void set_dirty();
    const T& get();

private:
    template <size_t... I>
    void refresh(std::index_sequence<I...>);

private:
    bool m_dirty = true;
    T m_data;
    FuncArgs m_args;
};

template <typename T, typename Func, typename FuncArgs>
template <typename... Args, typename>
inline dirty_t<T, Func, FuncArgs>::dirty_t(const Args&... args) :
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

///////////////////////////////////////////////////////////////////////////////
// repeat_t
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template<typename T, std::size_t I>
    using RepeatT = T;

    template<typename T, std::size_t N, typename I = std::make_index_sequence<N>>
    struct repeat_type;

    template<typename T, std::size_t N, std::size_t... I>
    struct repeat_type<T, N, std::index_sequence<I...>>
    {
        using type = std::tuple<RepeatT<T, I>...>;
    };
}

template <typename T, std::size_t N>
using repeat_t = typename detail::repeat_type<T, N>::type;

///////////////////////////////////////////////////////////////////////////////
// typelist_index
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename... Ts>
struct typelist_index;

template <typename T, typename... Rest>
struct typelist_index<T, T, Rest...> :
    std::integral_constant<std::size_t, 0>
{};

template <typename T, typename T0, typename... Rest>
struct typelist_index<T, T0, Rest...> :
    std::integral_constant<std::size_t, 1 + typelist_index<T, Rest...>::value>
{};

///////////////////////////////////////////////////////////////////////////////
// app_clock
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename _ = void>
    class app_clock_impl
    {
        using BaseClock = std::chrono::high_resolution_clock;

    public:
        using duration   = BaseClock::duration;
        using rep        = duration::rep;
        using period     = duration::period;
        using time_point = std::chrono::time_point<app_clock_impl>;
        constexpr static bool is_steady = BaseClock::is_steady;

        static time_point now() noexcept
        {
            return time_point { std::chrono::duration_cast<duration>(BaseClock::now() - start) };
        }

    private:
        static BaseClock::time_point start;
    };

    // NOTE: initializes the clock on static variable construction
    // This essentially creates a 0 epoch on application start
    template <typename _>
    app_clock_impl<_>::BaseClock::time_point app_clock_impl<_>::start = app_clock_impl::BaseClock::now();
}

using app_clock = detail::app_clock_impl<>;

// NOTE: stolen from http://stackoverflow.com/questions/42138599/how-to-format-stdchrono-durations
template <typename... Durations, class DurationIn>
inline std::tuple<Durations...> duration_parts(DurationIn d)
{
    std::tuple<Durations...> ret;
    using swallow = int[];
    (void)swallow
    {
        (
            std::get<Durations>(ret) = std::chrono::duration_cast<Durations>(d),
            d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(ret)),
            0
        )...
    };
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// print_fmt
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    /*
    // TODO: impl type safety and other stuff
    inline void print_fmt_impl(std::ostream& os, const char* fmt)
    {
        while (*fmt)
        {
            if (*fmt == '%')
            {
                if (*(fmt + 1) == '%')
                {
                    ++fmt;
                }
                else
                {
                    throw std::exception("print_fmt invalid format string: missing arguments");
                }
            }
            os << *fmt++;
        }
    }

    template <typename T, typename... Args>
    inline void print_fmt_impl(std::ostream& os, const char* fmt, const T& value, const Args&... args)
    {
        while (*fmt)
        {
            if (*fmt == '%')
            {
                if (*(fmt + 1) == '%')
                {
                    ++fmt;
                }
                else
                {
                    os << value;
                    print_fmt_impl(os, fmt + 2, args...);
                    return;
                }
            }
            os << *fmt++;
        }
        throw std::exception("print_fmt invalid format string: extra arguments");
    }
    */
}

template <typename... Args>
inline std::string print_fmt(const std::string& fmt, const Args&... args)
{
    int size = snprintf(nullptr, 0, fmt.c_str(), args...);
    std::unique_ptr<char[]> buf(new char[size + 1]);
    snprintf(buf.get(), size + 1, fmt.c_str(), args...);
    return std::string(buf.get());
}
