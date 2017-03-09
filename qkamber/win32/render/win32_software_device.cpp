
#include "precompiled.h"
#include "win32_software_device.h"

#include "render/render_primitive.h"
#include "render/render_buffers.h"
#include "render/software_buffers.h"
#include "win32_window.h"

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

    // TODO: temporary
    auto im0 = m_context.get_image_loader().load("tex4.bmp");
    m_tex0 = create_texture(im0.get());
    set_texture(m_tex0.get());

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
    struct heval : repeat_t<fp8, 3>
    {
        heval(fp8 a, fp8 b, fp8 c) : repeat_t<fp8, 3>(a, b, c)
        {}

        heval& operator+=(const heval& rhs)
        {
            get<0>(*this) += get<0>(rhs);
            get<1>(*this) += get<1>(rhs);
            get<2>(*this) += get<2>(rhs);
            return *this;
        }

        heval& operator-=(const heval& rhs)
        {
            get<0>(*this) -= get<0>(rhs);
            get<1>(*this) -= get<1>(rhs);
            get<2>(*this) -= get<2>(rhs);
            return *this;
        }

        bool operator>(fp8 value)
        {
            return get<0>(*this) > value && get<1>(*this) > value && get<2>(*this) > value;
        }
    };
}

// TODO: when point and line are impl with raw ops, move all of these to software device
void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: flush any gdi calls before drawing to backbuffer directly
    GdiFlush();

    // NOTE: shamelessly stolen from http://forum.devmaster.net/t/advanced-rasterization/6145
    // TODO: read this http://www.cs.unc.edu/~olano/papers/2dh-tri/
    const fp4 x0 = p0.position.x();
    const fp4 x1 = p1.position.x();
    const fp4 x2 = p2.position.x();

    const fp4 y0 = p0.position.y();
    const fp4 y1 = p1.position.y();
    const fp4 y2 = p2.position.y();

    float zi0 = p0.position.z();
    float zi1 = p1.position.z();
    float zi2 = p2.position.z();

    vec3 c0 = p0.color * 255.f;
    vec3 c1 = p1.color * 255.f;
    vec3 c2 = p2.color * 255.f;

    vec2 uv0 = p0.texcoord;
    vec2 uv1 = p1.texcoord;
    vec2 uv2 = p2.texcoord;

    float wi0 = p0.w_inv;
    float wi1 = p1.w_inv;
    float wi2 = p2.w_inv;

    // min bounding box
    const int min_x = ::max(static_cast<int>(::min(x0, x1, x2)), 0);
    const int max_x = ::min(static_cast<int>(::max(x0, x1, x2)), m_render_target->get_width());
    const int min_y = ::max(static_cast<int>(::min(y0, y1, y2)), 0);
    const int max_y = ::min(static_cast<int>(::max(y0, y1, y2)), m_render_target->get_height());

    const fp4 fp4_min_x = min_x;
    const fp4 fp4_min_y = min_y;

    // fp4 deltas
    const fp4 dx01 = x0 - x1;
    const fp4 dx12 = x1 - x2;
    const fp4 dx20 = x2 - x0;

    const fp4 dy01 = y0 - y1;
    const fp4 dy12 = y1 - y2;
    const fp4 dy20 = y2 - y0;

    // float lerp
    const float fdx01 = static_cast<float>(dx01);
    const float fdx12 = static_cast<float>(dx12);
    const float fdx20 = static_cast<float>(dx20);

    const float fdy01 = static_cast<float>(dy01);
    const float fdy12 = static_cast<float>(dy12);
    const float fdy20 = static_cast<float>(dy20);

    const float lerp_c = 1.0f / (fdx01 * fdy20 - fdx20 * fdy01);

    // constant part of half-edge functions + fill correction (left sides are filled on +1, right sides are not)
    const fp8 he_c0 = dy01.denorm_mul(x0) - dx01.denorm_mul(y0) + (dy01 < 0 || (dy01 == 0 && dx01 > 0));
    const fp8 he_c1 = dy12.denorm_mul(x1) - dx12.denorm_mul(y1) + (dy12 < 0 || (dy12 == 0 && dx12 > 0));
    const fp8 he_c2 = dy20.denorm_mul(x2) - dx20.denorm_mul(y2) + (dy20 < 0 || (dy20 == 0 && dx20 > 0));

    // running half-edge values, all in fixed-point 8 frac digits
    heval he_y =
    {
        he_c0 + dx01.denorm_mul(fp4_min_y) - dy01.denorm_mul(fp4_min_x),
        he_c1 + dx12.denorm_mul(fp4_min_y) - dy12.denorm_mul(fp4_min_x),
        he_c2 + dx20.denorm_mul(fp4_min_y) - dy20.denorm_mul(fp4_min_x)
    };
    const heval he_dx = { dx01, dx12, dx20 };
    const heval he_dy = { dy01, dy12, dy20 };

    // running z-value interpolation values
    float zi_y = (
        zi2 * static_cast<float>(get<0>(he_y)) * lerp_c +
        zi0 * static_cast<float>(get<1>(he_y)) * lerp_c +
        zi1 * static_cast<float>(get<2>(he_y)) * lerp_c
    );
    const float dzi_dx = (zi2 * fdx01 + zi0 * fdx12 + zi1 * fdx20) * lerp_c;
    const float dzi_dy = (zi2 * fdy01 + zi0 * fdy12 + zi1 * fdy20) * lerp_c;

    float wi_y = (
        wi2 * static_cast<float>(get<0>(he_y)) * lerp_c +
        wi0 * static_cast<float>(get<1>(he_y)) * lerp_c +
        wi1 * static_cast<float>(get<2>(he_y)) * lerp_c
    );
    const float dwi_dx = (wi2 * fdx01 + wi0 * fdx12 + wi1 * fdx20) * lerp_c;
    const float dwi_dy = (wi2 * fdy01 + wi0 * fdy12 + wi1 * fdy20) * lerp_c;

    // running color interpolation values
    vec3 c_y = (
        c2 * static_cast<float>(get<0>(he_y)) * lerp_c +
        c0 * static_cast<float>(get<1>(he_y)) * lerp_c +
        c1 * static_cast<float>(get<2>(he_y)) * lerp_c
    );
    const vec3 dc_dx = (c2 * fdx01 + c0 * fdx12 + c1 * fdx20) * lerp_c;
    const vec3 dc_dy = (c2 * fdy01 + c0 * fdy12 + c1 * fdy20) * lerp_c;

    // running texcoord interpolation values
    vec2 uv_y = (
        uv2 * static_cast<float>(get<0>(he_y)) * lerp_c +
        uv0 * static_cast<float>(get<1>(he_y)) * lerp_c +
        uv1 * static_cast<float>(get<2>(he_y)) * lerp_c
    );
    const vec2 duv_dx = (uv2 * fdx01 + uv0 * fdx12 + uv1 * fdx20) * lerp_c;
    const vec2 duv_dy = (uv2 * fdy01 + uv0 * fdy12 + uv1 * fdy20) * lerp_c;

    // buffers
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    const size_t color_stride = color_buf.get_stride();
    DWORD* color_ptr = color_buf.get_data() + min_y * color_stride;

    auto& depth_buf = static_cast<Win32DepthBuffer&>(m_render_target->get_depth_buffer());
    const size_t depth_stride = depth_buf.get_stride();
    float* depth_ptr = depth_buf.get_data() + min_y * depth_stride;

    for (int y = min_y; y < max_y; y++)
    {
        // Start value for horizontal scan
        heval he_x = he_y;
        float zi_x = zi_y;
        float wi_x = wi_y;
        vec3 c_x = c_y;
        vec2 uv_x = uv_y;

        for (int x = min_x; x < max_x; x++)
        {
            const float w = 1.0f / wi_x;
            // TODO: pretty sure this isnt right, should be 1/zi_x
            const float z = zi_x * w;

            if (z < depth_ptr[x] && he_x > 0)
            {
                const vec3 c = c_x * w;

                // TODO: get texture unit and sample uv; split raster func in variant
                SoftwareTexture* tex = static_cast<SoftwareTexture*>(m_texture);
                const uint8_t* texel = tex->sample(uv_x.x() * w, uv_x.y() * w);
                // ignore alpha atm

                color_ptr[x] = RGB(
                    (2.0f * texel[0] + c.z()) / 3.0f,
                    (2.0f * texel[1] + c.y()) / 3.0f,
                    (2.0f * texel[2] + c.x()) / 3.0f
                );
                depth_ptr[x] = z;
            }

            he_x -= he_dy;
            zi_x -= dzi_dy;
            wi_x -= dwi_dy;
            c_x -= dc_dy;
            uv_x -= duv_dy;
        }

        he_y += he_dx;
        zi_y += dzi_dx;
        wi_y += dwi_dx;
        c_y += dc_dx;
        uv_y += duv_dx;

        color_ptr += color_stride;
        depth_ptr += depth_stride;
    }
}
