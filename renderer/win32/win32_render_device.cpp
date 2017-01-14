
#include "stdafx.h"
#include "win32_render_device.h"
using namespace std;
using namespace Gdiplus;

void Win32RenderDevice::win32_init(HWND window_handle)
{
    flog();
    m_window_handle = window_handle;

    GdiplusStartupInput gdip_startup;
    GdiplusStartup(&m_gdiplus_token, &gdip_startup, nullptr);

    GetClientRect(m_window_handle, &m_rect);

    m_frontbuffer = GetDC(m_window_handle);
    m_backbuffer = CreateCompatibleDC(m_frontbuffer);
    m_backbuffer_bitmap = CreateCompatibleBitmap(
        m_frontbuffer,
        m_rect.right - m_rect.left,
        m_rect.bottom - m_rect.top
    );

    SelectObject(m_backbuffer, m_backbuffer_bitmap);

    m_backbuffer_brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    m_graphics = unique_ptr<Graphics>(new Graphics(m_backbuffer));
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

void Win32RenderDevice::draw_line(float x0, float y0, float x1, float y1)
{
    Pen p(Color(255, 150, 0, 255));
    m_graphics->DrawLine(&p, x0, y0, x1, y1);
}

void Win32RenderDevice::draw_tri(float x0, float y0, float x1, float y1, float x2, float y2)
{
    Point vertices[] = {
        Point((INT)x0, (INT)y0), 
        Point((INT)x1, (INT)y1),
        Point((INT)x2, (INT)y2)
    };

    SolidBrush hb(Color(150, 0, 200));
    m_graphics->FillPolygon(&hb, vertices, 3);
}

void Win32RenderDevice::draw_text(const std::string& text, float x, float y)
{
    Font font(L"TimesNewRoman", 12);
    SolidBrush hb(Color(150, 0, 200));
    
    unique_ptr<wchar_t[]> wcbuf(new wchar_t[text.size() + 1]);
    mbstowcs_s(nullptr, wcbuf.get(), text.size(), text.c_str(), text.size());
    m_graphics->DrawString(wcbuf.get(), text.size(), &font, PointF(3, 3), &hb);
}

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