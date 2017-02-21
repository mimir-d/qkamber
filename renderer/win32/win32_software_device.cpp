
#include "precompiled.h"
#include "win32_software_device.h"

#include "render_primitive.h"
#include "render_buffers.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Win32RenderTarget impl
///////////////////////////////////////////////////////////////////////////////
Win32ColorBuffer::Win32ColorBuffer(HDC surface_dc) :
    m_surface_dc(surface_dc)
{
    m_dc = CreateCompatibleDC(surface_dc);
    if (GetDeviceCaps(surface_dc, BITSPIXEL) != 32)
    {
        // TODO: fix this with forcing the window bpp thru wgl
        throw exception("window needs to be 32bpp");
    }
}

void Win32ColorBuffer::resize(int width, int height)
{
    if (m_bitmap != INVALID_HANDLE_VALUE)
        DeleteObject(m_bitmap);

    BITMAPINFO bi = { 0 };
    BITMAPINFOHEADER& bmi = bi.bmiHeader;
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = GetDeviceCaps(m_surface_dc, PLANES);
    bmi.biBitCount = GetDeviceCaps(m_surface_dc, BITSPIXEL);
    // limit to 1920x1080 because of fixed point math (2048x2048 max)
    bmi.biWidth = min(width, 1920);
    bmi.biHeight = -min(height, 1080);
    bmi.biCompression = BI_RGB;

    m_bitmap = CreateDIBSection(
        m_surface_dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<PVOID*>(&m_data_ptr),
        nullptr, 0
    );

    m_stride = (bmi.biWidth * bmi.biBitCount + 0x1f) >> 5;

    SelectObject(m_dc, m_bitmap);

    SetBkMode(m_dc, TRANSPARENT);
    SetTextColor(m_dc, 0x0040FF00);
}

///////////////////////////////////////////////////////////////////////////////
// Win32DepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
void Win32DepthBuffer::resize(int width, int height)
{
    m_stride = width;
    m_data.reset(new float[height * m_stride]);
}

///////////////////////////////////////////////////////////////////////////////
// Win32SoftwareDevice
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::win32_init(HWND window_handle)
{
    flog("on hwnd = %#x", window_handle);
    m_window_handle = window_handle;

    m_surface = GetDC(m_window_handle);
    GetClientRect(m_window_handle, &m_rect);

    // always create a default render target, but leave the option to be set
    m_default_target = create_render_target();
    m_render_target = m_default_target.get();

    win32_resize(&m_rect);

    // create brushes
    m_clear_brush = CreateSolidBrush(RGB(0, 0, 0));
    m_fill_brush = CreateSolidBrush(0x009600c8);
    m_line_pen = CreatePen(PS_SOLID, 1, 0x009600c8);

    // create font
    LOGFONT font_desc = { 0 };

    font_desc.lfHeight = -MulDiv(9, GetDeviceCaps(m_surface, LOGPIXELSY), 72);
    font_desc.lfQuality = ANTIALIASED_QUALITY;
    font_desc.lfPitchAndFamily = MONO_FONT;
    strcpy_s(font_desc.lfFaceName, LF_FACESIZE, "Courier New");

    m_font = CreateFontIndirect(&font_desc);

    log_info("Created Win32SoftwareDevice on hwnd = %#x", window_handle);
}

void Win32SoftwareDevice::win32_shutdown()
{
    flog("on hwnd = %#x", m_window_handle);

    DeleteObject(m_font);
    DeleteObject(m_line_pen);
    DeleteObject(m_fill_brush);
    DeleteObject(m_clear_brush);

    ReleaseDC(m_window_handle, m_surface);
}

void Win32SoftwareDevice::win32_resize(PRECT client_rect)
{
    CopyRect(&m_rect, client_rect);

    const int width = client_rect->right - client_rect->left;
    const int height = client_rect->bottom - client_rect->top;

    m_render_target->set_width(width);
    m_render_target->set_height(height);

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    color_buf.resize(width, height);

    auto& depth_buf = static_cast<Win32DepthBuffer&>(m_render_target->get_depth_buffer());
    depth_buf.resize(width, height);

    // create zbuffer and initialization buffer because it's faster to memcpy that
    m_zbuffer_clear.reset(new float[height * depth_buf.get_stride()]);
    float* zbuff_ptr = m_zbuffer_clear.get();
    std::fill(
        zbuff_ptr, zbuff_ptr + height * depth_buf.get_stride(),
        std::numeric_limits<float>::max()
    );

    // set up backbuffer objects
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
std::unique_ptr<RenderTarget> Win32SoftwareDevice::create_render_target()
{
    return std::make_unique<RenderTarget>(
        m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
        std::unique_ptr<ColorBuffer>{ new Win32ColorBuffer{ m_surface } },
        std::unique_ptr<DepthBuffer>{ new Win32DepthBuffer }
    );
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
    memcpy(
        depth_buf.get_data(), m_zbuffer_clear.get(),
        m_render_target->get_height() * depth_buf.get_stride() * sizeof(float)
    );
}

void Win32SoftwareDevice::swap_buffers()
{
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    BitBlt(
        m_surface,
        m_rect.left, m_rect.top,
        m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
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

    vec3 c0 = p0.color * 255.f;
    vec3 c1 = p1.color * 255.f;
    vec3 c2 = p2.color * 255.f;

    float z0 = p0.position.z();
    float z1 = p1.position.z();
    float z2 = p2.position.z();

    // min bounding box
    const int min_x = ::max(static_cast<int>(::min(x0, x1, x2)), 0);
    const int max_x = ::min(static_cast<int>(::max(x0, x1, x2)), static_cast<int>(m_rect.right - m_rect.left));
    const int min_y = ::max(static_cast<int>(::min(y0, y1, y2)), 0);
    const int max_y = ::min(static_cast<int>(::max(y0, y1, y2)), static_cast<int>(m_rect.bottom - m_rect.top));

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

    // running color interpolation values
    vec3 c_y = (
        c2 * static_cast<float>(get<0>(he_y)) +
        c0 * static_cast<float>(get<1>(he_y)) +
        c1 * static_cast<float>(get<2>(he_y))
    ) * lerp_c;
    const vec3 dc_dx = (c2 * fdx01 + c0 * fdx12 + c1 * fdx20) * lerp_c;
    const vec3 dc_dy = (c2 * fdy01 + c0 * fdy12 + c1 * fdy20) * lerp_c;

    // running z-value interpolation values
    float z_y = (
        z2 * static_cast<float>(get<0>(he_y)) * lerp_c +
        z0 * static_cast<float>(get<1>(he_y)) * lerp_c +
        z1 * static_cast<float>(get<2>(he_y)) * lerp_c
    );
    const float dz_dx = (z2 * fdx01 + z0 * fdx12 + z1 * fdx20) * lerp_c;
    const float dz_dy = (z2 * fdy01 + z0 * fdy12 + z1 * fdy20) * lerp_c;

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
        vec3 c_x = c_y;
        float z_x = z_y;

        for (int x = min_x; x < max_x; x++)
        {
            // TODO: there's some zbuff fighting at 50+ distance
            if (z_x < depth_ptr[x] && he_x > 0)
            {
                color_ptr[x] = RGB(c_x.z(), c_x.y(), c_x.x());;
                depth_ptr[x] = z_x;
            }

            he_x -= he_dy;
            c_x -= dc_dy;
            z_x -= dz_dy;
        }

        he_y += he_dx;
        c_y += dc_dx;
        z_y += dz_dx;

        color_ptr += color_stride;
        depth_ptr += depth_stride;
    }
}
