
#include "precompiled.h"
#include "sdl_software_device.h"

#include "sdl_window.h"
#include "render/material.h"
#include "render/render_primitive.h"
#include "render/render_buffers.h"
#include "render/software_buffers.h"
#include "scene/light.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// SdlSoftwareDevice
///////////////////////////////////////////////////////////////////////////////
SdlSoftwareDevice::SdlSoftwareDevice(QkEngine::Context& context) :
    m_context(context)
{
    flog("id = %#x", this);

    // create drawing stuff

    log_info("Created SDL software device");
}

SdlSoftwareDevice::~SdlSoftwareDevice()
{
    flog();

    log_info("Destroyed SDL software device");
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void SdlSoftwareDevice::draw_text(const std::string& text, int x, int y)
{
    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    // TODO: SDL_ttf
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<RenderTarget> SdlSoftwareDevice::create_render_target(int width, int height)
{
    flog();

    // TODO: or texture, etc..
    log_info("Creating SDL window render target...");
    return std::unique_ptr<RenderTarget>{ new SdlWindow{ m_context, width, height } };
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void SdlSoftwareDevice::clear()
{
    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // clear the zbuffer
    auto& depth_buf = static_cast<SoftwareDepthBuffer&>(m_render_target->get_depth_buffer());
    depth_buf.clear();
}

void SdlSoftwareDevice::swap_buffers()
{
    SDL_Renderer* renderer = static_cast<SdlWindow*>(m_render_target)->get_renderer();
    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());

    // shift all pixels to the gpu
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, color_buf.get_surface());
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

///////////////////////////////////////////////////////////////////////////////
// Low-level drawing methods
///////////////////////////////////////////////////////////////////////////////
void SdlSoftwareDevice::draw_points(const std::vector<DevicePoint>& points)
{
    std::vector<SDL_FPoint> sdl_points;
    sdl_points.reserve(points.size());

    std::transform(
        points.begin(), points.end(),
        std::back_inserter(sdl_points),
        [](const auto& p) { return SDL_FPoint{ p.position.x(), p.position.y() }; }
    );

    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPointsF(renderer, sdl_points.data(), sdl_points.size());
}

void SdlSoftwareDevice::draw_lines(const std::vector<DevicePoint>& points)
{
    std::vector<SDL_FPoint> sdl_points;
    sdl_points.reserve(points.size());

    std::transform(
        points.begin(), points.end(),
        std::back_inserter(sdl_points),
        [](const auto& p) { return SDL_FPoint{ p.position.x(), p.position.y() }; }
    );

    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, sdl_points.data(), sdl_points.size());
}
