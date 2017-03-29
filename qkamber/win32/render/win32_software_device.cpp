
#include "precompiled.h"
#include "win32_software_device.h"

#include "win32_window.h"
#include "render/material.h"
#include "render/render_primitive.h"
#include "render/render_buffers.h"
#include "render/software_buffers.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Win32SoftwareDevice
///////////////////////////////////////////////////////////////////////////////
Win32SoftwareDevice::Win32SoftwareDevice(QkEngine::Context& context) :
    m_context(context),
    m_font(reinterpret_cast<HFONT>(INVALID_HANDLE_VALUE))
{
    flog("id = %#x", this);

    // create drawing stuff
    m_clear_brush = CreateSolidBrush(RGB(0, 0, 0));
    m_fill_brush = CreateSolidBrush(0x009600c8);
    m_line_pen = CreatePen(PS_SOLID, 1, 0x009600c8);

    log_info("Created win32 software device");
}

Win32SoftwareDevice::~Win32SoftwareDevice()
{
    flog();

    DeleteObject(m_line_pen);
    DeleteObject(m_fill_brush);
    DeleteObject(m_clear_brush);
    DeleteObject(m_font);

    log_info("Destroyed win32 software device");
}

void Win32SoftwareDevice::set_render_target(RenderTarget* target)
{
    flog();

    SoftwareDevice::set_render_target(target);
    if (!target)
        return;

    // TODO: future, target might be a texture so extract just the DC
    auto window = static_cast<Win32Window*>(target);

    // create font
    LOGFONT font_desc = { 0 };

    font_desc.lfHeight = -MulDiv(9, GetDeviceCaps(window->get_dc(), LOGPIXELSY), 72);
    font_desc.lfQuality = ANTIALIASED_QUALITY;
    font_desc.lfPitchAndFamily = MONO_FONT;
    strcpy_s(font_desc.lfFaceName, LF_FACESIZE, "Courier New");

    m_font = CreateFontIndirect(&font_desc);

    // set up backbuffer objects
    auto& color_buf = static_cast<Win32ColorBuffer&>(target->get_color_buffer());
    HDC backbuffer = color_buf.get_dc();
    SelectObject(backbuffer, m_font);
    SelectObject(backbuffer, m_fill_brush);
    SelectObject(backbuffer, m_line_pen);
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_text(const std::string& text, int x, int y)
{
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    TextOut(color_buf.get_dc(), x, y, text.c_str(), static_cast<int>(text.size()));
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<RenderTarget> Win32SoftwareDevice::create_render_target(int width, int height)
{
    flog();

    // TODO: or texture, etc..
    log_info("Creating win32 window render target...");
    return std::unique_ptr<RenderTarget>{ new Win32Window{ m_context, width, height } };
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::clear()
{
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    RECT rc = { 0, 0, m_render_target->get_width(), m_render_target->get_height() };
    FillRect(color_buf.get_dc(), &rc, m_clear_brush);

    // clear the zbuffer
    auto& depth_buf = static_cast<Win32DepthBuffer&>(m_render_target->get_depth_buffer());
    std::fill(
        depth_buf.get_data(),
        depth_buf.get_data() + m_render_target->get_height() * depth_buf.get_stride(),
        std::numeric_limits<float>::max()
    );
}

void Win32SoftwareDevice::swap_buffers()
{
    HDC surface = static_cast<Win32Window*>(m_render_target)->get_dc();
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    BitBlt(
        surface,
        0, 0, m_render_target->get_width(), m_render_target->get_height(),
        color_buf.get_dc(),
        0, 0,
        SRCCOPY
    );
}

///////////////////////////////////////////////////////////////////////////////
// Low-level drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    switch (m_poly_mode)
    {
        case PolygonMode::Point: draw_tri_point(p0, p1, p2); break;
        case PolygonMode::Line:  draw_tri_line(p0, p1, p2);  break;
        case PolygonMode::Fill:  draw_tri_fill(p0, p1, p2);  break;
    }
}

void Win32SoftwareDevice::draw_line(const DevicePoint& p0, const DevicePoint& p1)
{
    const int x0 = static_cast<int>(p0.position.x()), y0 = static_cast<int>(p0.position.y());
    const int x1 = static_cast<int>(p1.position.x()), y1 = static_cast<int>(p1.position.y());

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    HDC dc = color_buf.get_dc();

    MoveToEx(dc, x0, y0, nullptr);
    LineTo(dc, x1, y1);
}

void Win32SoftwareDevice::draw_tri_point(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    const int r = 3;
    const int x0 = static_cast<int>(p0.position.x()), y0 = static_cast<int>(p0.position.y());
    const int x1 = static_cast<int>(p1.position.x()), y1 = static_cast<int>(p1.position.y());
    const int x2 = static_cast<int>(p2.position.x()), y2 = static_cast<int>(p2.position.y());

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    HDC dc = color_buf.get_dc();

    Ellipse(dc, x0 - r, y0 - r, x0 + r, y0 + r);
    Ellipse(dc, x1 - r, y1 - r, x1 + r, y1 + r);
    Ellipse(dc, x2 - r, y2 - r, x2 + r, y2 + r);
}

void Win32SoftwareDevice::draw_tri_line(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    const int x0 = static_cast<int>(p0.position.x()), y0 = static_cast<int>(p0.position.y());
    const int x1 = static_cast<int>(p1.position.x()), y1 = static_cast<int>(p1.position.y());
    const int x2 = static_cast<int>(p2.position.x()), y2 = static_cast<int>(p2.position.y());

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    HDC dc = color_buf.get_dc();

    MoveToEx(dc, x0, y0, nullptr);
    LineTo(dc, x1, y1);
    LineTo(dc, x2, y2);
    LineTo(dc, x0, y0);
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

// NOTE: this crashes the vs2015 compiler spectacularly, ill just leave it here
// auto c_init = []
// {
//     return vec<vec3, 3>{};
// }();

//template <bool with_color, bool with_tex>
void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: flush any gdi calls before drawing to backbuffer directly
    GdiFlush();

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
        p0.color.has_value() ? p0.color.value() : vec4{},
        p1.color.has_value() ? p1.color.value() : vec4{},
        p2.color.has_value() ? p2.color.value() : vec4{}
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
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    const size_t color_stride = color_buf.get_stride();
    DWORD* color_ptr = color_buf.get_data() + min_y * color_stride;

    auto& depth_buf = static_cast<Win32DepthBuffer&>(m_render_target->get_depth_buffer());
    const size_t depth_stride = depth_buf.get_stride();
    float* depth_ptr = depth_buf.get_data() + min_y * depth_stride;

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
                const Color ambient = m_material->get_ambient();
                const Color diffuse = [&]
                {
                    if (p0.color.has_value())
                        return static_cast<Color>(attrs.get<4>().value() * w);

                    const auto& textures = m_material->get_textures();
                    if (p0.texcoord.has_value() && textures.size() > 0)
                    {
                        // TODO: only 1 texture available atm
                        auto tex = static_cast<const SoftwareTexture*>(textures[0]);
                        const vec2 uv = attrs.get<5>().value() * w;
                        const uint8_t* c = tex->sample(uv.x(), uv.y());
                        // not quite like this
                        return Color{ c[0] / 255.0f, c[1] / 255.0f, c[2] / 255.0f, c[3] / 255.0f };
                    }

                    return m_material->get_diffuse();
                }();

                // lighting calculations in camera-space
                const vec3 view_pos = attrs.get<2>().value() * w;
                const vec3 n = (attrs.get<3>().value() * w).normalize();

                // TODO: temp
                const vec3 light_dir_denorm = vec3{ m_light_pos } - view_pos;
                const vec3 light_dir = light_dir_denorm.normalize();
                const float light_dist = light_dir_denorm.length();

                const float light_attenuation_coef[3] = { 0.0f, 0.0005f, 0.0025f };
                const float light_atten = 1.0f / (
                    light_attenuation_coef[0] +
                    light_attenuation_coef[1] * light_dist,
                    light_attenuation_coef[2] * light_dist * light_dist
                );

                float amb_coef = 0.2f;
                float diff_coef = max(0.0f, n * light_dir) * light_atten;

                const Color frag_color = ambient * amb_coef + diffuse * diff_coef;
                color_ptr[x] = RGB(
                    static_cast<uint8_t>(clamp(frag_color.b(), 0.0f, 1.0f) * 255.0f),
                    static_cast<uint8_t>(clamp(frag_color.g(), 0.0f, 1.0f) * 255.0f),
                    static_cast<uint8_t>(clamp(frag_color.r(), 0.0f, 1.0f) * 255.0f)
                );
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
}
