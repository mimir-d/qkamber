
#include "precompiled.h"
#include "software_device.h"

#include "render_primitive.h"
#include "software_buffers.h"

using namespace std;

namespace
{
    class NullTarget : public RenderTarget
    {
    public:
        ColorBuffer& get_color_buffer() final;
        DepthBuffer& get_depth_buffer() final;

        int get_width() const final;
        int get_height() const final;
    };

    inline ColorBuffer& NullTarget::get_color_buffer()
    {
        throw std::runtime_error("Attempted to draw in null target");
    }

    inline DepthBuffer& NullTarget::get_depth_buffer()
    {
        throw std::runtime_error("Attempted to draw in null target");
    }

    inline int NullTarget::get_width() const
    {
        return 0;
    }

    inline int NullTarget::get_height() const
    {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
SoftwareDevice::SoftwareDevice() :
    m_null_target(new NullTarget)
{
    set_render_target(nullptr);
}

template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

void SoftwareDevice::draw_primitive(const RenderPrimitive& primitive)
{
    const mat4& proj_matrix = m_params.get_proj_matrix();
    const mat4& mv_matrix = m_params.get_mv_matrix();
    const mat4& mvp_matrix = m_params.get_mvp_matrix();
    const mat3& normal_matrix = m_params.get_normal_matrix();
    const mat3x4& clip_matrix = m_params.get_clip_matrix();

    auto& vb = static_cast<const SoftwareVertexBuffer&>(primitive.vertices);
    auto& ib = static_cast<const SoftwareIndexBuffer&>(primitive.indices);

    // go thru declaration and figure out the offsets and data size
    int position_offset = -1;
    int normal_offset = -1;
    int color_offset = -1;
    int texcoord_offset = -1;
    for (auto& di : vb.get_declaration())
    {
        switch (di.semantic)
        {
            case VertexSemantic::Position: position_offset = static_cast<int>(di.offset); break;
            case VertexSemantic::Normal: normal_offset = static_cast<int>(di.offset); break;
            case VertexSemantic::Color: color_offset = static_cast<int>(di.offset); break;
            case VertexSemantic::Texcoord: texcoord_offset = static_cast<int>(di.offset); break;
        }
    }
    size_t vertex_size = vb.get_declaration().get_vertex_size();

    // TODO: this will need to change when index size is != uint16_t
    const uint16_t* ib_ptr = reinterpret_cast<const uint16_t*>(ib.data());

    // TODO: performance, push these to display lists and parallel process
    for (size_t i = 0; i < ib.get_count(); i += 3, ib_ptr += 3)
    {
        // TODO: cache transformed vertices with index as key
        // vertex position computations
        const float* p_p0 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[0] * vertex_size);
        const float* p_p1 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[1] * vertex_size);
        const float* p_p2 = reinterpret_cast<const float*>(vb.data() + position_offset + ib_ptr[2] * vertex_size);

        // transform to view-space
        const vec4 v0v = mv_matrix * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        const vec4 v1v = mv_matrix * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        const vec4 v2v = mv_matrix * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

        const vec3 v0v_3 = vec3{ v0v };
        const vec3 v1v_3 = vec3{ v1v };
        const vec3 v2v_3 = vec3{ v2v };

        // compute view-space normal
        const vec3 dv1 = (v2v_3 - v0v_3);
        const vec3 dv2 = (v1v_3 - v0v_3);
        // TODO: fix cross operator to work with temporaries
        const vec3 vnv = (dv1 ^ dv2).normalize();

        if (v0v_3 * vnv < 0)
        {
            // discard back-facing triangle
            continue;
        }

        // transform to clip-space
        vec4 v0c = mvp_matrix * vec4{ p_p0[0], p_p0[1], p_p0[2], 1.0f };
        vec4 v1c = mvp_matrix * vec4{ p_p1[0], p_p1[1], p_p1[2], 1.0f };
        vec4 v2c = mvp_matrix * vec4{ p_p2[0], p_p2[1], p_p2[2], 1.0f };

        // infinity transition when any vertices of the triangles are on +plane and the other on -plane
        const int s0 = sgn(v0c.w());
        const int s1 = sgn(v1c.w());
        const int s2 = sgn(v2c.w());
        if (!(s0 == s1 && s1 == s2))
            continue;

        const float wi0 = 1.0f / v0c.w();
        const float wi1 = 1.0f / v1c.w();
        const float wi2 = 1.0f / v2c.w();

        // perspective division
        v0c *= wi0;
        v1c *= wi1;
        v2c *= wi2;

        // frustrum culling - left, right view planes
        if (v0c.x() < -1.0f && v1c.x() < -1.0f && v2c.x() < -1.0f)
            continue;
        if (v0c.x() > 1.0f && v1c.x() > 1.0f && v2c.x() > 1.0f)
            continue;

        // frustrum culling - top, down view planes
        if (v0c.y() < -1.0f && v1c.y() < -1.0f && v2c.y() < -1.0f)
            continue;
        if (v0c.y() > 1.0f && v1c.y() > 1.0f && v2c.y() > 1.0f)
            continue;

        // frustrum culling - near, far view planes
        if (v0c.z() < 0.0f && v1c.z() < 0.0f && v2c.z() < 0.0f)
            continue;
        if (v0c.z() > 1.0f && v1c.z() > 1.0f && v2c.z() > 1.0f)
            continue;

        // transform to device space
        const vec3 v0d = clip_matrix * v0c;
        const vec3 v1d = clip_matrix * v1c;
        const vec3 v2d = clip_matrix * v2c;

        DevicePoint dp[3];
        dp[0].position = vec4{ v0d.x(), v0d.y(), v0c.z(), wi0 };
        dp[1].position = vec4{ v1d.x(), v1d.y(), v1c.z(), wi1 };
        dp[2].position = vec4{ v2d.x(), v2d.y(), v2c.z(), wi2 };

        dp[0].view_position = v0v_3 * wi0;
        dp[1].view_position = v1v_3 * wi1;
        dp[2].view_position = v2v_3 * wi2;

        if (normal_offset >= 0)
        {
            const float* p_n0 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[0] * vertex_size);
            const float* p_n1 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[1] * vertex_size);
            const float* p_n2 = reinterpret_cast<const float*>(vb.data() + normal_offset + ib_ptr[2] * vertex_size);

            dp[0].view_normal = (normal_matrix * vec3{ p_n0[0], p_n0[1], p_n0[2] }) * wi0;
            dp[1].view_normal = (normal_matrix * vec3{ p_n1[0], p_n1[1], p_n1[2] }) * wi1;
            dp[2].view_normal = (normal_matrix * vec3{ p_n2[0], p_n2[1], p_n2[2] }) * wi2;
        }

        if (color_offset >= 0)
        {
            const float* p_c0 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[0] * vertex_size);
            const float* p_c1 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[1] * vertex_size);
            const float* p_c2 = reinterpret_cast<const float*>(vb.data() + color_offset + ib_ptr[2] * vertex_size);

            // TODO: make a ptr-based vec3
            dp[0].color = Color{ p_c0[0], p_c0[1], p_c0[2], p_c0[3] } * wi0;
            dp[1].color = Color{ p_c1[0], p_c1[1], p_c1[2], p_c1[3] } * wi1;
            dp[2].color = Color{ p_c2[0], p_c2[1], p_c2[2], p_c2[3] } * wi2;
        }

        if (texcoord_offset >= 0)
        {
            const float* p_uv0 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[0] * vertex_size);
            const float* p_uv1 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[1] * vertex_size);
            const float* p_uv2 = reinterpret_cast<const float*>(vb.data() + texcoord_offset + ib_ptr[2] * vertex_size);

            dp[0].texcoord = vec2{ p_uv0[0], p_uv0[1] } * wi0;
            dp[1].texcoord = vec2{ p_uv1[0], p_uv1[1] } * wi1;
            dp[2].texcoord = vec2{ p_uv2[0], p_uv2[1] } * wi2;
        }

        if (m_debug_normals)
        {
            for (int i = 0; i < 3; i++)
            {
                // compute screen-space (vertex + normal)
                vec3 v_dn = dp[i].view_position.value() * (1.0f / wi0) + dp[i].view_normal.value().normalize() * 0.5f;
                vec4 v_dnc = proj_matrix * vec4{ v_dn, 1.0f };
                v_dnc *= 1.0f / v_dnc.w();
                vec3 v_dnd = clip_matrix * v_dnc;

                DevicePoint n_dp[2];
                n_dp[0].position = dp[i].position;
                n_dp[1].position = vec4{ v_dnd.x(), v_dnd.y(), 0, 0 };
                draw_lines({n_dp[0], n_dp[1]});
            }
        }

        draw_tri(dp[0], dp[1], dp[2]);
    }
}

void SoftwareDevice::draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    switch (m_poly_mode)
    {
        case PolygonMode::Point:
            draw_points({ p0, p1, p2 });
            break;

        case PolygonMode::Line:
            draw_lines({ p0, p1, p2, p0 });
            break;

        case PolygonMode::Fill:
            draw_fill(p0, p1, p2);
            break;
    }
}

namespace
{
    class lerp_halfedge
    {
        using fp4v = vec<fp4, 3>;
        using fp8v = vec<fp8, 3>;

    public:
        lerp_halfedge(const fp4v& x, const fp4v& y, fp4 x0, fp4 y0) :
            // deltas in fp4
            dx4{ x[0] - x[1], x[1] - x[2], x[2] - x[0] },
            dy4{ y[0] - y[1], y[1] - y[2], y[2] - y[0] },

            // constant part of half-edge functions + fill correction (left sides are filled on +1, right sides are not)
            const8 {
                dy4[0].denorm_mul(x[0]) - dx4[0].denorm_mul(y[0]) + (dy4[0] < 0 || (dy4[0] == 0 && dx4[0] > 0)),
                dy4[1].denorm_mul(x[1]) - dx4[1].denorm_mul(y[1]) + (dy4[1] < 0 || (dy4[1] == 0 && dx4[1] > 0)),
                dy4[2].denorm_mul(x[2]) - dx4[2].denorm_mul(y[2]) + (dy4[2] < 0 || (dy4[2] == 0 && dx4[2] > 0))
            },

            // running half-edge values, all in fixed-point 8 frac digits
            value_y {
                const8[0] + dx4[0].denorm_mul(y0) - dy4[0].denorm_mul(x0),
                const8[1] + dx4[1].denorm_mul(y0) - dy4[1].denorm_mul(x0),
                const8[2] + dx4[2].denorm_mul(y0) - dy4[2].denorm_mul(x0)
            },
            value_x{ 0, 0, 0 },
            dx{ dx4[0], dx4[1], dx4[2] },
            dy{ dy4[0], dy4[1], dy4[2] },

            // deltas in float
            fdx{ static_cast<float>(dx4[0]), static_cast<float>(dx4[1]), static_cast<float>(dx4[2]) },
            fdy{ static_cast<float>(dy4[0]), static_cast<float>(dy4[1]), static_cast<float>(dy4[2]) },

            // interpolation normalization
            lerp_norm{ 1.0f / (fdx[0] * fdy[2] - fdx[2] * fdy[0]) },

            // weights for 0,1,2 lerp dot products
            weight {
                static_cast<float>(value_y[1]) * lerp_norm,
                static_cast<float>(value_y[2]) * lerp_norm,
                static_cast<float>(value_y[0]) * lerp_norm
            },
            weight_dx { fdx[1] * lerp_norm, fdx[2] * lerp_norm, fdx[0] * lerp_norm },
            weight_dy { fdy[1] * lerp_norm, fdy[2] * lerp_norm, fdy[0] * lerp_norm }
        {
            // first iteration values
            value_x = value_y;
        }
        ~lerp_halfedge() = default;

    public:
        const fp8v& value() const
        {
            return value_x;
        }

        const vec3& w() const
        {
            return weight;
        }

        const vec3& w_dx() const
        {
            return weight_dx;
        }

        const vec3& w_dy() const
        {
            return weight_dy;
        }

        void incr_y()
        {
            value_y += dx;
            value_x = value_y;
        }

        void incr_x()
        {
            value_x -= dy;
        }

    private:
        // NOTE: these need to be places here in order for init to work correctly
        const fp4v dx4, dy4;
        const fp8v const8;
        const vec3 fdx, fdy;
        const float lerp_norm;

        fp8v value_x, value_y;
        const fp8v dx, dy;

        // weights
        const vec3 weight;
        const vec3 weight_dx;
        const vec3 weight_dy;
    };

    template <typename T>
    class lerp_attr
    {
    public:
        lerp_attr(const lerp_halfedge& he, const vec<T, 3>& attr) :
            value_y{ attr[0] * he.w()[0] + attr[1] * he.w()[1] + attr[2] * he.w()[2] },
            dx{ attr[0] * he.w_dx()[0] + attr[1] * he.w_dx()[1] + attr[2] * he.w_dx()[2] },
            dy{ attr[0] * he.w_dy()[0] + attr[1] * he.w_dy()[1] + attr[2] * he.w_dy()[2] }
        {
            // first iteration values
            m_value_x = value_y;
        }
        ~lerp_attr() = default;

        const T& value() const
        {
            return m_value_x;
        }

        void incr_y()
        {
            value_y += dx;
            m_value_x = value_y;
        }

        void incr_x()
        {
            m_value_x -= dy;
        }

    private:
        T m_value_x, value_y;
        const T dx;
        const T dy;
    };

    template <typename... Attrs>
    class lerp_pack
    {
    public:
        lerp_pack(lerp_attr<Attrs>&&... args) :
            m_data{ std::forward<lerp_attr<Attrs>>(args)... }
        {}
        ~lerp_pack() = default;

        void incr_y()
        {
            incr_y(std::make_index_sequence<sizeof...(Attrs)>());
        }

        void incr_x()
        {
            incr_x(std::make_index_sequence<sizeof...(Attrs)>());
        }

        template <size_t I, typename Attr = typename typelist_at<I, Attrs...>::type>
        const lerp_attr<Attr>& get() const
        {
            return std::get<I>(m_data);
        }

    private:
        template <size_t... I>
        void incr_y(std::index_sequence<I...>)
        {
            using swallow = int[];
            (void)swallow{ (std::get<I>(m_data).incr_y(), 0)... };
        }

        template <size_t... I>
        void incr_x(std::index_sequence<I...>)
        {
            using swallow = int[];
            (void)swallow{ (std::get<I>(m_data).incr_x(), 0)... };
        }

        tuple<lerp_attr<Attrs>...> m_data;
    };
}

void SoftwareDevice::draw_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: shamelessly stolen from http://forum.devmaster.net/t/advanced-rasterization/6145
    // TODO: read this http://www.cs.unc.edu/~olano/papers/2dh-tri/
    const vec<fp4, 3> x = { p0.position.x(), p1.position.x(), p2.position.x() };
    const vec<fp4, 3> y = { p0.position.y(), p1.position.y(), p2.position.y() };

    // TODO: if-constexpr could really benefit this function
    const vec<vec3, 3> view_positions = {
        p0.view_position.has_value() ? p0.view_position.value() : vec3{},
        p1.view_position.has_value() ? p1.view_position.value() : vec3{},
        p2.view_position.has_value() ? p2.view_position.value() : vec3{}
    };

    const vec<vec3, 3> normals = {
        p0.view_normal.has_value() ? p0.view_normal.value() : vec3{},
        p1.view_normal.has_value() ? p1.view_normal.value() : vec3{},
        p2.view_normal.has_value() ? p2.view_normal.value() : vec3{}
    };

    const vec<Color, 3> colors = {
        p0.color.has_value() ? p0.color.value() : Color{},
        p1.color.has_value() ? p1.color.value() : Color{},
        p2.color.has_value() ? p2.color.value() : Color{}
    };

    const vec<vec2, 3> texcoords = {
        p0.texcoord.has_value() ? p0.texcoord.value() : vec2{},
        p1.texcoord.has_value() ? p1.texcoord.value() : vec2{},
        p2.texcoord.has_value() ? p2.texcoord.value() : vec2{}
    };

    // min bounding box
    const int min_x = ::max(static_cast<int>(::min(x[0], x[1], x[2])), 0);
    const int max_x = ::min(static_cast<int>(::max(x[0], x[1], x[2])), m_render_target->get_width());
    const int min_y = ::max(static_cast<int>(::min(y[0], y[1], y[2])), 0);
    const int max_y = ::min(static_cast<int>(::max(y[0], y[1], y[2])), m_render_target->get_height());

    // half-edge interpolation
    lerp_halfedge he{ x, y, min_x, min_y };

    // attribute interpolation
    lerp_pack<float, float, vec3, vec3, Color, vec2> attrs
    {
        { he, { p0.position.z(), p1.position.z(), p2.position.z() } },
        { he, { p0.position.w(), p1.position.w(), p2.position.w() } },
        { he, view_positions },
        { he, normals },
        { he, colors },
        { he, texcoords }
    };

    // buffers
    auto& color_buf = m_render_target->get_color_buffer();
    const size_t color_stride = color_buf.get_stride();
    uint32_t* color_ptr = color_buf.lock() + min_y * color_stride;
    ColorBufferFormat color_format = color_buf.get_format();

    auto& depth_buf = m_render_target->get_depth_buffer();
    const size_t depth_stride = depth_buf.get_stride();
    float* depth_ptr = depth_buf.lock() + min_y * depth_stride;

    for (int y = min_y; y < max_y; y++)
    {
        for (int x = min_x; x < max_x; x++)
        {
            const float w = 1.0f / attrs.get<1>().value();
            // TODO: pretty sure this isnt right, should be 1/zi_x
            const float z = attrs.get<0>().value() * w;

            if (he.value()[0] > 0 && he.value()[1] > 0 && he.value()[2] > 0 && z < depth_ptr[x])
            {
                // TODO: alpha transparency
                const Color mat_diffuse = [&]
                {
                    if (p0.color.has_value())
                        return static_cast<Color>(attrs.get<4>().value() * w);

                    if (p0.texcoord.has_value())
                    {
                        const vec2 uv = attrs.get<5>().value() * w;
                        Color tex_color;
                        float count = 0;

                        // average all the texture units
                        for (auto unit : m_texture_units)
                        {
                            if (!unit)
                                continue;

                            auto tex = static_cast<const SoftwareTexture*>(unit);
                            tex_color += tex->sample(uv.x(), uv.y());
                            count += 1;
                        }
                        return Color{ tex_color * (1.0f / count) };
                    }

                    return m_params.get_material_diffuse();
                }();

                const Color frag_color = [&]
                {
                    if (!m_params.get_material_lighting())
                        return mat_diffuse;

                    const Color& mat_ambient = m_params.get_material_ambient();
                    const Color& mat_specular = m_params.get_material_specular();
                    const Color& mat_emissive = m_params.get_material_emissive();
                    const float mat_shininess = m_params.get_material_shininess();

                    // lighting calculations in camera-space
                    const vec3 view_pos = attrs.get<2>().value() * w;
                    const vec3 view_norm = (attrs.get<3>().value() * w).normalize();

                    Color ambient, diffuse, specular;
                    float light_count = 0;
                    for (size_t i = 0; i < m_light_units.size(); i++)
                    {
                        auto& light = m_light_units[i];
                        if (!light)
                            continue;

                        const vec3 light_dir_denorm = m_light_view_positions[i] - view_pos;
                        const vec3 light_dir = light_dir_denorm.normalize();
                        const float light_dist = light_dir_denorm.length();

                        auto& atten_coef = light->get_attenuation();
                        const float light_atten = 1.0f / (
                            atten_coef[1] +
                            atten_coef[2] * light_dist,
                            atten_coef[3] * light_dist * light_dist
                        );

                        // ambient color
                        ambient += mat_ambient % light->get_ambient() * light_atten;

                        // diffuse color
                        const float diff_coef = std::max(0.0f, view_norm * light_dir);
                        diffuse += mat_diffuse % light->get_diffuse() * diff_coef * light_atten;

                        // specular color
                        if (diff_coef > 0)
                        {
                            const vec3 half_vec = (light_dir - view_pos).normalize();
                            const float spec_coef = pow(std::max(0.0f, view_norm * half_vec), mat_shininess);
                            specular += mat_specular % light->get_specular() * spec_coef * light_atten;
                        }
                        light_count ++;
                    }

                    return Color{ (ambient + diffuse + specular) * (1.0f / light_count) + mat_emissive };
                }();

                switch (color_format)
                {
                    case ColorBufferFormat::ARGB8:
                        color_ptr[x] = (
                            (static_cast<uint8_t>(clamp(frag_color.r(), 0.0f, 1.0f) * 255.0f) << 16) +
                            (static_cast<uint8_t>(clamp(frag_color.g(), 0.0f, 1.0f) * 255.0f) <<  8) +
                            (static_cast<uint8_t>(clamp(frag_color.b(), 0.0f, 1.0f) * 255.0f))
                        );
                        break;

                    case ColorBufferFormat::xBGR8:
                        color_ptr[x] = (
                            (static_cast<uint8_t>(clamp(frag_color.b(), 0.0f, 1.0f) * 255.0f) << 16) +
                            (static_cast<uint8_t>(clamp(frag_color.g(), 0.0f, 1.0f) * 255.0f) <<  8) +
                            (static_cast<uint8_t>(clamp(frag_color.r(), 0.0f, 1.0f) * 255.0f))
                        );
                        break;

                    default:
                        throw std::runtime_error("unusable color buffer format");
                }

                depth_ptr[x] = z;
            }

            he.incr_x();
            attrs.incr_x();
        }

        he.incr_y();
        attrs.incr_y();

        color_ptr += color_stride;
        depth_ptr += depth_stride;
    }

    depth_buf.unlock();
    color_buf.unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
unique_ptr<VertexBuffer> SoftwareDevice::create_vertex_buffer(unique_ptr<VertexDecl> decl, size_t count)
{
    auto ret = unique_ptr<VertexBuffer>{ new SoftwareVertexBuffer{ move(decl), count } };
    dlog("Created vertex buffer %#x", ret.get());
    return ret;
}

unique_ptr<IndexBuffer> SoftwareDevice::create_index_buffer(size_t count)
{
    auto ret = unique_ptr<IndexBuffer>{ new SoftwareIndexBuffer{ count } };
    dlog("Created index buffer %#x", ret.get());
    return ret;
}

unique_ptr<Texture> SoftwareDevice::create_texture(size_t width, size_t height, PixelFormat format)
{
    auto ret = unique_ptr<Texture>{ new SoftwareTexture{ width, height, format } };
    dlog("Created texture %#x", ret.get());
    return ret;
}
