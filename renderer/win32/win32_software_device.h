#pragma once

#include "software/software_device.h"
#include "renderer.h"
#include "misc.h"

class Win32SoftwareDevice : public SoftwareDevice
{
public:
    void win32_init(HWND window_handle);
    void win32_shutdown();
    void win32_resize(PRECT client_rect);

    // drawing methods
    void draw_text(const std::string& text, float x, float y) final;

    // framebuffer methods
    void clear() final;
    void swap_buffers() final;

protected:
    void draw_tri_point(float x0, float y0, float x1, float y1, float x2, float y2) final;
    void draw_tri_line(float x0, float y0, float x1, float y1, float x2, float y2) final;
    void draw_tri_fill(float x0, float y0, float x1, float y1, float x2, float y2) final;

private:
    RECT m_rect;
    HDC m_frontbuffer;

    HBRUSH m_backbuffer_brush;
    HDC m_backbuffer = static_cast<HDC>(INVALID_HANDLE_VALUE);
    HBITMAP m_backbuffer_bitmap = static_cast<HBITMAP>(INVALID_HANDLE_VALUE);

    ULONG_PTR m_gdiplus_token;
    HWND m_window_handle;

    std::unique_ptr<Gdiplus::Graphics> m_graphics;
};
