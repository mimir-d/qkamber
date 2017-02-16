
#include "stdafx.h"
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
    m_graphics->DrawString(wcbuf.get(), text.size(), &font, p, &hb);
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

inline float half_space(float x0, float y0, float x1, float y1, float x2, float y2)
{
    return (x0 - x1) * (y2 - y0) - (y0 - y1) * (x2 - x0);
}

void Win32SoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    const float x0 = p0.position.x();
    const float x1 = p1.position.x();
    const float x2 = p2.position.x();

    const float y0 = p0.position.y();
    const float y1 = p1.position.y();
    const float y2 = p2.position.y();

    // min bounding box
    int min_x = static_cast<int>(::min(x0, x1, x2));
    int max_x = static_cast<int>(::max(x0, x1, x2));
    int min_y = static_cast<int>(::min(y0, y1, y2));
    int max_y = static_cast<int>(::max(y0, y1, y2));

    DWORD* color_ptr = m_backbuffer_bits + min_y * m_backbuffer_stride;

    for (int y = min_y; y <= max_y; y++)
    {
        for (int x = min_x; x <= max_x; x++)
        {
            if (half_space(x0, y0, x1, y1, static_cast<float>(x), static_cast<float>(y)) > 0 &&
                half_space(x1, y1, x2, y2, static_cast<float>(x), static_cast<float>(y)) > 0 &&
                half_space(x2, y2, x0, y0, static_cast<float>(x), static_cast<float>(y)) > 0
            ) {
                color_ptr[x] = 0x00FF0000;
            }
        }
        color_ptr += m_backbuffer_stride;
    }
}