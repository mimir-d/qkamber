
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

#define RASTER_FLOAT_1x1

#ifdef RASTER_FLOAT_1x1
void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: shamelessly stolen from http://forum.devmaster.net/t/advanced-rasterization/6145
    // TODO: read this http://www.cs.unc.edu/~olano/papers/2dh-tri/
    const float x0 = p0.position.x();
    const float x1 = p1.position.x();
    const float x2 = p2.position.x();

    const float y0 = p0.position.y();
    const float y1 = p1.position.y();
    const float y2 = p2.position.y();

    vec3 c0 = p0.color * 255.f;
    vec3 c1 = p1.color * 255.f;
    vec3 c2 = p2.color * 255.f;

    // min bounding box
    const int min_x = ::max(static_cast<int>(::min(x0, x1, x2)), 0);
    const int max_x = ::min(static_cast<int>(::max(x0, x1, x2)), static_cast<int>(m_rect.right - m_rect.left));
    const int min_y = ::max(static_cast<int>(::min(y0, y1, y2)), 0);
    const int max_y = ::min(static_cast<int>(::max(y0, y1, y2)), static_cast<int>(m_rect.bottom - m_rect.top));

    // deltas
    const float dx01 = x0 - x1;
    const float dx12 = x1 - x2;
    const float dx20 = x2 - x0;

    const float dy01 = y0 - y1;
    const float dy12 = y1 - y2;
    const float dy20 = y2 - y0;

    const float lerp_c = 1.0f / (dx01 * dy20 - dx20 * dy01);
    // vec3 dc01 = p1.color - p0.color;
    // vec3 dc20 = p2.color - p0.color;
    const vec3 dc_dx = (c2 * dx01 + c0 * dx12 + c1 * dx20) * lerp_c;
    const vec3 dc_dy = (c2 * dy01 + c0 * dy12 + c1 * dy20) * lerp_c;

    // constant part of half-edge functions
    const float he_c0 = dy01 * x0 - dx01 * y0;
    const float he_c1 = dy12 * x1 - dx12 * y1;
    const float he_c2 = dy20 * x2 - dx20 * y2;

    // running half-edge values
    float he_y0 = he_c0 + dx01 * min_y - dy01 * min_x;
    float he_y1 = he_c1 + dx12 * min_y - dy12 * min_x;
    float he_y2 = he_c2 + dx20 * min_y - dy20 * min_x;
    vec3 c_y = (c2 * he_y0 + c0 * he_y1 + c1 * he_y2) * lerp_c;

    DWORD* color_ptr = m_backbuffer_bits + min_y * m_backbuffer_stride;
    for (int y = min_y; y <= max_y; y++)
    {
        // Start value for horizontal scan
        float he_x0 = he_y0;
        float he_x1 = he_y1;
        float he_x2 = he_y2;
        vec3 c_x = c_y;

        for (int x = min_x; x <= max_x; x++)
        {
            if (he_x0 > 0 && he_x1 > 0 && he_x2 > 0)
                color_ptr[x] = to_rgb(c_x);

            he_x0 -= dy01;
            he_x1 -= dy12;
            he_x2 -= dy20;
            c_x -= dc_dy;
        }

        he_y0 += dx01;
        he_y1 += dx12;
        he_y2 += dx20;
        c_y += dc_dx;
        color_ptr += m_backbuffer_stride;
    }
}
#elif defined(RASTER_FLOAT_8x8)
void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    // NOTE: shamelessly stolen from http://forum.devmaster.net/t/advanced-rasterization/6145
    // TODO: read this http://www.cs.unc.edu/~olano/papers/2dh-tri/
    const float x0 = p0.position.x();
    const float x1 = p1.position.x();
    const float x2 = p2.position.x();

    const float y0 = p0.position.y();
    const float y1 = p1.position.y();
    const float y2 = p2.position.y();

    vec3 c0 = p0.color * 255.f;
    vec3 c1 = p1.color * 255.f;
    vec3 c2 = p2.color * 255.f;

    // min bounding box
    int min_x = ::max(static_cast<int>(::min(x0, x1, x2)), 0);
    int max_x = ::min(static_cast<int>(::max(x0, x1, x2)), static_cast<int>(m_rect.right - m_rect.left));
    int min_y = ::max(static_cast<int>(::min(y0, y1, y2)), 0);
    int max_y = ::min(static_cast<int>(::max(y0, y1, y2)), static_cast<int>(m_rect.bottom - m_rect.top));

    // deltas
    const float dx01 = x0 - x1;
    const float dx12 = x1 - x2;
    const float dx20 = x2 - x0;

    const float dy01 = y0 - y1;
    const float dy12 = y1 - y2;
    const float dy20 = y2 - y0;

    const float lerp_c = 1.0f / (dx01 * dy20 - dx20 * dy01);
    const vec3 dc_dx = (c2 * dx01 + c0 * dx12 + c1 * dx20) * lerp_c;
    const vec3 dc_dy = (c2 * dy01 + c0 * dy12 + c1 * dy20) * lerp_c;

    // Block size, standard 8x8 (must be power of two)
    const int q = 8;

    // Start in corner of 8x8 block
    min_x &= ~(q - 1);
    min_y &= ~(q - 1);

    // constant part of half-edge functions
    const float he_c0 = dy01 * x0 - dx01 * y0;
    const float he_c1 = dy12 * x1 - dx12 * y1;
    const float he_c2 = dy20 * x2 - dx20 * y2;

    DWORD* color_ptr = m_backbuffer_bits + min_y * m_backbuffer_stride;

    for (int y = min_y; y <= max_y; y += q)
    {
        for (int x = min_x; x <= max_x; x += q)
        {
            // Corners of block
            int cx0 = x;
            int cx1 = (x + q - 1);
            int cy0 = y;
            int cy1 = (y + q - 1);

            // Evaluate half-space functions
            bool a00 = he_c0 + dx01 * cy0 - dy01 * cx0 > 0;
            bool a10 = he_c0 + dx01 * cy0 - dy01 * cx1 > 0;
            bool a01 = he_c0 + dx01 * cy1 - dy01 * cx0 > 0;
            bool a11 = he_c0 + dx01 * cy1 - dy01 * cx1 > 0;
            int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

            bool b00 = he_c1 + dx12 * cy0 - dy12 * cx0 > 0;
            bool b10 = he_c1 + dx12 * cy0 - dy12 * cx1 > 0;
            bool b01 = he_c1 + dx12 * cy1 - dy12 * cx0 > 0;
            bool b11 = he_c1 + dx12 * cy1 - dy12 * cx1 > 0;
            int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

            bool c00 = he_c2 + dx20 * cy0 - dy20 * cx0 > 0;
            bool c10 = he_c2 + dx20 * cy0 - dy20 * cx1 > 0;
            bool c01 = he_c2 + dx20 * cy1 - dy20 * cx0 > 0;
            bool c11 = he_c2 + dx20 * cy1 - dy20 * cx1 > 0;
            int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

            // skip block when outside an edge
            if ((a | b | c) == 0x0) continue;

            DWORD* buffer = color_ptr;

            float he_y0 = he_c0 + dx01 * y - dy01 * x;
            float he_y1 = he_c1 + dx12 * y - dy12 * x;
            float he_y2 = he_c2 + dx20 * y - dy20 * x;
            vec3 c_y = (c2 * he_y0 + c0 * he_y1 + c1 * he_y2) * lerp_c;

            if ((a & b & c) == 0xF)
            {
                for (int iy = 0; iy < q; iy++)
                {
                    vec3 c_x = c_y;

                    for (int ix = x; ix < x + q; ix++)
                    {
                        buffer[ix] = to_rgb(c_x);
                        c_x -= dc_dy;
                    }
                    c_y += dc_dx;

                    buffer += m_backbuffer_stride;
                }
            }
            else
            {
                for (int iy = y; iy < y + q; iy++)
                {
                    float he_x0 = he_y0;
                    float he_x1 = he_y1;
                    float he_x2 = he_y2;
                    vec3 c_x = c_y;

                    for (int ix = x; ix < x + q; ix++)
                    {
                        if (he_x0 > 0 && he_x1 > 0 && he_x2 > 0)
                        {
                            buffer[ix] = to_rgb(c_x);
                        }

                        he_x0 -= dy01;
                        he_x1 -= dy12;
                        he_x2 -= dy20;
                        c_x -= dc_dy;
                    }

                    he_y0 += dx01;
                    he_y1 += dx12;
                    he_y2 += dx20;
                    c_y += dc_dx;

                    buffer += m_backbuffer_stride;
                }
            }
        }

        color_ptr += q* m_backbuffer_stride;
    }
}
#elif defined(RASTER_FP_1x1)
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
#elif defined(RASTER_FP_8x8)
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
    int min_x = ::max((::min(x0, x1, x2) + 0xf) >> 4, 0);
    int max_x = ::min((::max(x0, x1, x2) + 0xf) >> 4, static_cast<int>(m_rect.right - m_rect.left));
    int min_y = ::max((::min(y0, y1, y2) + 0xf) >> 4, 0);
    int max_y = ::min((::max(y0, y1, y2) + 0xf) >> 4, static_cast<int>(m_rect.bottom - m_rect.top));

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

    // Block size, standard 8x8 (must be power of two)
    const int q = 8;

    // Start in corner of 8x8 block
    min_x &= ~(q - 1);
    min_y &= ~(q - 1);

    // constant part of half-edge functions + fill correction (left sides are filled on +1, right sides are not)
    const int fp8_c0 = dy01 * x0 - dx01 * y0 + (dy01 < 0 || (dy01 == 0 && dx01 > 0));
    const int fp8_c1 = dy12 * x1 - dx12 * y1 + (dy12 < 0 || (dy12 == 0 && dx12 > 0));
    const int fp8_c2 = dy20 * x2 - dx20 * y2 + (dy20 < 0 || (dy20 == 0 && dx20 > 0));

    const int fp8_dx01 = dx01 << 4;
    const int fp8_dx12 = dx12 << 4;
    const int fp8_dx20 = dx20 << 4;

    const int fp8_dy01 = dy01 << 4;
    const int fp8_dy12 = dy12 << 4;
    const int fp8_dy20 = dy20 << 4;

    DWORD* color_ptr = m_backbuffer_bits + min_y * m_backbuffer_stride;

    for (int y = min_y; y < max_y; y += q)
    {
        for (int x = min_x; x < max_x; x += q)
        {
            // Corners of block
            int cx0 = x << 4;
            int cx1 = (x + q - 1) << 4;
            int cy0 = y << 4;
            int cy1 = (y + q - 1) << 4;

            // Evaluate half-space functions
            bool a00 = fp8_c0 + dx01 * cy0 - dy01 * cx0 > 0;
            bool a10 = fp8_c0 + dx01 * cy0 - dy01 * cx1 > 0;
            bool a01 = fp8_c0 + dx01 * cy1 - dy01 * cx0 > 0;
            bool a11 = fp8_c0 + dx01 * cy1 - dy01 * cx1 > 0;
            int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

            bool b00 = fp8_c1 + dx12 * cy0 - dy12 * cx0 > 0;
            bool b10 = fp8_c1 + dx12 * cy0 - dy12 * cx1 > 0;
            bool b01 = fp8_c1 + dx12 * cy1 - dy12 * cx0 > 0;
            bool b11 = fp8_c1 + dx12 * cy1 - dy12 * cx1 > 0;
            int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

            bool c00 = fp8_c2 + dx20 * cy0 - dy20 * cx0 > 0;
            bool c10 = fp8_c2 + dx20 * cy0 - dy20 * cx1 > 0;
            bool c01 = fp8_c2 + dx20 * cy1 - dy20 * cx0 > 0;
            bool c11 = fp8_c2 + dx20 * cy1 - dy20 * cx1 > 0;
            int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

            // Skip block when outside an edge
            //if (a == 0x0 || b == 0x0 || c == 0x0) continue;
            if ((a | b | c) == 0x0) continue;

            DWORD* buffer = color_ptr;

            int he_y0 = fp8_c0 + dx01 * cy0 - dy01 * cx0;
            int he_y1 = fp8_c1 + dx12 * cy0 - dy12 * cx0;
            int he_y2 = fp8_c2 + dx20 * cy0 - dy20 * cx0;

            float fhe_y0 = 0.00390625f * he_y0;
            float fhe_y1 = 0.00390625f * he_y1;
            float fhe_y2 = 0.00390625f * he_y2;

            // float fhe_y0 = f_c0 + fdx01 * y - fdy01 * x;
            // float fhe_y1 = f_c1 + fdx12 * y - fdy12 * x;
            // float fhe_y2 = f_c2 + fdx20 * y - fdy20 * x;
            // vec3 c_y = (c2 * fhe_y0 + c0 * fhe_y1 + c1 * fhe_y2) * lerp_c;
            vec3 c_y = (c2 * fhe_y0 + c0 * fhe_y1 + c1 * fhe_y2) * lerp_c;

            //if (a == 0xF && b == 0xF && c == 0xF)
            if ((a & b & c) == 0xF)
            {
                for (int iy = 0; iy < q; iy++)
                {
                    vec3 c_x = c_y;
                    for (int ix = x; ix < x + q; ix++)
                    {
                        buffer[ix] = to_rgb(c_x);
                        c_x -= dc_dy;
                    }

                    c_y += dc_dx;
                    buffer += m_backbuffer_stride;
                }
            }
            else
            {


                for (int iy = y; iy < y + q; iy++)
                {
                    int he_x0 = he_y0;
                    int he_x1 = he_y1;
                    int he_x2 = he_y2;
                    vec3 c_x = c_y;

                    for (int ix = x; ix < x + q; ix++)
                    {
                        if (he_x0 > 0 && he_x1 > 0 && he_x2 > 0)
                            buffer[ix] = to_rgb(c_x);

                        he_x0 -= fp8_dy01;
                        he_x1 -= fp8_dy12;
                        he_x2 -= fp8_dy20;
                        c_x -= dc_dy;
                    }

                    he_y0 += fp8_dx01;
                    he_y1 += fp8_dx12;
                    he_y2 += fp8_dx20;
                    c_y += dc_dx;

                    buffer += m_backbuffer_stride;
                }
            }
        }

        color_ptr += q * m_backbuffer_stride;
    }
}
#endif
