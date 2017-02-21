#pragma once

#include "software/software_device.h"
#include "renderer.h"
#include "render_buffers.h"
#include "misc.h"

class Win32ColorBuffer : public ColorBuffer
{
public:
    Win32ColorBuffer(HDC surface);
    ~Win32ColorBuffer() = default;

    HDC get_dc();

    DWORD* get_data();
    size_t get_stride();

    void resize(int width, int height);

private:
    HDC m_surface_dc;

    HDC m_dc;
    HBITMAP m_bitmap = reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE);

    DWORD* m_data_ptr;
    size_t m_stride;
};

class Win32DepthBuffer : public DepthBuffer
{
public:
    float* get_data();
    size_t get_stride();

    void resize(int width, int height);

private:
    std::unique_ptr<float[]> m_data;
    size_t m_stride;
};

class Win32SoftwareDevice : public SoftwareDevice
{
public:
    void win32_init(HWND window_handle);
    void win32_shutdown();
    void win32_resize(PRECT client_rect);

    // drawing methods
    void draw_text(const std::string& text, int x, int y) final;

    // resource management methods
    std::unique_ptr<RenderTarget> create_render_target() final;

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
    HDC m_surface;

    HBRUSH m_clear_brush;
    std::unique_ptr<float[]> m_zbuffer_clear;

    HFONT m_font;
    HPEN m_line_pen;
    HBRUSH m_fill_brush;

    HWND m_window_handle;
    std::unique_ptr<RenderTarget> m_default_target;
};

///////////////////////////////////////////////////////////////////////////////
// Win32RenderTarget impl
///////////////////////////////////////////////////////////////////////////////
inline HDC Win32ColorBuffer::get_dc()
{
    return m_dc;
}

inline DWORD* Win32ColorBuffer::get_data()
{
    return m_data_ptr;
}

inline size_t Win32ColorBuffer::get_stride()
{
    return m_stride;
}

///////////////////////////////////////////////////////////////////////////////
// Win32DepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline float* Win32DepthBuffer::get_data()
{
    return m_data.get();
}

inline size_t Win32DepthBuffer::get_stride()
{
    return m_stride;
}
