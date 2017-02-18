
#include "precompiled.h"
#include "win32_software_device.h"

#include "render_primitive.h"
#include "render_buffers.h"

using namespace std;
using namespace Gdiplus;

void Win32SoftwareDevice::win32_init(HWND window_handle)
{
    flog("on hwnd = %#x", window_handle);
    m_window_handle = window_handle;

    GdiplusStartupInput gdip_startup;
    GdiplusStartup(&m_gdiplus_token, &gdip_startup, nullptr);

    m_frontbuffer = GetDC(m_window_handle);
    m_backbuffer_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    RECT rc;
    GetClientRect(m_window_handle, &rc);
    win32_resize(&rc);
}

void Win32SoftwareDevice::win32_shutdown()
{
    flog("on hwnd = %#x", m_window_handle);

    // delete backbuffer
    DeleteObject(m_backbuffer_brush);

    DeleteObject(m_backbuffer_bitmap);
    DeleteDC(m_backbuffer);
    ReleaseDC(m_window_handle, m_frontbuffer);

    GdiplusShutdown(m_gdiplus_token);
}

void Win32SoftwareDevice::win32_resize(PRECT client_rect)
{
    if (m_backbuffer_bitmap != INVALID_HANDLE_VALUE)
    {
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

    m_backbuffer_stride = ((bmi.biWidth * bmi.biBitCount + 31) / 32);

    SelectObject(m_backbuffer, m_backbuffer_bitmap);
    m_graphics = unique_ptr<Graphics>(new Graphics(m_backbuffer));
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_text(const std::string& text, int x, int y)
{
    const Font font(L"Consolas", 10);
    const SolidBrush hb(Color(150, 0, 200));
    const PointF p(static_cast<float>(x), static_cast<float>(y));

    unique_ptr<wchar_t[]> wcbuf(new wchar_t[text.size() + 1]);
    mbstowcs_s(nullptr, wcbuf.get(), text.size() + 1, text.c_str(), text.size());
    m_graphics->DrawString(wcbuf.get(), static_cast<INT>(text.size()), &font, p, &hb);
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::clear()
{
    RECT rc;
    GetClientRect(m_window_handle, &rc);
    FillRect(m_backbuffer, &rc, m_backbuffer_brush);
}

void Win32SoftwareDevice::swap_buffers()
{
    BitBlt(m_frontbuffer,
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
    const SolidBrush hb(Color(150, 0, 200));

    m_graphics->FillEllipse(&hb, p0.position.x(), p0.position.y(), 5.0f, 5.0f);
    m_graphics->FillEllipse(&hb, p1.position.x(), p1.position.y(), 5.0f, 5.0f);
    m_graphics->FillEllipse(&hb, p2.position.x(), p2.position.y(), 5.0f, 5.0f);
}

void Win32SoftwareDevice::draw_tri_line(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    const Pen p(Color(255, 150, 0, 255));
    const PointF points[] =
    {
        {p0.position.x(), p0.position.y()},
        {p1.position.x(), p1.position.y()},
        {p2.position.x(), p2.position.y()}
    };
    const size_t count = sizeof(points) / sizeof(points[0]);

    m_graphics->DrawPolygon(&p, points, count);
}

namespace
{
    inline DWORD to_rgb(vec3 c)
    {
        return (static_cast<DWORD>(c.x()) << 16) + (static_cast<DWORD>(c.y()) << 8) + static_cast<DWORD>(c.z());
    }
}

void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: shamelessly stolen from http://forum.devmaster.net/t/advanced-rasterization/6145
    // TODO: read this http://www.cs.unc.edu/~olano/papers/2dh-tri/
    const int x0 = to_fp4(p0.position.x());
    const int x1 = to_fp4(p1.position.x());
    const int x2 = to_fp4(p2.position.x());

    const int y0 = to_fp4(p0.position.y());
    const int y1 = to_fp4(p1.position.y());
    const int y2 = to_fp4(p2.position.y());

    vec3 c0 = p0.color * 255.f;
    vec3 c1 = p1.color * 255.f;
    vec3 c2 = p2.color * 255.f;

    // min bounding box
    const int min_x = ::max((::min(x0, x1, x2) + 0xf) >> 4, 0);
    const int max_x = ::min((::max(x0, x1, x2) + 0xf) >> 4, static_cast<int>(m_rect.right - m_rect.left));
    const int min_y = ::max((::min(y0, y1, y2) + 0xf) >> 4, 0);
    const int max_y = ::min((::max(y0, y1, y2) + 0xf) >> 4, static_cast<int>(m_rect.bottom - m_rect.top));

    const int fp4_min_x = min_x << 4;
    const int fp4_min_y = min_y << 4;

    // deltas
    const int dx01 = x0 - x1;
    const int dx12 = x1 - x2;
    const int dx20 = x2 - x0;

    const int dy01 = y0 - y1;
    const int dy12 = y1 - y2;
    const int dy20 = y2 - y0;

    const float fdx01 = 0.0625f * dx01;
    const float fdx12 = 0.0625f * dx12;
    const float fdx20 = 0.0625f * dx20;

    const float fdy01 = 0.0625f * dy01;
    const float fdy12 = 0.0625f * dy12;
    const float fdy20 = 0.0625f * dy20;

    const float lerp_c = 1.0f / (fdx01 * fdy20 - fdx20 * fdy01);
    const vec3 dc_dx = (c2 * fdx01 + c0 * fdx12 + c1 * fdx20) * lerp_c;
    const vec3 dc_dy = (c2 * fdy01 + c0 * fdy12 + c1 * fdy20) * lerp_c;

    // constant part of half-edge functions + fill correction (left sides are filled on +1, right sides are not)
    const int fp8_c0 = dy01 * x0 - dx01 * y0 + (dy01 < 0 || (dy01 == 0 && dx01 > 0));
    const int fp8_c1 = dy12 * x1 - dx12 * y1 + (dy12 < 0 || (dy12 == 0 && dx12 > 0));
    const int fp8_c2 = dy20 * x2 - dx20 * y2 + (dy20 < 0 || (dy20 == 0 && dx20 > 0));

    // running half-edge values, all in fixed-point 8 frac digits
    int he_y0 = fp8_c0 + dx01 * fp4_min_y - dy01 * fp4_min_x;
    int he_y1 = fp8_c1 + dx12 * fp4_min_y - dy12 * fp4_min_x;
    int he_y2 = fp8_c2 + dx20 * fp4_min_y - dy20 * fp4_min_x;

    float fhe_y0 = 0.00390625f * he_y0;
    float fhe_y1 = 0.00390625f * he_y1;
    float fhe_y2 = 0.00390625f * he_y2;
    vec3 c_y = (c2 * fhe_y0 + c0 * fhe_y1 + c1 * fhe_y2) * lerp_c;

    const int fp8_dx01 = dx01 << 4;
    const int fp8_dx12 = dx12 << 4;
    const int fp8_dx20 = dx20 << 4;

    const int fp8_dy01 = dy01 << 4;
    const int fp8_dy12 = dy12 << 4;
    const int fp8_dy20 = dy20 << 4;

    DWORD* color_ptr = m_backbuffer_bits + min_y * m_backbuffer_stride;

    for (int y = min_y; y < max_y; y++)
    {
        // Start value for horizontal scan
        int he_x0 = he_y0;
        int he_x1 = he_y1;
        int he_x2 = he_y2;
        vec3 c_x = c_y;

        for (int x = min_x; x < max_x; x++)
        {
            if (he_x0 > 0 && he_x1 > 0 && he_x2 > 0)
                color_ptr[x] = to_rgb(c_x);

            he_x0 -= fp8_dy01;
            he_x1 -= fp8_dy12;
            he_x2 -= fp8_dy20;
            c_x -= dc_dy;
        }

        he_y0 += fp8_dx01;
        he_y1 += fp8_dx12;
        he_y2 += fp8_dx20;
        c_y += dc_dx;
        color_ptr += m_backbuffer_stride;
    }
}
