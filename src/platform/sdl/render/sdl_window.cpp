
#include "precompiled.h"
#include "sdl_window.h"

#include "time/time_system.h"
#include "input/input_system.h"
#include "platform/sdl/sdl_app.h"
#include "platform/sdl/render/sdl_software_device.h"
#include "platform/sdl/input/sdl_input_device.h"

using namespace std;

constexpr char WINDOW_TITLE[] = "Qkamber";

///////////////////////////////////////////////////////////////////////////////
// SdlColorBuffer impl
///////////////////////////////////////////////////////////////////////////////
SdlColorBuffer::SdlColorBuffer(int width, int height) :
    ColorBuffer(ColorBufferFormat::ARGB8),
    m_surface(nullptr),
    m_renderer(nullptr)
{
    flog("id = %#x", this);

    resize(width, height);
    log_info("Created SDL color buffer");
}

SdlColorBuffer::~SdlColorBuffer()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_FreeSurface(m_surface);
}

uint32_t* SdlColorBuffer::lock()
{
    SDL_LockSurface(m_surface);
    return static_cast<uint32_t*>(m_surface->pixels);
}

void SdlColorBuffer::unlock()
{
    SDL_UnlockSurface(m_surface);
}

void SdlColorBuffer::resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    // update dimensions
    m_width = width;
    m_height = height;

    // there's no resize; just recreate
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_FreeSurface(m_surface);
    }

    m_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    m_renderer = SDL_CreateSoftwareRenderer(m_surface);
}

///////////////////////////////////////////////////////////////////////////////
// SdlWindow impl
///////////////////////////////////////////////////////////////////////////////
SdlWindow::SdlWindow(QkEngine::Context& context, int width, int height) :
    m_context(context),
    m_width(width),
    m_height(height)
{
    flog("id = %#x", this);

    create_window(width, height);

    log_info("Finished creating SDL window target %#x", this);
}

SdlWindow::~SdlWindow()
{
    flog();

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    log_info("Destroyed SDL window");
}

void SdlWindow::create_window(int width, int height)
{
    flog();
    m_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window)
        throw std::runtime_error("SDL_CreateWindow failed");
    log_info("Created window = %#x, title = %s", m_window, WINDOW_TITLE);

    SDL_SetWindowResizable(m_window, SDL_TRUE);

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer)
        throw std::runtime_error("SDL_CreateRenderer failed");

    // create render buffer objects
    m_color_buf = std::make_unique<SdlColorBuffer>(width, height);
    m_depth_buf = std::make_unique<SoftwareDepthBuffer>(width, height);

    // TODO: until sdl window events are in place
    m_context.on_resize(width, height);
}
