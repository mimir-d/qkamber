
#include "stdafx.h"
#include "win32_software_device.h"

#include "render_primitive.h"
#include "render_buffers.h"

using namespace std;
using namespace Gdiplus;

Win32RenderDevice::Win32RenderDevice() :
    m_backbuffer(static_cast<HDC>(INVALID_HANDLE_VALUE)),
    m_backbuffer_bitmap(static_cast<HBITMAP>(INVALID_HANDLE_VALUE))
{}

void Win32RenderDevice::win32_init(HWND window_handle)
{
    flog();
    m_window_handle = window_handle;

    GdiplusStartupInput gdip_startup;
    GdiplusStartup(&m_gdiplus_token, &gdip_startup, nullptr);

    m_frontbuffer = GetDC(m_window_handle);
    m_backbuffer_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    RECT rc;
    GetClientRect(m_window_handle, &rc);
    win32_resize(&rc);
}

void Win32RenderDevice::win32_shutdown()
{
    flog();
    // delete backbuffer
    DeleteObject(m_backbuffer_brush);

    DeleteObject(m_backbuffer_bitmap);
    DeleteDC(m_backbuffer);
    ReleaseDC(m_window_handle, m_frontbuffer);

    GdiplusShutdown(m_gdiplus_token);
}

void Win32RenderDevice::win32_resize(PRECT client_rect)
{
    if (m_backbuffer_bitmap != INVALID_HANDLE_VALUE)
    {
        DeleteObject(m_backbuffer_bitmap);
        DeleteDC(m_backbuffer);
    }

    CopyRect(&m_rect, client_rect);
    GetClientRect(m_window_handle, &m_rect);
    m_backbuffer = CreateCompatibleDC(m_frontbuffer);
    m_backbuffer_bitmap = CreateCompatibleBitmap(
        m_frontbuffer,
        client_rect->right - client_rect->left,
        client_rect->bottom - client_rect->top
    );

    SelectObject(m_backbuffer, m_backbuffer_bitmap);
    m_graphics = unique_ptr<Graphics>(new Graphics(m_backbuffer));
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32RenderDevice::draw_text(const std::string& text, float x, float y)
{
    Font font(L"TimesNewRoman", 12);
    SolidBrush hb(Color(150, 0, 200));

    unique_ptr<wchar_t[]> wcbuf(new wchar_t[text.size() + 1]);
    mbstowcs_s(nullptr, wcbuf.get(), text.size(), text.c_str(), text.size());
    m_graphics->DrawString(wcbuf.get(), text.size(), &font, PointF(3, 3), &hb);
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void Win32RenderDevice::clear()
{
    RECT rc;
    GetClientRect(m_window_handle, &rc);
    FillRect(m_backbuffer, &rc, m_backbuffer_brush);
}

void Win32RenderDevice::swap_buffers()
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
// Helper methods
///////////////////////////////////////////////////////////////////////////////
void Win32RenderDevice::draw_tri_wireframe(float x0, float y0, float x1, float y1, float x2, float y2)
{
    const Pen p(Color(255, 150, 0, 255));
    const PointF points[] =
    {
        {x0, y0}, {x1, y1},
        {x0, y0}, {x2, y2},
        {x1, y1}, {x2, y2}
    };
    const size_t count = sizeof(points) / sizeof(points[0]);

    m_graphics->DrawPolygon(&p, points, count);
}

void Win32RenderDevice::draw_tri(float x0, float y0, float x1, float y1, float x2, float y2)
{
    const SolidBrush hb(Color(150, 0, 200));
    const PointF points[] = {
        { x0, y0 },
        { x1, y1 },
        { x2, y2 }
    };
    const size_t count = sizeof(points) / sizeof(points[0]);

    m_graphics->FillPolygon(&hb, points, count);
}