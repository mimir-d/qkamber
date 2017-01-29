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
    vec& operator*=(float rhs);
    
    vec operator+(const vec& rhs) const;
    vec operator-(const vec& rhs) const;
    vec operator*(float rhs) const;

    T operator*(const vec& rhs) const;

private:
    struct no_init_tag {};
    vec(no_init_tag) {}

private:
    template <int I, typename = void>
    struct binary_op
    {
        template <typename Func>
        vec& operator()(Func fun, vec& out, const vec& lhs, const vec& rhs) const;
    };

    template <typename _>
    struct binary_op<0, _>
    {
        template <typename Func>
        vec& operator()(Func, vec& out, const vec&, const vec&) const;
    };

    template <int I, typename = void>
    struct dot
    {
        T operator()(const vec& lhs, const vec& rhs) const;
    };

    template <typename _>
    struct dot<0, _>
    {
        T operator()(const vec&, const vec&) const;
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
    vec3 operator^(const vec3& rhs);
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
namespace detail
{
    template <int D0, int D1, template<int, int> class Op, int I = 0, int J = 0>
    struct iterate2
    {
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs& lhs, const Rhs& rhs) const
        {
            Op<I,J>()(out, lhs, rhs);
            return iterate2<D0, D1, Op, I, J+1>()(out, lhs, rhs);
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
    };

    template <int D0, int D1, template<int, int> class Op>
    struct iterate2<D0, D1, Op, D0, 0>
    {
        template <typename Result, typename Lhs, typename Rhs>
        Result& operator()(Result& out, const Lhs&, const Rhs&) const
        {
            return out;
        }
    };
}

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

    row operator[](int index);
    //const_row operator[](int index) const;

    mat& operator=(const mat& rhs);

    //template <typename = std::enable_if<D0 == D1>::type>
    mat& operator*=(const mat& rhs);
    mat& operator*=(T rhs);

    template <int D2>
    mat<T, D0, D2> operator*(const mat<T, D1, D2>& rhs);
    vec<T, D0> operator*(const vec<T, D1>& rhs);
    mat operator*(T rhs);

private:
    // optimization for binary operations that overwrite internal data:
    // since there is no need to zero initialize the matrix, just skip that
    struct no_init_tag {};
    mat(no_init_tag) {}

private:
    

    template <template<int, int> class Op, int I = 0, int J = 0>
    struct binary_op
    {
        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const mat& lhs, const Rhs& rhs) const;
    };

    template <template<int, int> class Op, int I>
    struct binary_op<Op, I, D1>
    {
        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const mat& lhs, const Rhs& rhs) const;
    };

    template <template<int, int> class Op>
    struct binary_op<Op, D0, 0>
    {
        template <typename Result, typename Rhs>
        Result& operator()(Result& out, const mat&, const Rhs&) const;
    };

    template <int I, int J>
    struct eq_op
    {
        void operator()(mat& out, const mat& lhs, const mat& rhs) const;
    };

    template <int I, int J>
    struct scale_op
    {
        void operator()(mat& out, const mat& lhs, T rhs) const;
    };

    template <int K = 0, typename _ = void>
    struct mul_op
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, D1, D2>& rhs) const;
        };
    };

    template <typename _>
    struct mul_op<0, _>
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat&, const mat<T, D1, D2>&) const;
        };
    };

    template <typename _>
    struct mul_op<D1, _>
    {
        template <int I, int J>
        struct mul_row
        {
            template <int D2>
            void operator()(mat<T, D0, D2>& out, const mat&, const mat<T, D1, D2>&) const;
        };
    };

    template <int I, int J>
    struct transform_op
    {
        void operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const;
    };

private:
    std::array<T, D0 * D1> m_data;
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
    const T len2 = dot<N>()(*this, *this);
    return static_cast<T>(sqrt(len2));
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::normalize() const
{
    vec ret;
    const float factor = 1.0f / len();
    auto fun = [factor](float& vo, float v1, float) { vo = static_cast<T>(v1 * factor); };
    return binary_op<N>()(fun, ret, *this, *this);
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
    auto fun = [](float v1, float) { return v1; };
    return binary_op<N>()(fun, *this, rhs, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator+=(const vec& rhs)
{
    auto fun = [](float v1, float v2) { return v1 + v2; };
    return binary_op<N>()(fun, *this, *this, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator-=(const vec& rhs)
{
    auto fun = [](float v1, float v2) { return v1 - v2; };
    return binary_op<N>()(fun, *this, *this, rhs);
}

template <typename T, int N>
inline vec<T, N>& vec<T, N>::operator*=(float rhs)
{
    auto fun = [rhs](float v1, float) { return v1 * rhs; };
    return binary_op<N>()(fun, *this, *this, *this);
}
 
template <typename T, int N>
inline vec<T, N> vec<T, N>::operator+(const vec& rhs) const
{
    no_init_tag no_init;
    vec ret(no_init);

    auto fun = [](float v1, float v2) { return v1 + v2; };
    binary_op<N>()(fun, ret, *this, rhs);
    return ret;
}

template <typename T, int N>
inline vec<T, N> vec<T, N>::operator-(const vec& rhs) const
{
    no_init_tag no_init;
    vec ret(no_init);

    auto fun = [](float v1, float v2) { return v1 - v2; };
    binary_op<N>()(fun, ret, *this, rhs);
    return ret;
}

template <typename T, int N>
inline T vec<T, N>::operator*(const vec& rhs) const
{
    return dot<N>()(*this, rhs);
}

///////////////////////////////////////////////////////////////////////////////
// vec<T, N>::binary_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int N>
template <int I, typename _>
template <typename Func>
inline vec<T, N>& vec<T, N>::binary_op<I, _>::operator()(Func fun, vec& out, const vec& lhs, const vec& rhs) const
{
    out.m_data[I-1] = fun(lhs.m_data[I-1], rhs.m_data[I-1]);
    return binary_op<I-1>()(fun, out, lhs, rhs);
}

template <typename T, int N>
template <typename _>
template <typename Func>
inline vec<T, N>& vec<T, N>::binary_op<0, _>::operator()(Func, vec& out, const vec&, const vec&) const
{
    return out;
}

///////////////////////////////////////////////////////////////////////////////
// vec<T, N>::dot impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int N>
template <int I, typename _>
inline T vec<T, N>::dot<I, _>::operator()(const vec& lhs, const vec& rhs) const
{
    return lhs.m_data[I-1] * rhs.m_data[I-1] + dot<I-1>()(lhs, rhs);
}

template <typename T, int N>
template <typename _>
inline T vec<T, N>::dot<0, _>::operator()(const vec&, const vec&) const
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// vec3 impl
///////////////////////////////////////////////////////////////////////////////
static_assert(sizeof(vec3) == 3*sizeof(float), "vec3 is not a value type");

inline vec3::vec3(float x, float y, float z)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
}

inline vec3 vec3::operator^(const vec3& rhs)
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
static_assert(sizeof(vec4) == 4*sizeof(float), "vec4 is not a value type");

inline vec4::vec4(float x, float y, float z, float w)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
    m_data[3] = w;
}

///////////////////////////////////////////////////////////////////////////////
// mat<T, D0, D1> impl
///////////////////////////////////////////////////////////////////////////////
//static_assert(sizeof(mat4) == 16*sizeof(float), "mat4 is not a value type");

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
inline mat<T, D0, D1>::mat()
{
    m_data.fill(0);
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>::mat(T data[D0 * D1])
{
    std::copy(begin(data), end(data), m_data.begin());
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>::mat(const mat& rhs) :
    m_data(rhs.m_data)
{}

template <typename T, int D0, int D1>
inline typename mat<T, D0, D1>::row
mat<T, D0, D1>::operator[](int index)
{
    return row(m_data.data() + index * D0);
}

//template <typename T, int D0, int D1>
//inline typename mat<T, D0, D1>::const_row
//mat<T, D0, D1>::operator[](int index) const
//{
//    return const_row(m_data.data() + index * D0);
//}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator=(const mat& rhs)
{
    return binary_op<eq_op>()(*this, *this, rhs);
}

template <typename T, int D0, int D1>
//template <typename _>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator*=(const mat& rhs)
{
    return binary_op<mul_op<>::mul_row>()(*this, *this, rhs);
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1>& mat<T, D0, D1>::operator*=(T rhs)
{
    return binary_op<scale_op>()(*this, *this, rhs);
}

template <typename T, int D0, int D1>
template <int D2>
inline mat<T, D0, D2> mat<T, D0, D1>::operator*(const mat<T, D1, D2>& rhs)
{
    no_init_tag no_init;
    mat<T, D0, D2> ret(no_init);
    
    //binary_op<mul_op<>::mul_row>()(ret, *this, rhs);
    detail::iterate2<D0, D2, mul_op<>::mul_row>()(ret, *this, rhs);
    mat m;
    //detail::iterate2<D0, D1, eq_op>()(m,m,m);
    return ret;
}

template <typename T, int D0, int D1>
inline vec<T, D0> mat<T, D0, D1>::operator*(const vec<T, D1>& rhs)
{
    vec<T, D0> ret;
    binary_op<transform_op>()(ret, *this, rhs);
    return ret;
}

template <typename T, int D0, int D1>
inline mat<T, D0, D1> mat<T, D0, D1>::operator*(T rhs)
{
    no_init_tag no_init;
    mat ret(no_init);

    binary_op<scale_op>()(ret, *this, rhs);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// mat4::binary_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <template<int, int> class Op, int I, int J>
template <typename Result, typename Rhs>
inline Result& mat<T, D0, D1>::binary_op<Op, I, J>::operator()(Result& out, const mat& lhs, const Rhs& rhs) const
{
    Op<I, J>()(out, lhs, rhs);
    return binary_op<Op, I, J+1>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <template<int, int> class Op, int I>
template <typename Result, typename Rhs>
inline Result& mat<T, D0, D1>::binary_op<Op, I, D1>::operator()(Result& out, const mat& lhs, const Rhs& rhs) const
{
    return out;
    //return binary_op<Op, I+1>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <template<int, int> class Op>
template <typename Result, typename Rhs>
inline Result& mat<T, D0, D1>::binary_op<Op, D0, 0>::operator()(Result& out, const mat&, const Rhs&) const
{
    return out;
}

///////////////////////////////////////////////////////////////////////////////
// mat4::eq_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I, int J>
inline void mat<T, D0, D1>::eq_op<I, J>::operator()(mat& out, const mat& lhs, const mat& rhs) const
{
    out.m_data[I*4 + J] = rhs.m_data[I*4 + J];
}

///////////////////////////////////////////////////////////////////////////////
// mat4::scale_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I, int J>
inline void mat<T, D0, D1>::scale_op<I, J>::operator()(mat& out, const mat& lhs, T rhs) const
{
    out.m_data[I*4 + J] = lhs.m_data[I*4 + J] * rhs;
}

///////////////////////////////////////////////////////////////////////////////
// mat4::mul_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int K, typename _>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<K, _>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, D1, D2>& rhs) const
{
    out.m_data[I*4 + J] += lhs.m_data[I*4 + K] * rhs.m_data[K*4 + J];
    typename mul_op<K+1, _>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <typename _>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<0, _>::mul_row<I, J>::operator()(mat<T, D0, D2>& out, const mat& lhs, const mat<T, D1, D2>& rhs) const
{
    // start with 0 case because we need to initialize the out matrix
    out.m_data[I*4 + J] = lhs.m_data[I*4] * rhs.m_data[J];
    typename mul_op<1, _>::template mul_row<I, J>()(out, lhs, rhs);
}

template <typename T, int D0, int D1>
template <typename _>
template <int I, int J>
template <int D2>
inline void mat<T, D0, D1>::mul_op<D1, _>::mul_row<I, J>::operator()(mat<T, D0, D2>&, const mat&, const mat<T, D1, D2>&) const
{}

///////////////////////////////////////////////////////////////////////////////
// mat4::transform_op impl
///////////////////////////////////////////////////////////////////////////////
template <typename T, int D0, int D1>
template <int I, int J>
inline void mat<T, D0, D1>::transform_op<I, J>::operator()(vec<T, D0>& out, const mat& lhs, const vec<T, D1>& rhs) const
{
    out[I] += lhs.m_data[I*4 + J] * rhs[J];
}
