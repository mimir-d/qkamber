
#include "precompiled.h"
#include "win32_software_device.h"

#include "render_primitive.h"
#include "render_buffers.h"

using namespace std;

void Win32SoftwareDevice::win32_init(HWND window_handle)
{
    flog("on hwnd = %#x", window_handle);
    m_window_handle = window_handle;

    m_frontbuffer = GetDC(m_window_handle);

    RECT rc;
    GetClientRect(m_window_handle, &rc);
    win32_resize(&rc);

    // create brushes
    m_backbuffer_brush = CreateSolidBrush(0);
    m_fill_brush = CreateSolidBrush(0x009600c8);
    m_line_pen = CreatePen(PS_SOLID, 1, 0x009600c8);

    // create font
    LOGFONT font_desc = { 0 };

    font_desc.lfHeight = -MulDiv(9, GetDeviceCaps(m_backbuffer, LOGPIXELSY), 72);
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
    DeleteObject(m_backbuffer_brush);

    delete[] m_zbuffer;
    delete[] m_zbuffer_clear;

    DeleteObject(m_backbuffer_bitmap);
    DeleteDC(m_backbuffer);
    ReleaseDC(m_window_handle, m_frontbuffer);
}

void Win32SoftwareDevice::win32_resize(PRECT client_rect)
{
    if (m_backbuffer_bitmap != INVALID_HANDLE_VALUE)
    {
        delete[] m_zbuffer;
        delete[] m_zbuffer_clear;

        DeleteObject(m_backbuffer_bitmap);
        DeleteDC(m_backbuffer);
    }

    CopyRect(&m_rect, client_rect);
    GetClientRect(m_window_handle, &m_rect);
    m_backbuffer = CreateCompatibleDC(m_frontbuffer);

    if (GetDeviceCaps(m_frontbuffer, BITSPIXEL) != 32)
    {
        // TODO: fix this with forcing the window bpp thru wgl
        throw exception("window needs to be 32bpp");
    }

    BITMAPINFO bi = { 0 };
    BITMAPINFOHEADER& bmi = bi.bmiHeader;
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = GetDeviceCaps(m_frontbuffer, PLANES);
    bmi.biBitCount = GetDeviceCaps(m_frontbuffer, BITSPIXEL);
    // limit to 1920x1080 because of fixed point math (2048x2048 max)
    bmi.biWidth = max(client_rect->right - client_rect->left, 1920L);
    bmi.biHeight = -max(client_rect->bottom - client_rect->top, 1080L);
    bmi.biCompression = BI_RGB;

    m_backbuffer_bitmap = CreateDIBSection(
        m_frontbuffer,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<PVOID*>(&m_backbuffer_bits),
        nullptr, 0
    );

    const size_t stride = ((bmi.biWidth * bmi.biBitCount + 31) / 32);
    m_backbuffer_stride = sizeof(DWORD) * stride;

    // create zbuffer and initialization buffer because it's faster to memcpy that
    m_zbuffer = new uint8_t[-bmi.biHeight * stride * sizeof(float)];
    m_zbuffer_clear = new uint8_t[-bmi.biHeight * stride * sizeof(float)];

    float* zbuff_ptr = reinterpret_cast<float*>(m_zbuffer_clear);
    std::fill(
        zbuff_ptr, zbuff_ptr + (-bmi.biHeight * m_backbuffer_stride) / sizeof(float),
        std::numeric_limits<float>::max()
    );

    // set up backbuffer objects
    SelectObject(m_backbuffer, m_backbuffer_bitmap);
    SelectObject(m_backbuffer, m_font);
    SelectObject(m_backbuffer, m_fill_brush);
    SelectObject(m_backbuffer, m_line_pen);

    SetBkMode(m_backbuffer, TRANSPARENT);
    SetTextColor(m_backbuffer, 0x0040FF00);
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_text(const std::string& text, int x, int y)
{
    TextOut(m_backbuffer, x, y, text.c_str(), static_cast<int>(text.size()));
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::clear()
{
    const size_t height = static_cast<size_t>(m_rect.bottom - m_rect.top);
    FillRect(m_backbuffer, &m_rect, m_backbuffer_brush);

    // clear the zbuffer
    memcpy(m_zbuffer, m_zbuffer_clear, height * m_backbuffer_stride);
}

void Win32SoftwareDevice::swap_buffers()
{
    BitBlt(
        m_frontbuffer,
        m_rect.left, m_rect.top,
        m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
        m_backbuffer,
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

    Ellipse(m_backbuffer, x0 - r, y0 - r, x0 + r, y0 + r);
    Ellipse(m_backbuffer, x1 - r, y1 - r, x1 + r, y1 + r);
    Ellipse(m_backbuffer, x2 - r, y2 - r, x2 + r, y2 + r);
}

void Win32SoftwareDevice::draw_tri_line(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    const int x0 = static_cast<int>(p0.position.x()), y0 = static_cast<int>(p0.position.y());
    const int x1 = static_cast<int>(p1.position.x()), y1 = static_cast<int>(p1.position.y());
    const int x2 = static_cast<int>(p2.position.x()), y2 = static_cast<int>(p2.position.y());

    MoveToEx(m_backbuffer, x0, y0, nullptr);
    LineTo(m_backbuffer, x1, y1);
    LineTo(m_backbuffer, x2, y2);
    LineTo(m_backbuffer, x0, y0);
}

namespace
{
    inline DWORD to_rgb(vec3 c)
    {
        return (static_cast<DWORD>(c.x()) << 16) + (static_cast<DWORD>(c.y()) << 8) + static_cast<DWORD>(c.z());
    }

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

    DWORD* color_ptr = reinterpret_cast<DWORD*>(m_backbuffer_bits + min_y * m_backbuffer_stride);
    float* zbuff_ptr = reinterpret_cast<float*>(m_zbuffer + min_y * m_backbuffer_stride);

    for (int y = min_y; y < max_y; y++)
    {
        // Start value for horizontal scan
        heval he_x = he_y;
        vec3 c_x = c_y;
        float z_x = z_y;

        for (int x = min_x; x < max_x; x++)
        {
            // TODO: there's some zbuff fighting at 50+ distance
            if (z_x < zbuff_ptr[x] && he_x > 0)
            {
                color_ptr[x] = to_rgb(c_x);
                zbuff_ptr[x] = z_x;
            }

            he_x -= he_dy;
            c_x -= dc_dy;
            z_x -= dz_dy;
        }

        he_y += he_dx;
        c_y += dc_dx;
        z_y += dz_dx;

        reinterpret_cast<uint8_t*&>(color_ptr) += m_backbuffer_stride;
        reinterpret_cast<uint8_t*&>(zbuff_ptr) += m_backbuffer_stride;
    }
}
