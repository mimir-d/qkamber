#pragma once

#include "renderer.h"
#include "misc.h"

struct RenderPrimitive;
struct RenderParams;

namespace detail
{
    // TODO: would like to use a lambda here, but no idea how atm
    struct make_mvp
    {
        mat4 operator()(const mat4& world, const mat4& view, const mat4& proj) const
        {
            return proj * view * world;
        }
    };
}

class Win32RenderDevice : public RenderDevice
{
public:
    Win32RenderDevice();
    ~Win32RenderDevice() = default;

    void win32_init(HWND window_handle);
    void win32_shutdown();

    // drawing methods
    void draw_primitive(const RenderPrimitive& primitive) override;
    void draw_text(const std::string& text, float x, float y) override;

    // state methods
    void set_world_matrix(mat4 world_matrix) override;
    void set_view_matrix(mat4 view_matrix) override;
    void set_proj_matrix(mat4 proj_matrix) override;
    void set_clip_matrix(mat3x4 clip_matrix) override;

    // framebuffer methods
    void clear() override;
    void swap_buffers() override;

private:
    void draw_line(float x0, float y0, float x1, float y1);
    void draw_tri(float x0, float y0, float x1, float y1, float x2, float y2);

private:
    mat4 m_world_matrix, m_view_matrix, m_proj_matrix;
    mat3x4 m_clip_matrix;

    // computed stuff
    dirty_t<mat4, detail::make_mvp> m_mvp_matrix;

    ULONG_PTR m_gdiplus_token;
    HWND m_window_handle;

    RECT m_rect;
    HBRUSH m_backbuffer_brush;
    HBITMAP m_backbuffer_bitmap;
    HDC m_frontbuffer, m_backbuffer;

    std::unique_ptr<Gdiplus::Graphics> m_graphics;
};

inline Win32RenderDevice::Win32RenderDevice() :
    m_mvp_matrix(m_world_matrix, m_view_matrix, m_proj_matrix)
{}

inline void Win32RenderDevice::set_world_matrix(mat4 world_matrix)
{
    m_world_matrix = world_matrix;
    m_mvp_matrix.set_dirty();
}

inline void Win32RenderDevice::set_view_matrix(mat4 view_matrix)
{
    m_view_matrix = view_matrix;
    m_mvp_matrix.set_dirty();
}

inline void Win32RenderDevice::set_proj_matrix(mat4 proj_matrix)
{
    m_proj_matrix = proj_matrix;
    m_mvp_matrix.set_dirty();
}

inline void Win32RenderDevice::set_clip_matrix(mat3x4 clip_matrix)
{
    m_clip_matrix = clip_matrix;
}
