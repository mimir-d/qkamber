#pragma once

#include <cstring>

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
// optional_t
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class optional_t
{
public:
    optional_t() = default;
    optional_t(const T& rhs);
    optional_t(T&& rhs);
    optional_t(const optional_t& rhs);
    ~optional_t();

    optional_t& operator=(const T& rhs);

    operator bool() const;
    bool has_value() const;

    T& value();
    const T& value() const;

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    bool m_has_value = false;
};

template <typename T>
optional_t<T>::optional_t(const T& rhs) :
    m_has_value(true)
{
    new (&m_storage) T{ rhs };
}

template <typename T>
optional_t<T>::optional_t(T&& rhs) :
    m_has_value(true)
{
    new (&m_storage) T{ std::move(rhs) };
}

template <typename T>
optional_t<T>::optional_t(const optional_t& rhs) :
    m_has_value(rhs.m_has_value)
{
    // NOTE: T may not be trivially copyable, so make a new one invoking copy ctor
    new (&m_storage) T{ *reinterpret_cast<const T*>(&rhs.m_storage) };
}

template <typename T>
optional_t<T>::~optional_t()
{
    if (m_has_value)
        reinterpret_cast<T*>(&m_storage)->~T();
}

template <typename T>
optional_t<T>& optional_t<T>::operator=(const T& rhs)
{
    new (&m_storage) T{ rhs };
    m_has_value = true;
    return *this;
}

template <typename T>
optional_t<T>::operator bool() const
{
    return m_has_value;
}

template <typename T>
bool optional_t<T>::has_value() const
{
    return m_has_value;
}

template <typename T>
T& optional_t<T>::value()
{
    if (!m_has_value)
        throw std::runtime_error("optional_t doesnt have value");
    return *reinterpret_cast<T*>(&m_storage);
}

template <typename T>
const T& optional_t<T>::value() const
{
    if (!m_has_value)
        throw std::runtime_error("optional_t doesnt have value");
    return *reinterpret_cast<const T*>(&m_storage);
}

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
// typelist_at
///////////////////////////////////////////////////////////////////////////////
template <size_t I, typename... Ts>
struct typelist_at;

template <typename T, typename... Rest>
struct typelist_at<0, T, Rest...>
{
    using type = T;
};

template <size_t I, typename T, typename... Rest>
struct typelist_at<I, T, Rest...> :
    typelist_at<I-1, Rest...>
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
                    throw std::runtime_error("print_fmt invalid format string: missing arguments");
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
        throw std::runtime_error("print_fmt invalid format string: extra arguments");
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

///////////////////////////////////////////////////////////////////////////////
// path utilities
///////////////////////////////////////////////////////////////////////////////
// TODO: move these to a path class, zip assets, etc
inline std::string dirname(const std::string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

inline std::string basename(const std::string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return path;
    return path.substr(pos + 1);
}
