#pragma once

#include "software/software_device.h"
#include "renderer.h"
#include "misc.h"

class Win32RenderDevice : public SoftwareDevice
{
public:
    Win32RenderDevice();
    ~Win32RenderDevice() = default;

    void win32_init(HWND window_handle);
    void win32_shutdown();
    void win32_resize(PRECT client_rect);

    // drawing methods
    void draw_text(const std::string& text, float x, float y) override;

    // framebuffer methods
    void clear() override;
    void swap_buffers() override;

protected:
    void draw_tri_wireframe(float x0, float y0, float x1, float y1, float x2, float y2) override;
    void draw_tri(float x0, float y0, float x1, float y1, float x2, float y2) override;

private:
    RECT m_rect;
    HDC m_frontbuffer;

    HBRUSH m_backbuffer_brush;
    HDC m_backbuffer;
    HBITMAP m_backbuffer_bitmap;

    ULONG_PTR m_gdiplus_token;
    HWND m_window_handle;

    std::unique_ptr<Gdiplus::Graphics> m_graphics;
};

// TODO: rename this to win32_software_device