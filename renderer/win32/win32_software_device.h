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
    void draw_text(const std::string& text, int x, int y) final;

    // framebuffer methods
    void clear() final;
    void swap_buffers() final;

protected:
    void draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2) final;

private:
    void draw_tri_point(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2);
    void draw_tri_line(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2);
    void draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2);

private:
    RECT m_rect;
    HDC m_frontbuffer;

    HBRUSH m_backbuffer_brush;
    HDC m_backbuffer = static_cast<HDC>(INVALID_HANDLE_VALUE);

    HBITMAP m_backbuffer_bitmap = static_cast<HBITMAP>(INVALID_HANDLE_VALUE);
    // TODO: extract to a RenderTarget
    size_t m_backbuffer_stride;
    uint8_t* m_backbuffer_bits;
    uint8_t* m_zbuffer;
    uint8_t* m_zbuffer_clear;

    HFONT m_font;
    HPEN m_line_pen;
    HBRUSH m_fill_brush;

    HWND m_window_handle;
};
