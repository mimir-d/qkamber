#pragma once

#include "misc.h"

///////////////////////////////////////////////////////////////////////////////
// generic math
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename T>
    class RealGenerator
    {
        static_assert(std::is_floating_point<T>::value, "T must be floating point");
    public:
        RealGenerator();
        T operator()();

    private:
        std::default_random_engine m_gen;
        std::uniform_real_distribution<T> m_dist;
    };
};

constexpr float PI      = 3.141592653f;
constexpr float PI_2    = 1.570796326f;
constexpr float PI_4    = 0.785398163f;

inline float frand();

// variadic min/max
template <
    typename T, typename... Args,
    typename = std::enable_if_t<std::is_same<repeat_t<T, sizeof...(Args)>, std::tuple<Args...>>::value>
>
inline const T& min(const T& a, const Args&... args);

template <
    typename T, typename... Args,
    typename = std::enable_if_t<std::is_same<repeat_t<T, sizeof...(Args)>, std::tuple<Args...>>::value>
>
inline const T& max(const T& a, const Args&... args);

template <typename T>
inline T clamp(T value, T min_value, T max_value);

// TODO: rename or move to namespace?
struct no_init_tag {};

///////////////////////////////////////////////////////////////////////////////
// FixedPoint
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t Digits>
class FixedPoint
{
    static_assert(std::is_integral<T>::value, "FixedPoint can only accept integral types as backing");
public:
    //FixedPoint(const FixedPoint& rhs);
    ~FixedPoint() = default;

    // converting ctors
    FixedPoint(T value, size_t frac_digits = Digits);
    FixedPoint(float value);

    template <size_t RhsDigits, typename = std::enable_if_t<(RhsDigits <= Digits)>>
    FixedPoint(const FixedPoint<T, RhsDigits>& rhs);

    // NOTE: explicit for narrowing conversion
    // TODO: how to diff between first and this method in external definition
    // template <size_t RhsDigits, typename = std::enable_if_t<(RhsDigits > Digits)>>
    // explicit FixedPoint(const FixedPoint<T, RhsDigits>& rhs);

    explicit operator T() const;
    explicit operator float() const;

    FixedPoint& operator+=(const FixedPoint& rhs);
    FixedPoint& operator-=(const FixedPoint& rhs);
    FixedPoint& operator*=(const FixedPoint& rhs);

    FixedPoint operator+(const FixedPoint& rhs) const;
    FixedPoint operator-(const FixedPoint& rhs) const;
    FixedPoint operator*(const FixedPoint& rhs) const;

    template <size_t RhsDigits>
    FixedPoint<T, Digits + RhsDigits> denorm_mul(const FixedPoint<T, RhsDigits>& rhs) const;

    bool operator<(const FixedPoint& rhs) const;
    bool operator>(const FixedPoint& rhs) const;
    bool operator==(const FixedPoint& rhs) const;

private:
    template <typename, size_t>
    friend class FixedPoint;

    T m_value;
};

using fp4 = FixedPoint<int32_t, 4>;
using fp8 = FixedPoint<int32_t, 8>;

///////////////////////////////////////////////////////////////////////////////
// vector types
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
class vec
{
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
public:
    vec();
    vec(const vec& rhs);

    template <
        typename... Args,
        typename = std::enable_if_t<std::is_convertible<std::tuple<Args...>, repeat_t<T, N>>::value>
    >
    vec(Args&&... args);
    vec(no_init_tag) {}

    T length() const;
    T length_sq() const;
    vec normalize() const;

    T& operator[](size_t index);
    T operator[](size_t index) const;

    vec& operator=(const vec& rhs);
    vec& operator=(float rhs);

    vec& operator+=(const vec& rhs);
    vec& operator-=(const vec& rhs);
    vec& operator*=(T rhs);

    vec operator-() const;

    vec operator+(const vec& rhs) const;
    vec operator-(const vec& rhs) const;
    vec operator*(T rhs) const;

    T operator*(const vec& rhs) const;

private:
    template <int I>
    struct eq_op
    {
        void operator()(vec& out, const vec& rhs) const;
        void operator()(vec& out, float rhs) const;
    };
    template <int I>
    struct add_op
    {
        void operator()(vec& out, const vec& lhs, const vec& rhs) const;
    };
    template <int I>
    struct sub_op
    {
        void operator()(vec& out, const vec& lhs, const vec& rhs) const;
    };
    template <int I>
    struct mul_op
    {
        void operator()(vec& out, const vec& lhs, T rhs) const;
    };
    template <int I>
    struct neg_op
    {
        void operator()(vec& out, const vec& rhs) const;
    };
    template <int I>
    struct dot_op
    {
        void operator()(T& out, const vec& lhs, const vec& rhs) const;
    };

protected:
    std::array<T, N> m_data;
};

// TODO: make vec2 typed
class vec2 : public vec<float, 2>
{
public:
    using vec<float, 2>::vec;

    vec2() : vec<float, 2>() {}
    vec2(const vec2& rhs) : vec<float, 2>(rhs) {}
    vec2(const vec<float, 2>& rhs) : vec<float, 2>(rhs) {}

    // TODO: move these in vec?
    float& x() { return m_data[0]; }
    float& y() { return m_data[1]; }

    float x() const { return m_data[0]; }
    float y() const { return m_data[1]; }
};

class vec3 : public vec<float, 3>
{
public:
    using vec<float, 3>::vec;

    vec3() : vec<float, 3>() {}
    vec3(const vec3& rhs) : vec<float, 3>(rhs) {}
    vec3(const vec<float, 3>& rhs) : vec<float, 3>(rhs) {}

    using vec::operator=;

    float& x() { return m_data[0]; }
    float& y() { return m_data[1]; }
    float& z() { return m_data[2]; }

    float x() const { return m_data[0]; }
    float y() const { return m_data[1]; }
    float z() const { return m_data[2]; }

    // cross product
    vec3 operator^(const vec3& rhs) const;
};

class vec4 : public vec<float, 4>
{
public:
    using vec<float, 4>::vec;

    vec4() : vec<float, 4>() {}
    vec4(const vec4& rhs) : vec<float, 4>(rhs) {}
    vec4(const vec<float, 4>& rhs) : vec<float, 4>(rhs) {}

    float& x() { return m_data[0]; }
    float& y() { return m_data[1]; }
    float& z() { return m_data[2]; }
    float& w() { return m_data[3]; }

    float x() const { return m_data[0]; }
    float y() const { return m_data[1]; }
    float z() const { return m_data[2]; }
    float w() const { return m_data[3]; }
};

class Color : public vec<float, 4>
{
    using Base = vec<float, 4>;
public:
    using Base::Base;

    Color() : Base() {}
    Color(const Color& rhs) : Base(rhs) {}
    Color(const Base& rhs) : Base(rhs) {}

    float& r() { return m_data[0]; }
    float& g() { return m_data[1]; }
    float& b() { return m_data[2]; }
    float& a() { return m_data[3]; }

    float r() const { return m_data[0]; }
    float g() const { return m_data[1]; }
    float b() const { return m_data[2]; }
    float a() const { return m_data[3]; }
};

///////////////////////////////////////////////////////////////////////////////
// matrix types
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1 = D0>
class mat
{
public:
    template <bool is_const>
    class row_t
    {
    public:
        typedef T value_t;
        typedef typename std::conditional<is_const, const T&, T&>::type ref_t;
        typedef typename std::conditional<is_const, const T*, T*>::type ptr_t;

    public:
        row_t(ptr_t start);
        row_t(const row_t<false>& rhs);

        ref_t operator[](int index);
        value_t operator[](int index) const;

        row_t& operator=(const vec<T, D1>& rhs);
        operator vec<T, D1>() const;

    private:
        template <int I>
        struct set_vec
        {
            void operator()(row_t& out, const vec<T, D1>& rhs);
        };
        template <int I>
        struct get_vec
        {
            void operator()(vec<T, D1>& out, const row_t& rhs);
        };

    private:
        // needed because of the non-const to const copy ctor
        friend row_t<is_const>;
        ptr_t m_start;
    };

    typedef row_t<false> row;
    typedef const row_t<true> const_row;

public:
    mat();
    mat(const mat& rhs);

    template <
        typename... Args,
        typename = std::enable_if_t<std::is_convertible<std::tuple<Args...>, repeat_t<T, D0 * D1>>::value>
    >
    mat(Args&&... args);
    mat(no_init_tag) {}

    row operator[](int index)
    {
        return row(m_data.data() + D1 * index);
    }
    // RANT: vs2015 signals this as redefinition if defined outside the class, even if
    // the method is marked as const as opposed to the higher one
    const_row operator[](int index) const
    {
        return const_row(m_data.data() + D1 * index);
    }

    mat& operator=(const mat& rhs);

    template <typename = std::enable_if_t<D0 == D1>>
    mat& operator*=(const mat& rhs);
    mat& operator*=(T rhs);

    template <int D2>
    mat<T, D0, D2> operator*(const mat<T, D1, D2>& rhs) const;
    vec<T, D0> operator*(const vec<T, D1>& rhs) const;
    mat operator*(T rhs) const;

private:
    template <int I>
    struct eq_op
    {
        void operator()(mat& out, const mat& rhs) const;
    };

    template <int I>
    struct scale_op
    {
        void operator()(mat& out, const mat& lhs, T rhs) const;
    };

    // NOTE: this needs to have increasing K such that the compiler sees the instructions sequentially forward
    // RANT: vs2015 can't deal with implicit D1 so a value copy needs to happen here
    template <int _D1 = D1, int K = 0>
    struct mul_op
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const;
        };
    };

    template <int _D1>
    struct mul_op<_D1, 0>
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const;
        };
    };

    template <int _D1>
    struct mul_op<_D1, _D1>
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat&, const mat<T, _D1, D2>&) const;
        };
    };

    template <int I, int J>
    struct transform_op
    {
        void operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const;
    };

    template <int I>
    struct transform_op<I, 0>
    {
        void operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const;
    };

protected:
    std::array<T, D0 * D1> m_data;
};

using mat3 = mat<float, 3>;
using mat3x4 = mat<float, 3, 4>;

class mat4 : public mat<float, 4>
{
public:
    using mat<float, 4>::mat;

    mat4() : mat<float, 4>() {}
    mat4(const mat4& rhs) : mat<float, 4>(rhs) {}
    mat4(const mat<float, 4>& rhs) : mat<float, 4>(rhs) {}

    // TODO: future normal matrix stuff
    /*
    float det() const
    {
        const auto& m = m_data.data();
        return
            m[12] * m[9] * m[6] * m[3] -
            m[8] * m[13] * m[6] * m[3] -
            m[12] * m[5] * m[10] * m[3] +
            m[4] * m[13] * m[10] * m[3] +
            m[8] * m[5] * m[14] * m[3] -
            m[4] * m[9] * m[14] * m[3] -
            m[12] * m[9] * m[2] * m[7] +
            m[8] * m[13] * m[2] * m[7] +
            m[12] * m[1] * m[10] * m[7] -
            m[0] * m[13] * m[10] * m[7] -
            m[8] * m[1] * m[14] * m[7] +
            m[0] * m[9] * m[14] * m[7] +
            m[12] * m[5] * m[2] * m[11] -
            m[4] * m[13] * m[2] * m[11] -
            m[12] * m[1] * m[6] * m[11] +
            m[0] * m[13] * m[6] * m[11] +
            m[4] * m[1] * m[14] * m[11] -
            m[0] * m[5] * m[14] * m[11] -
            m[8] * m[5] * m[2] * m[15] +
            m[4] * m[9] * m[2] * m[15] +
            m[8] * m[1] * m[6] * m[15] -
            m[0] * m[9] * m[6] * m[15] -
            m[4] * m[1] * m[10] * m[15] +
            m[0] * m[5] * m[10] * m[15];
    }

    mat4 transpose() const
    {
        return mat4 {
            m_data[0], m_data[4], m_data[8], m_data[12],
            m_data[1], m_data[5], m_data[9], m_data[13],
            m_data[2], m_data[6], m_data[10], m_data[14],
            m_data[3], m_data[7], m_data[11], m_data[15]
        };
    }

    mat4 invert() const
    {
        float x = det();
        if (fabs(x) <= 1e-20)
            return mat4 {};

        mat4 ret;
        float *i = ret.m_data.data();
        const float *m = m_data.data();

        i[0] = (- m[13] * m[10] * m[7] + m[9] * m[14] * m[7] + m[13] * m[6] * m[11] - m[5] * m[14] * m[11] - m[9] * m[6] * m[15] + m[5] * m[10] * m[15]) / x;
        i[4] = (+ m[12] * m[10] * m[7] - m[8] * m[14] * m[7] - m[12] * m[6] * m[11] + m[4] * m[14] * m[11] + m[8] * m[6] * m[15] - m[4] * m[10] * m[15]) / x;
        i[8] = (- m[12] * m[9] * m[7] + m[8] * m[13] * m[7] + m[12] * m[5] * m[11] - m[4] * m[13] * m[11] - m[8] * m[5] * m[15] + m[4] * m[9] * m[15]) / x;
        i[12] = (+ m[12] * m[9] * m[6] - m[8] * m[13] * m[6] - m[12] * m[5] * m[10] + m[4] * m[13] * m[10] + m[8] * m[5] * m[14] - m[4] * m[9] * m[14]) / x;
        i[1] = (+ m[13] * m[10] * m[3] - m[9] * m[14] * m[3] - m[13] * m[2] * m[11] + m[1] * m[14] * m[11] + m[9] * m[2] * m[15] - m[1] * m[10] * m[15]) / x;
        i[5] = (- m[12] * m[10] * m[3] + m[8] * m[14] * m[3] + m[12] * m[2] * m[11] - m[0] * m[14] * m[11] - m[8] * m[2] * m[15] + m[0] * m[10] * m[15]) / x;
        i[9] = (+ m[12] * m[9] * m[3] - m[8] * m[13] * m[3] - m[12] * m[1] * m[11] + m[0] * m[13] * m[11] + m[8] * m[1] * m[15] - m[0] * m[9] * m[15]) / x;
        i[13] = (- m[12] * m[9] * m[2] + m[8] * m[13] * m[2] + m[12] * m[1] * m[10] - m[0] * m[13] * m[10] - m[8] * m[1] * m[14] + m[0] * m[9] * m[14]) / x;
        i[2] = (- m[13] * m[6] * m[3] + m[5] * m[14] * m[3] + m[13] * m[2] * m[7] - m[1] * m[14] * m[7] - m[5] * m[2] * m[15] + m[1] * m[6] * m[15]) / x;
        i[6] = (+ m[12] * m[6] * m[3] - m[4] * m[14] * m[3] - m[12] * m[2] * m[7] + m[0] * m[14] * m[7] + m[4] * m[2] * m[15] - m[0] * m[6] * m[15]) / x;
        i[10] = (- m[12] * m[5] * m[3] + m[4] * m[13] * m[3] + m[12] * m[1] * m[7] - m[0] * m[13] * m[7] - m[4] * m[1] * m[15] + m[0] * m[5] * m[15]) / x;
        i[14] = (+ m[12] * m[5] * m[2] - m[4] * m[13] * m[2] - m[12] * m[1] * m[6] + m[0] * m[13] * m[6] + m[4] * m[1] * m[14] - m[0] * m[5] * m[14]) / x;
        i[3] = (+ m[9] * m[6] * m[3] - m[5] * m[10] * m[3] - m[9] * m[2] * m[7] + m[1] * m[10] * m[7] + m[5] * m[2] * m[11] - m[1] * m[6] * m[11]) / x;
        i[7] = (- m[8] * m[6] * m[3] + m[4] * m[10] * m[3] + m[8] * m[2] * m[7] - m[0] * m[10] * m[7] - m[4] * m[2] * m[11] + m[0] * m[6] * m[11]) / x;
        i[11] = (+ m[8] * m[5] * m[3] - m[4] * m[9] * m[3] - m[8] * m[1] * m[7] + m[0] * m[9] * m[7] + m[4] * m[1] * m[11] - m[0] * m[5] * m[11]) / x;
        i[15] = (- m[8] * m[5] * m[2] + m[4] * m[9] * m[2] + m[8] * m[1] * m[6] - m[0] * m[9] * m[6] - m[4] * m[1] * m[10] + m[0] * m[5] * m[10]) / x;

        return ret;
    }
    */

public:
    static mat4 identity();
    static mat4 translate(float x, float y, float z);
    static mat4 rotate(float x, float y, float z);
    static mat4 scale(float x, float y, float z);

    static mat4 lookat(const vec3& eye, const vec3& at, const vec3& up);
    static mat4 proj_perspective(float fov, float aspect, float near_plane, float far_plane);
    // static mat4 proj_ortho();
    // TODO: maybe move this?
    static mat3x4 clip(float x, float y, float width, float height, float near_limit, float far_limit);
};

///////////////////////////////////////////////////////////////////////////////
// generic math impl
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline detail::RealGenerator<T>::RealGenerator() :
    m_gen(static_cast<unsigned>(time(nullptr)))
{}

template <typename T>
inline T detail::RealGenerator<T>::operator()()
{
    return m_dist(m_gen);
}

inline float frand()
{
    static detail::RealGenerator<float> gen;
    return gen();
}

namespace detail
{
    template <typename T>
    inline const T& min_impl(const T& a)
    {
        return a;
    }

    template <typename T, typename... Args>
    inline const T& min_impl(const T& a, const T& b, const Args&... args)
    {
        return min_impl(a < b ? a : b, args...);
    }

    template <typename T>
    inline const T& max_impl(const T& a)
    {
        return a;
    }

    template <typename T, typename... Args>
    inline const T& max_impl(const T& a, const T& b, const Args&... args)
    {
        return max_impl(a > b ? a : b, args...);
    }
}

// variadic min/max with min 1 arg
template <typename T, typename... Args, typename>
inline const T& min(const T& a, const Args&... args)
{
    return detail::min_impl(a, args...);
}

template <typename T, typename... Args, typename>
inline const T& max(const T& a, const Args&... args)
{
    return detail::max_impl(a, args...);
}

template <typename T>
inline T clamp(T value, T min_value, T max_value)
{
    if (value > max_value)
        return max_value;
    if (value < min_value)
        return min_value;
    return value;
}

///////////////////////////////////////////////////////////////////////////////
// FixedPoint
///////////////////////////////////////////////////////////////////////////////
//template <typename T, size_t Digits>
//inline FixedPoint<T, Digits>::FixedPoint(const FixedPoint& rhs)
//{
//    m_value = rhs.m_value;
//}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>::FixedPoint(T value, size_t frac_digits) :
    m_value(value << frac_digits)
{}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>::FixedPoint(float value)
{
    m_value = static_cast<T>(value * static_cast<float>(1 << Digits));
}

template <typename T, size_t Digits>
template <size_t RhsDigits, typename>
inline FixedPoint<T, Digits>::FixedPoint(const FixedPoint<T, RhsDigits>& rhs)
{
    m_value = rhs.m_value << (Digits - RhsDigits);
}

// template <typename T, size_t Digits>
// template <size_t RhsDigits, typename>
// inline FixedPoint<T, Digits>::FixedPoint(const FixedPoint<T, RhsDigits>& rhs)
// {
//     m_value = static_cast<T>(rhs) << (RhsDigits - Digits);
// }

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>::operator T() const
{
    const T rounding = (1 << Digits) - 1;
    return (m_value + rounding) >> Digits;
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>::operator float() const
{
    const float f = 1.0f / (1 << Digits);
    return m_value * f;
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>& FixedPoint<T, Digits>::operator+=(const FixedPoint& rhs)
{
    m_value += rhs.m_value;
    return *this;
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>& FixedPoint<T, Digits>::operator-=(const FixedPoint& rhs)
{
    m_value -= rhs.m_value;
    return *this;
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits>& FixedPoint<T, Digits>::operator*=(const FixedPoint& rhs)
{
    m_value = (m_value * rhs.m_value) >> Digits;
    return *this;
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits> FixedPoint<T, Digits>::operator+(const FixedPoint& rhs) const
{
    return { m_value + rhs.m_value, 0 };
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits> FixedPoint<T, Digits>::operator-(const FixedPoint& rhs) const
{
    return { m_value - rhs.m_value, 0 };
}

template <typename T, size_t Digits>
inline FixedPoint<T, Digits> FixedPoint<T, Digits>::operator*(const FixedPoint& rhs) const
{
    return { (m_value * rhs.m_value) >> Digits, 0 };
}

template <typename T, size_t Digits>
template <size_t RhsDigits>
inline FixedPoint<T, Digits + RhsDigits>
FixedPoint<T, Digits>::denorm_mul(const FixedPoint<T, RhsDigits>& rhs) const
{
    return { m_value * rhs.m_value, 0 };
}

template <typename T, size_t Digits>
inline bool FixedPoint<T, Digits>::operator<(const FixedPoint& rhs) const
{
    return m_value < rhs.m_value;
}

template <typename T, size_t Digits>
inline bool FixedPoint<T, Digits>::operator>(const FixedPoint& rhs) const
{
    return m_value > rhs.m_value;
}

template <typename T, size_t Digits>
inline bool FixedPoint<T, Digits>::operator==(const FixedPoint& rhs) const
{
    return m_value == rhs.m_value;
}

///////////////////////////////////////////////////////////////////////////////
// vec impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <int D0, template<int> class Func, int I = 0>
    struct iterate1
    {
        template <typename Result, typename... Args>
        Result& operator()(Result& out, Args&&... args) const
        {
            Func<I>()(out, std::forward<Args>(args)...);
            return iterate1<D0, Func, I+1>()(out, std::forward<Args>(args)...);
        }
    };

    template <int D0, template<int> class Func>
    struct iterate1<D0, Func, D0>
    {
        template <typename Result, typename... Args>
        Result& operator()(Result& out, Args&&...) const
        {
            return out;
        }
    };
}

template <typename T, size_t N>
inline vec<T, N>::vec()
{
    m_data.fill(0);
}

template <typename T, size_t N>
inline vec<T, N>::vec(const vec& rhs) :
    m_data(rhs.m_data)
{}

template <typename T, size_t N>
template <typename... Args, typename>
inline vec<T, N>::vec(Args&&... args) :
    m_data { static_cast<T>(args)... }
{}

template <typename T, size_t N>
inline T vec<T, N>::length() const
{
    return static_cast<T>(sqrt(length_sq()));
}

template <typename T, size_t N>
inline T vec<T, N>::length_sq() const
{
    T len2 = 0;
    detail::iterate1<N, dot_op>()(len2, *this, *this);
    return len2;
}

template <typename T, size_t N>
inline vec<T, N> vec<T, N>::normalize() const
{
    vec ret;

    T len = length();
    if (len > std::numeric_limits<float>::min())
        return detail::iterate1<N, mul_op>()(ret, *this, 1.0f / len);

    return ret;
}

template <typename T, size_t N>
inline T& vec<T, N>::operator[](size_t index)
{
    return m_data[index];
}

template <typename T, size_t N>
inline T vec<T, N>::operator[](size_t index) const
{
    return m_data[index];
}

template <typename T, size_t N>
inline vec<T, N>& vec<T, N>::operator=(const vec& rhs)
{
    return detail::iterate1<N, eq_op>()(*this, rhs);
}

template <typename T, size_t N>
inline vec<T, N>& vec<T, N>::operator=(float rhs)
{
    return detail::iterate1<N, eq_op>()(*this, rhs);
}

template <typename T, size_t N>
inline vec<T, N>& vec<T, N>::operator+=(const vec& rhs)
{
    return detail::iterate1<N, add_op>()(*this, *this, rhs);
}

template <typename T, size_t N>
inline vec<T, N>& vec<T, N>::operator-=(const vec& rhs)
{
    return detail::iterate1<N, sub_op>()(*this, *this, rhs);
}

template <typename T, size_t N>
inline vec<T, N>& vec<T, N>::operator*=(T rhs)
{
    return detail::iterate1<N, mul_op>()(*this, *this, rhs);
}

template <typename T, size_t N>
inline vec<T, N> vec<T, N>::operator-() const
{
    vec ret(no_init_tag {});
    return detail::iterate1<N, neg_op>()(ret, *this);
}

template <typename T, size_t N>
inline vec<T, N> vec<T, N>::operator+(const vec& rhs) const
{
    vec ret(no_init_tag {});
    return detail::iterate1<N, add_op>()(ret, *this, rhs);
}

template <typename T, size_t N>
inline vec<T, N> vec<T, N>::operator-(const vec& rhs) const
{
    vec ret(no_init_tag {});
    return detail::iterate1<N, sub_op>()(ret, *this, rhs);
}

template <typename T, size_t N>
inline vec<T, N> vec<T, N>::operator*(T rhs) const
{
    vec ret(no_init_tag {});
    return detail::iterate1<N, mul_op>()(ret, *this, rhs);
}

template <typename T, size_t N>
inline T vec<T, N>::operator*(const vec& rhs) const
{
    T ret = 0;
    return detail::iterate1<N, dot_op>()(ret, *this, rhs);
}

///////////////////////////////////////////////////////////////////////////////
// vec<T, N> operations impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
template <int I>
inline void vec<T, N>::eq_op<I>::operator()(vec& out, const vec& rhs) const
{
    out.m_data[I] = rhs.m_data[I];
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::eq_op<I>::operator()(vec& out, float rhs) const
{
    out.m_data[I] = rhs;
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::add_op<I>::operator()(vec& out, const vec& lhs, const vec& rhs) const
{
    out.m_data[I] = lhs.m_data[I] + rhs.m_data[I];
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::sub_op<I>::operator()(vec& out, const vec& lhs, const vec& rhs) const
{
    out.m_data[I] = lhs.m_data[I] - rhs.m_data[I];
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::mul_op<I>::operator()(vec& out, const vec& lhs, T rhs) const
{
    out.m_data[I] = lhs.m_data[I] * rhs;
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::neg_op<I>::operator()(vec& out, const vec& rhs) const
{
    out.m_data[I] = -rhs.m_data[I];
}

template <typename T, size_t N>
template <int I>
inline void vec<T, N>::dot_op<I>::operator()(T& out, const vec& lhs, const vec& rhs) const
{
    out += lhs.m_data[I] * rhs.m_data[I];
}

///////////////////////////////////////////////////////////////////////////////
// vec3 impl
///////////////////////////////////////////////////////////////////////////////
static_assert(sizeof(vec3) == 3 * sizeof(float), "vec3 is not a value type");

inline vec3 vec3::operator^(const vec3& rhs) const
{
    return vec3 {
        m_data[1] * rhs.m_data[2] - m_data[2] * rhs.m_data[1],
        m_data[2] * rhs.m_data[0] - m_data[0] * rhs.m_data[2],
        m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0]
    };
}

///////////////////////////////////////////////////////////////////////////////
// vec4 impl
///////////////////////////////////////////////////////////////////////////////
static_assert(sizeof(vec4) == 4 * sizeof(float), "vec4 is not a value type");

///////////////////////////////////////////////////////////////////////////////
// mat<T, D0, D1>::row_t<is_const> impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline mat<T, D0, D1>::row_t<is_const>::row_t(ptr_t start) :
    m_start(start)
{}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline mat<T, D0, D1>::row_t<is_const>::row_t(const row_t<false>& rhs) :
    m_start(rhs.m_start)
{}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline typename mat<T, D0, D1>::template row_t<is_const>::ref_t
mat<T, D0, D1>::row_t<is_const>::operator[](int index)
{
    return m_start[index];
}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline typename mat<T, D0, D1>::template row_t<is_const>::value_t
mat<T, D0, D1>::row_t<is_const>::operator[](int index) const
{
    return m_start[index];
}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline typename mat<T, D0, D1>::row_t<is_const>& mat<T, D0, D1>::row_t<is_const>::operator=(const vec<T, D1>& rhs)
{
    return detail::iterate1<D1, set_vec>()(*this, rhs);
}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
inline typename mat<T, D0, D1>::row_t<is_const>::operator vec<T, D1>() const
{
    vec<T, D1> ret;
    detail::iterate1<D1, get_vec>()(ret, *this);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// mat<T, D0, D1>::row_t<is_const>::set/get_vec impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <bool is_const>
template <int I>
inline void mat<T, D0, D1>::row_t<is_const>::set_vec<I>::operator()(row_t& out, const vec<T, D1>& rhs)
{
    out.m_start[I] = rhs[I];
}

template <typename T, size_t D0, size_t D1>
template <bool is_const>
template <int I>
inline void mat<T, D0, D1>::row_t<is_const>::get_vec<I>::operator()(vec<T, D1>& out, const row_t& rhs)
{
    out[I] = rhs.m_start[I];
}

///////////////////////////////////////////////////////////////////////////////
// mat<T, D0, D1> impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <int D0, int D1, template<int, int> class Op, int I = 0, int J = 0>
    struct iterate2
    {
        template <typename Result, typename... Args>
        Result& operator()(Result& out, Args&&... args) const
        {
            Op<I,J>()(out, std::forward<Args>(args)...);
            return iterate2<D0, D1, Op, I, J+1>()(out, std::forward<Args>(args)...);
        }
    };

    template <int D0, int D1, template<int, int> class Op, int I>
    struct iterate2<D0, D1, Op, I, D1>
    {
        template <typename Result, typename... Args>
        Result& operator()(Result& out, Args&&... args) const
        {
            return iterate2<D0, D1, Op, I+1>()(out, std::forward<Args>(args)...);
        }
    };

    template <int D0, int D1, template<int, int> class Op>
    struct iterate2<D0, D1, Op, D0, 0>
    {
        template <typename Result, typename... Args>
        Result& operator()(Result& out, Args&&...) const
        {
            return out;
        }
    };
}

static_assert(sizeof(mat4) == 16 * sizeof(float), "mat4 is not a value type");

template <typename T, size_t D0, size_t D1>
inline mat<T, D0, D1>::mat()
{
    m_data.fill(0);
}

template <typename T, size_t D0, size_t D1>
inline mat<T, D0, D1>::mat(const mat& rhs) :
    m_data(rhs.m_data)
{}

template <typename T, size_t D0, size_t D1>
template <typename... Args, typename>
inline mat<T, D0, D1>::mat(Args&&... args) :
    m_data { static_cast<T>(args)... }
{}

template <typename T, size_t D0, size_t D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator=(const mat& rhs)
{
    return detail::iterate1<D0 * D1, eq_op>()(*this, rhs);
}

template <typename T, size_t D0, size_t D1>
template <typename>
mat<T, D0, D1>& mat<T, D0, D1>::operator*=(const mat& rhs)
{
    // NOTE: keep result in a different var because cells get overwritten
    // while doing the multiplication algo
    mat result(no_init_tag {});

    detail::iterate2<D0, D1, mul_op<>::mul_row>()(result, *this, rhs);
    return detail::iterate1<D0 * D1, eq_op>()(*this, result);
}

template <typename T, size_t D0, size_t D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator*=(T rhs)
{
    return detail::iterate1<D0 * D1, scale_op>()(*this, *this, rhs);
}

template <typename T, size_t D0, size_t D1>
template <int D2>
inline mat<T, D0, D2> mat<T, D0, D1>::operator*(const mat<T, D1, D2>& rhs) const
{
    mat<T, D0, D2> ret(no_init_tag {});
    return detail::iterate2<D0, D2, mul_op<>::mul_row>()(ret, *this, rhs);
}

template <typename T, size_t D0, size_t D1>
inline vec<T, D0> mat<T, D0, D1>::operator*(const vec<T, D1>& rhs) const
{
    vec<T, D0> ret(no_init_tag {});
    return detail::iterate2<D0, D1, transform_op>()(ret, *this, rhs);
}

template <typename T, size_t D0, size_t D1>
inline mat<T, D0, D1> mat<T, D0, D1>::operator*(T rhs) const
{
    mat ret(no_init_tag {});
    return detail::iterate1<D0 * D1, scale_op>()(ret, *this, rhs);
}

///////////////////////////////////////////////////////////////////////////////
// mat4::eq_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <int I>
inline void mat<T, D0, D1>::eq_op<I>::operator()(mat& out, const mat& rhs) const
{
    out.m_data[I] = rhs.m_data[I];
}

///////////////////////////////////////////////////////////////////////////////
// mat4::scale_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <int I>
inline void mat<T, D0, D1>::scale_op<I>::operator()(mat& out, const mat& lhs, T rhs) const
{
    out.m_data[I] = lhs.m_data[I] * rhs;
}

///////////////////////////////////////////////////////////////////////////////
// mat4::mul_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <int _D1, int K>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, K>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const
{
    out.m_data[I*D1 + J] += lhs.m_data[I*D1 + K] * rhs.m_data[K*D1 + J];
    typename mul_op<_D1, K+1>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, size_t D0, size_t D1>
template <int _D1>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, 0>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const
{
    // start with 0 case because we need to initialize the out matrix
    out.m_data[I*D1 + J] = lhs.m_data[I*D1] * rhs.m_data[J];
    typename mul_op<_D1, 1>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, size_t D0, size_t D1>
template <int _D1>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, _D1>::mul_row<I, J>::operator()(mat<T, D0, D2>&, const mat&, const mat<T, _D1, D2>&) const
{}

///////////////////////////////////////////////////////////////////////////////
// mat4::transform_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, size_t D0, size_t D1>
template <int I, int J>
inline void mat<T, D0, D1>::transform_op<I, J>::operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const
{
    out[I] += lhs.m_data[I*D1 + J] * rhs[J];
}

template <typename T, size_t D0, size_t D1>
template <int I>
inline void mat<T, D0, D1>::transform_op<I, 0>::operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const
{
    out[I] = lhs.m_data[I*D1] * rhs[0];
}

///////////////////////////////////////////////////////////////////////////////
// matrices impl -- right hand math
///////////////////////////////////////////////////////////////////////////////
inline mat4 mat4::identity()
{
    return mat4 {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

inline mat4 mat4::translate(float x, float y, float z)
{
    return mat4 {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    };
}

inline mat4 mat4::rotate(float x, float y, float z)
{
    // yaw pitch roll = y x z (directx)
    const float ca = cos(x), sa = sin(x);
    const float cb = cos(y), sb = sin(y);
    const float cc = cos(z), sc = sin(z);

    return mat4 {
        cc*cb, -sc*ca + cc*sb*sa,  sc*sa + cc*sb*ca, 0,
        sc*cb,  cc*ca + sc*sb*sa, -cc*sa + sc*sb*ca, 0,
        -sb, cb*sa, cb*ca, 0,
        0, 0, 0, 1
    };
}

inline mat4 mat4::scale(float x, float y, float z)
{
    return mat4 {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };
}

inline mat4 mat4::lookat(const vec3& eye, const vec3& at, const vec3& up)
{
    vec3 cz = (eye - at).normalize();
    vec3 cx = (up ^ cz).normalize();
    vec3 cy = cz ^ cx;

    return mat4 {
        cx.x(), cx.y(), cx.z(), -cx * eye,
        cy.x(), cy.y(), cy.z(), -cy * eye,
        cz.x(), cz.y(), cz.z(), -cz * eye,
        0, 0, 0, 1
    };
}

inline mat4 mat4::proj_perspective(float fov, float aspect, float n, float f)
{
    const float h = 1.0f / tan(fov / 2);
    const float r = f / (n - f);

    return mat4 {
        h / aspect, 0, 0, 0,
        0, h, 0, 0,
        0, 0, r, r * n,
        0, 0, -1, 0
    };
}

inline mat3x4 mat4::clip(float x, float y, float w, float h, float n, float f)
{
    return mat3x4 {
        w/2, 0, 0, x + w / 2,
        0, h/2, 0, y + h / 2,
        0, 0, (f - n) / 2, (f + n) / 2
    };
}
