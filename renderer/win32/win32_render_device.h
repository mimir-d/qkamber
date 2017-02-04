#pragma once

#include "software/software_device.h"
#include "renderer.h"
#include "misc.h"

class Win32RenderDevice : public SoftwareDevice
{
public:
    void win32_init(HWND window_handle);
    void win32_shutdown();

    // drawing methods
    void draw_text(const std::string& text, float x, float y) override;

    // framebuffer methods
    void clear() override;
    void swap_buffers() override;

protected:
    void draw_line(float x0, float y0, float x1, float y1) override;
    void draw_tri(float x0, float y0, float x1, float y1, float x2, float y2) override;

private:
    ULONG_PTR m_gdiplus_token;
    HWND m_window_handle;

    RECT m_rect;
    HBRUSH m_backbuffer_brush;
    HBITMAP m_backbuffer_bitmap;
    HDC m_frontbuffer, m_backbuffer;

    std::unique_ptr<Gdiplus::Graphics> m_graphics;
};

// TODO: rename this to win32_software_device