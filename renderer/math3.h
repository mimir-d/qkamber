#pragma once

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

inline float frand()
{
    static detail::RealGenerator<float> gen;
    return gen();
}

///////////////////////////////////////////////////////////////////////////////
// vector types
///////////////////////////////////////////////////////////////////////////////
template <typename T, int N>
class vec
{
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
public:
    vec();
    vec(const vec& rhs);

    T len() const;
    vec normalize() const;

    T& operator[](size_t index);
    T operator[](size_t index) const;

    vec& operator=(const vec& rhs);

    vec& operator+=(const vec& rhs);
    vec& operator-=(const vec& rhs);
    vec& operator*=(T rhs);

    vec operator-() const;

    vec operator+(const vec& rhs) const;
    vec operator-(const vec& rhs) const;
    vec operator*(T rhs) const;

    T operator*(const vec& rhs) const;

protected:
    struct no_init_tag {};
    vec(no_init_tag) {}

private:
    template <int I>
    struct eq_op
    {
        void operator()(vec& out, const vec& rhs) const;
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

class vec3 : public vec<float, 3>
{
public:
    vec3() : vec<float, 3>() {}
    vec3(const vec3& rhs) : vec<float, 3>(rhs) {}
    vec3(const vec<float, 3>& rhs) : vec<float, 3>(rhs) {}
    vec3(float x, float y, float z);

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
    vec4() : vec<float, 4>() {}
    vec4(const vec4& rhs) : vec<float, 4>(rhs) {}
    vec4(const vec<float, 4>& rhs) : vec<float, 4>(rhs) {}
    vec4(float x, float y, float z, float w);

    float& x() { return m_data[0]; }
    float& y() { return m_data[1]; }
    float& z() { return m_data[2]; }
    float& w() { return m_data[3]; }

    float x() const { return m_data[0]; }
    float y() const { return m_data[1]; }
    float z() const { return m_data[2]; }
    float w() const { return m_data[3]; }
};

///////////////////////////////////////////////////////////////////////////////
// matrix types
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1 = D0>
class mat
{
public:
    //TODO: add bounds checking
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
    // TODO: refactor this with initializer_list
    // mat(const T data[D0 * D1]);
    mat(const mat& rhs);

    row operator[](int index)
    {
        return row(m_data.data() + D0 * index);
    }
    // RANT: vs2012 signals this as redefinition if defined outside the class, even if
    // the method is marked as const as opposed to the higher one
    const_row operator[](int index) const
    {
        return const_row(m_data.data() + D0 * index);
    }

    mat& operator=(const mat& rhs);

    // RANT: vs2012 cannot have default value for function templates, see:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#226
    template <int _D1>
    typename std::enable_if<D0 == _D1, mat&>::type operator*=(const mat<T, D0, _D1>& rhs);
    mat& operator*=(T rhs);

    template <int D2>
    mat<T, D0, D2> operator*(const mat<T, D1, D2>& rhs);
    vec<T, D0> operator*(const vec<T, D1>& rhs);
    mat operator*(T rhs);

protected:
    // optimization for binary operations that overwrite internal data:
    // since there is no need to zero initialize the matrix, just skip that
    struct no_init_tag {};
    mat(no_init_tag) {}

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
    // RANT: vs2012 can't deal with implicit D1 so a value copy needs to happen here
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

protected:
    std::array<T, D0 * D1> m_data;
};

class mat4 : public mat<float, 4>
{
public:
    mat4() : mat<float, 4>() {}
    // TODO: initializer_list
    // mat4(const float m[16]) : mat<float, 4>(m) {}
    mat4(const mat<float, 4>& rhs) : mat<float, 4>(rhs) {}
    mat4(const mat4& rhs) : mat<float, 4>(rhs) {}

private:
    mat4(no_init_tag) {}

public:
    static mat4 identity();
    static mat4 translate(float x, float y, float z);
    static mat4 rotate(float x, float y, float z);
    static mat4 scale(float x, float y, float z);

    static mat4 lookat(const vec3& eye, const vec3& at, const vec3& up);
    static mat4 proj_perspective(float fov, float aspect, float near_plane, float far_plane);
    // static mat4 proj_ortho();
    static mat4 clip(float x, float y, float width, float height, float near_limit, float far_limit);
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

///////////////////////////////////////////////////////////////////////////////
// vec impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <int D0, template<int> class Func, int I = 0>
    struct iterate1
    {
        // TODO: variadic template
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs& lhs, const Rhs& rhs) const
        {
            Func<I>()(out, lhs, rhs);
            return iterate1<D0, Func, I+1>()(out, lhs, rhs);
        }

        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const Rhs& rhs) const
        {
            Func<I>()(out, rhs);
            return iterate1<D0, Func, I+1>()(out, rhs);
        }
    };

    template <int D0, template<int> class Func>
    struct iterate1<D0, Func, D0>
    {
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs&, const Rhs&) const
        {
            return out;
        }

        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const Rhs&) const
        {
            return out;
        }
    };
}

template <typename T, int N>
inline vec<T, N>::vec()
{
    m_data.fill(0);
}

template <typename T, int N>
inline vec<T, N>::vec(const vec& rhs) :
    m_data(rhs.m_data)
{}

template <typename T, int N>
inline T vec<T, N>::len() const
{
    T len2 = 0;
    detail::iterate1<N, dot_op>()(len2, *this, *this);
    return static_cast<T>(sqrt(len2));
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::normalize() const
{
    vec ret;
    const float factor = 1.0f / len();
    return detail::iterate1<N, mul_op>()(ret, *this, factor);
}

template <typename T, int N>
inline T& vec<T, N>::operator[](size_t index)
{
    return m_data[index];
}

template <typename T, int N>
inline T vec<T, N>::operator[](size_t index) const
{
    return m_data[index];
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator=(const vec& rhs)
{
    return detail::iterate1<N, eq_op>()(*this, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator+=(const vec& rhs)
{
    return detail::iterate1<N, add_op>()(*this, *this, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator-=(const vec& rhs)
{
    return detail::iterate1<N, sub_op>()(*this, *this, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator*=(T rhs)
{
    return detail::iterate1<N, mul_op>()(*this, *this, rhs);
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::operator-() const
{
    no_init_tag no_init;
    vec ret(no_init);
    return detail::iterate1<N, neg_op>()(ret, *this);
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::operator+(const vec& rhs) const
{
    no_init_tag no_init;
    vec ret(no_init);
    return detail::iterate1<N, add_op>()(ret, *this, rhs);
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::operator-(const vec& rhs) const
{
    no_init_tag no_init;
    vec ret(no_init);
    return detail::iterate1<N, sub_op>()(ret, *this, rhs);
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::operator*(T rhs) const
{
    no_init_tag no_init;
    vec ret(no_init);
    return detail::iterate1<N, mul_op>()(ret, *this, rhs);
}

template <typename T, int N>
inline T vec<T, N>::operator*(const vec& rhs) const
{
    T ret = 0;
    return detail::iterate1<N, dot_op>()(ret, *this, rhs);
}

///////////////////////////////////////////////////////////////////////////////
// vec<T, N> operations impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int N>
template <int I>
inline void vec<T, N>::eq_op<I>::operator()(vec& out, const vec& rhs) const
{
    out.m_data[I] = rhs.m_data[I];
}

template <typename T, int N>
template <int I>
inline void vec<T, N>::add_op<I>::operator()(vec& out, const vec& lhs, const vec& rhs) const
{
    out.m_data[I] = lhs.m_data[I] + rhs.m_data[I];
}

template <typename T, int N>
template <int I>
inline void vec<T, N>::sub_op<I>::operator()(vec& out, const vec& lhs, const vec& rhs) const
{
    out.m_data[I] = lhs.m_data[I] - rhs.m_data[I];
}

template <typename T, int N>
template <int I>
inline void vec<T, N>::mul_op<I>::operator()(vec& out, const vec& lhs, T rhs) const
{
    out.m_data[I] = lhs.m_data[I] * rhs;
}

template <typename T, int N>
template <int I>
inline void vec<T, N>::neg_op<I>::operator()(vec& out, const vec& rhs) const
{
    out.m_data[I] = -rhs.m_data[I];
}

template <typename T, int N>
template <int I>
inline void vec<T, N>::dot_op<I>::operator()(T& out, const vec& lhs, const vec& rhs) const
{
    out += lhs.m_data[I] * rhs.m_data[I];
}

///////////////////////////////////////////////////////////////////////////////
// vec3 impl
///////////////////////////////////////////////////////////////////////////////
static_assert(sizeof(vec3) == 3 * sizeof(float), "vec3 is not a value type");

inline vec3::vec3(float x, float y, float z)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
}

inline vec3 vec3::operator^(const vec3& rhs) const
{
    return vec3(
        m_data[1]*rhs.m_data[2] - m_data[2]*rhs.m_data[1],
        m_data[2]*rhs.m_data[0] - m_data[0]*rhs.m_data[2],
        m_data[0]*rhs.m_data[1] - m_data[1]*rhs.m_data[0]
    );
}

///////////////////////////////////////////////////////////////////////////////
// vec4 impl
///////////////////////////////////////////////////////////////////////////////
static_assert(sizeof(vec4) == 4 * sizeof(float), "vec4 is not a value type");

inline vec4::vec4(float x, float y, float z, float w)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
    m_data[3] = w;
}

///////////////////////////////////////////////////////////////////////////////
// mat<T, D0, D1>::row_t<is_const> impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <bool is_const>
inline mat<T, D0, D1>::row_t<is_const>::row_t(ptr_t start) :
    m_start(start)
{}

template <typename T, int D0, int D1>
template <bool is_const>
inline mat<T, D0, D1>::row_t<is_const>::row_t(const row_t<false>& rhs) :
    m_start(rhs.m_start)
{}

template <typename T, int D0, int D1>
template <bool is_const>
inline typename mat<T, D0, D1>::template row_t<is_const>::ref_t
mat<T, D0, D1>::row_t<is_const>::operator[](int index)
{
    return m_start[index];
}

template <typename T, int D0, int D1>
template <bool is_const>
inline typename mat<T, D0, D1>::template row_t<is_const>::value_t
mat<T, D0, D1>::row_t<is_const>::operator[](int index) const
{
    return m_start[index];
}

template <typename T, int D0, int D1>
template <bool is_const>
inline typename mat<T, D0, D1>::row_t<is_const>& mat<T, D0, D1>::row_t<is_const>::operator=(const vec<T, D1>& rhs)
{
    return detail::iterate1<D1, set_vec>()(*this, rhs);
}

template <typename T, int D0, int D1>
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
template <typename T, int D0, int D1>
template <bool is_const>
template <int I>
inline void mat<T, D0, D1>::row_t<is_const>::set_vec<I>::operator()(row_t& out, const vec<T, D1>& rhs)
{
    out.m_start[I] = rhs[I];
}

template <typename T, int D0, int D1>
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
        // TODO: variadic templates
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs& lhs, const Rhs& rhs) const
        {
            Op<I,J>()(out, lhs, rhs);
            return iterate2<D0, D1, Op, I, J+1>()(out, lhs, rhs);
        }

        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const Rhs& rhs) const
        {
            Op<I,J>()(out, rhs);
            return iterate2<D0, D1, Op, I, J+1>()(out, rhs);
        }
    };

    template <int D0, int D1, template<int, int> class Op, int I>
    struct iterate2<D0, D1, Op, I, D1>
    {
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs& lhs, const Rhs& rhs) const
        {
            return iterate2<D0, D1, Op, I+1>()(out, lhs, rhs);
        }

        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const Rhs& rhs) const
        {
            return iterate2<D0, D1, Op, I+1>()(out, rhs);
        }
    };

    template <int D0, int D1, template<int, int> class Op>
    struct iterate2<D0, D1, Op, D0, 0>
    {
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs&, const Rhs&) const
        {
            return out;
        }

        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const Rhs&) const
        {
            return out;
        }
    };
}

static_assert(sizeof(mat4) == 16 * sizeof(float), "mat4 is not a value type");

template <typename T, int D0, int D1>
inline mat<T, D0, D1>::mat()
{
    m_data.fill(0);
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>::mat(const mat& rhs) :
    m_data(rhs.m_data)
{}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator=(const mat& rhs)
{
    return detail::iterate1<D0 * D1, eq_op>()(*this, rhs);
}

template <typename T, int D0, int D1>
template <int _D1>
inline typename std::enable_if<D0 == _D1, mat<T, D0, D1>&>::type
mat<T, D0, D1>::operator*=(const mat<T, D0, _D1>& rhs)
{
    return detail::iterate2<D0, D1, mul_op<>::mul_row>()(*this, *this, rhs);
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator*=(T rhs)
{
    return detail::iterate1<D0 * D1, scale_op>()(*this, *this, rhs);
}

template <typename T, int D0, int D1>
template <int D2>
inline mat<T, D0, D2> mat<T, D0, D1>::operator*(const mat<T, D1, D2>& rhs)
{
    no_init_tag no_init;
    mat<T, D0, D2> ret(no_init);
    return detail::iterate2<D0, D2, mul_op<>::mul_row>()(ret, *this, rhs);
}

template <typename T, int D0, int D1>
inline vec<T, D0> mat<T, D0, D1>::operator*(const vec<T, D1>& rhs)
{
    vec<T, D0> ret;
    return detail::iterate2<D0, D1, transform_op>()(ret, *this, rhs);
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1> mat<T, D0, D1>::operator*(T rhs)
{
    no_init_tag no_init;
    mat ret(no_init);
    return detail::iterate1<D0 * D1, scale_op>()(ret, *this, rhs);
}

///////////////////////////////////////////////////////////////////////////////
// mat4::eq_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I>
inline void mat<T, D0, D1>::eq_op<I>::operator()(mat& out, const mat& rhs) const
{
    out.m_data[I] = rhs.m_data[I];
}

///////////////////////////////////////////////////////////////////////////////
// mat4::scale_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I>
inline void mat<T, D0, D1>::scale_op<I>::operator()(mat& out, const mat& lhs, T rhs) const
{
    out.m_data[I] = lhs.m_data[I] * rhs;
}

///////////////////////////////////////////////////////////////////////////////
// mat4::mul_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int _D1, int K>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, K>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const
{
    out.m_data[I*D0 + J] += lhs.m_data[I*D0 + K] * rhs.m_data[K*D0 + J];
    typename mul_op<_D1, K+1>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <int _D1>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, 0>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, _D1, D2>& rhs) const
{
    // start with 0 case because we need to initialize the out matrix
    out.m_data[I*D0 + J] = lhs.m_data[I*D0] * rhs.m_data[J];
    typename mul_op<_D1, 1>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <int _D1>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<_D1, _D1>::mul_row<I, J>::operator()(mat<T, D0, D2>&, const mat&, const mat<T, _D1, D2>&) const
{}

///////////////////////////////////////////////////////////////////////////////
// mat4::transform_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I, int J>
inline void mat<T, D0, D1>::transform_op<I, J>::operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const
{
    out[I] += lhs.m_data[I*D0 + J] * rhs[J];
}

///////////////////////////////////////////////////////////////////////////////
// matrices impl
///////////////////////////////////////////////////////////////////////////////
inline mat4 mat4::identity()
{
    no_init_tag no_init;
    mat4 ret(no_init);

    ret[0] = vec4(1, 0, 0, 0);
    ret[1] = vec4(0, 1, 0, 0);
    ret[2] = vec4(0, 0, 1, 0);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}

inline mat4 mat4::translate(float x, float y, float z)
{
    no_init_tag no_init;
    mat4 ret(no_init);

    ret[0] = vec4(1, 0, 0, x);
    ret[1] = vec4(0, 1, 0, y);
    ret[2] = vec4(0, 0, 1, z);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}

inline mat4 mat4::rotate(float x, float y, float z)
{
    // yaw pitch roll = y x z (directx)
    no_init_tag no_init;
    mat4 ret(no_init);
    const float ca = cos(x), sa = sin(x);
    const float cb = cos(y), sb = sin(y);
	const float cc = cos(z), sc = sin(z);

    ret[0] = vec4(cc*cb, -sc*ca + cc*sb*sa, sc*sa + cc*sb*ca, 0);
    ret[1] = vec4(sc*cb, cc*ca + sc*sb*sa, -cc*sa + sc*sb*ca, 0);
    ret[2] = vec4(-sb, cb*sa, cb*ca, 0);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}

inline mat4 mat4::scale(float x, float y, float z)
{
    no_init_tag no_init;
    mat4 ret(no_init);

    ret[0] = vec4(x, 0, 0, 0);
    ret[1] = vec4(0, y, 0, 0);
    ret[2] = vec4(0, 0, z, 0);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}

inline mat4 mat4::lookat(const vec3& eye, const vec3& at, const vec3& up)
{
    no_init_tag no_init;
    mat4 ret(no_init);
    vec3 z = (at - eye).normalize();
    vec3 x = (z ^ up).normalize();
    vec3 y = x ^ z;

    ret[0] = vec4(x[0], x[1], x[2], -x * eye);
    ret[1] = vec4(y[0], y[1], y[2], -y * eye);
    ret[2] = vec4(-z[0], -z[1], -z[2], z * eye);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}

inline mat4 mat4::proj_perspective(float fov, float aspect, float n, float f)
{
    no_init_tag no_init;
    mat4 ret(no_init);
    const float ti = 1.0f / tan(fov / 2);

    ret[0] = vec4(ti / aspect, 0, 0, 0);
    ret[1] = vec4(0, ti, 0, 0);
    ret[2] = vec4(0, 0, f / (n-f), n * f / (n-f));
    ret[3] = vec4(0, 0, -1, 0);

    return ret;
}

inline mat4 mat4::clip(float x, float y, float w, float h, float n, float f)
{
    no_init_tag no_init;
    mat4 ret(no_init);

    ret[0] = vec4(w/2, 0, 0, x + w/2);
    ret[1] = vec4(0, h/2, 0, y + h/2);
    ret[2] = vec4(0, 0, (f - n)/2, (f + n) / 2);
    ret[3] = vec4(0, 0, 0, 1);

    return ret;
}
