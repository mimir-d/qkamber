#pragma once

#include "engine.h"
#include "render_buffers.h"
#include "render_window.h"

class Win32SoftwareDevice;

class Win32ColorBuffer : public ColorBuffer
{
public:
    Win32ColorBuffer(HDC surface, int width, int height);
    ~Win32ColorBuffer();

    HDC get_dc();

    DWORD* get_data();
    size_t get_stride();

    // TODO: should these actually be recreated instead of resized?
    void resize(int width, int height);

private:
    HDC m_surface_dc;

    HDC m_dc;
    HBITMAP m_bitmap = reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE);

    DWORD* m_data_ptr;
    size_t m_width = 0;
    size_t m_height = 0;
    size_t m_stride;
};

class Win32DepthBuffer : public DepthBuffer
{
public:
    Win32DepthBuffer(int width, int height);
    ~Win32DepthBuffer() = default;

    float* get_data();
    size_t get_stride();

    void resize(int width, int height);

    // TODO: should this optimization be here or somewhere else?
    float* get_data_clear();

private:
    std::unique_ptr<float[]> m_data;
    std::unique_ptr<float[]> m_data_clear;
    size_t m_width = 0;
    size_t m_height = 0;
};

class Win32Window : public RenderWindow
{
public:
    Win32Window(QkEngine::Context& context, int width, int height);
    ~Win32Window();

    HDC get_dc() const;

    int get_width() const final;
    int get_height() const final;

    ColorBuffer& get_color_buffer() final;
    DepthBuffer& get_depth_buffer() final;

private:
    void register_class();
    void unregister_class();
    void create_window(int width, int height);
    void register_inputs();

    void pause_timer(bool enable);

    bool on_input(HRAWINPUT raw_handle);
    void on_resize(RECT& rc);
    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    HDC m_dc = static_cast<HDC>(INVALID_HANDLE_VALUE);
    HWND m_window_handle = static_cast<HWND>(INVALID_HANDLE_VALUE);
    RECT m_rect = { 0 };

    enum class WindowState : int
    {
        Unknown = -1,
        Normal = SIZE_RESTORED,
        Sizing = SIZE_MAXHIDE + 1000
    };
    WindowState m_window_state = WindowState::Unknown;

    std::unique_ptr<Win32ColorBuffer> m_color_buf;
    std::unique_ptr<Win32DepthBuffer> m_depth_buf;

    QkEngine::Context& m_context;
    bool m_discard_input = false;
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
    return m_width;
}

inline float* Win32DepthBuffer::get_data_clear()
{
    return m_data_clear.get();
}

///////////////////////////////////////////////////////////////////////////////
// Win32Window impl
///////////////////////////////////////////////////////////////////////////////
inline HDC Win32Window::get_dc() const
{
    return m_dc;
}

inline int Win32Window::get_width() const
{
    return m_rect.right - m_rect.left;
}

inline int Win32Window::get_height() const
{
    return m_rect.bottom - m_rect.top;
}

inline ColorBuffer& Win32Window::get_color_buffer()
{
    return *m_color_buf.get();
}

inline DepthBuffer& Win32Window::get_depth_buffer()
{
    return *m_depth_buf.get();
}