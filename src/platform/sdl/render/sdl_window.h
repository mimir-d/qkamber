#pragma once

#include "engine.h"
#include "render/render_buffers.h"
#include "render/render_window.h"

class Win32SoftwareDevice;

class SdlColorBuffer : public ColorBuffer
{
public:
    SdlColorBuffer(int width, int height);
    ~SdlColorBuffer();

    SDL_Renderer* get_renderer();
    SDL_Surface* get_surface();

    void resize(int width, int height);

private:
    SDL_Renderer* m_renderer;
    SDL_Surface* m_surface;

    size_t m_width;
    size_t m_height;
};

class SdlDepthBuffer : public DepthBuffer
{
public:
    SdlDepthBuffer(int width, int height);
    ~SdlDepthBuffer() = default;

    float* get_data();
    size_t get_stride();

    void resize(int width, int height);

private:
    std::unique_ptr<float[]> m_data;
    size_t m_width = 0;
    size_t m_height = 0;
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

private:
    void create_window(int width, int height);

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    int m_width;
    int m_height;

    std::unique_ptr<SdlColorBuffer> m_color_buf;
    std::unique_ptr<SdlDepthBuffer> m_depth_buf;

    QkEngine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// SdlColorBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline SDL_Renderer* SdlColorBuffer::get_renderer()
{
    return m_renderer;
}

inline SDL_Surface* SdlColorBuffer::get_surface()
{
    return m_surface;
}

///////////////////////////////////////////////////////////////////////////////
// SdlDepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
inline float* SdlDepthBuffer::get_data()
{
    return m_data.get();
}

inline size_t SdlDepthBuffer::get_stride()
{
    return m_width;
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