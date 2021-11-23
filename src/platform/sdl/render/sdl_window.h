#pragma once

#include "engine.h"
#include "render/software_buffers.h"
#include "render/render_buffers.h"
#include "render/render_window.h"

class Win32SoftwareDevice;

class SdlColorBuffer : public ColorBuffer
{
public:
    SdlColorBuffer(int width, int height);
    ~SdlColorBuffer();

    uint32_t* lock() final;
    void unlock() final;
    size_t get_stride() final;

    SDL_Renderer* get_renderer();
    SDL_Surface* get_surface();

    void resize(int width, int height);

private:
    SDL_Renderer* m_renderer;
    SDL_Surface* m_surface;

    size_t m_width;
    size_t m_height;
};

class SdlWindow : public RenderWindow
{
public:
    SdlWindow(QkEngine::Context& context, int width, int height);
    ~SdlWindow();

    SDL_Renderer* get_renderer() const;

    int get_width() const final;
    int get_height() const final;

    ColorBuffer& get_color_buffer() final;
    DepthBuffer& get_depth_buffer() final;

    void resize(int width, int height);

private:
    void create_window(int width, int height);

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    int m_width;
    int m_height;

    std::unique_ptr<SdlColorBuffer> m_color_buf;
    std::unique_ptr<SoftwareDepthBuffer> m_depth_buf;

    QkEngine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// SdlColorBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline size_t SdlColorBuffer::get_stride()
{
    return m_surface->pitch / 4;
}

inline SDL_Renderer* SdlColorBuffer::get_renderer()
{
    return m_renderer;
}

inline SDL_Surface* SdlColorBuffer::get_surface()
{
    return m_surface;
}

///////////////////////////////////////////////////////////////////////////////
// SdlWindow impl
///////////////////////////////////////////////////////////////////////////////
inline SDL_Renderer* SdlWindow::get_renderer() const
{
    return m_renderer;
}

inline int SdlWindow::get_width() const
{
    return m_width;
}

inline int SdlWindow::get_height() const
{
    return m_height;
}

inline ColorBuffer& SdlWindow::get_color_buffer()
{
    return *m_color_buf.get();
}

inline DepthBuffer& SdlWindow::get_depth_buffer()
{
    return *m_depth_buf.get();
}
