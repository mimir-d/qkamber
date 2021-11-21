
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
void SdlSoftwareDevice::draw_tri(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    switch (m_poly_mode)
    {
        case PolygonMode::Point: draw_tri_point(p0, p1, p2); break;
        case PolygonMode::Line:  draw_tri_line(p0, p1, p2);  break;
        case PolygonMode::Fill:  draw_tri_fill(p0, p1, p2);  break;
    }
}

void SdlSoftwareDevice::draw_line(const DevicePoint& p0, const DevicePoint& p1)
{
    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLineF(
        renderer,
        p0.position.x(), p0.position.y(),
        p1.position.x(), p1.position.y()
    );
}

void SdlSoftwareDevice::draw_tri_point(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    SDL_FPoint points[] = {
        { p0.position.x(), p0.position.y() },
        { p1.position.x(), p1.position.y() },
        { p2.position.x(), p2.position.y() }
    };

    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPointsF(renderer, points, 3);
}

void SdlSoftwareDevice::draw_tri_line(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
    SDL_FPoint points[] = {
        { p0.position.x(), p0.position.y() },
        { p1.position.x(), p1.position.y() },
        { p2.position.x(), p2.position.y() },
        { p0.position.x(), p0.position.y() }
    };

    // for (int i =0; i <4;i++) {
    //     log_info("p%d: %f %f", i, points[i].x, points[i].y);
    // }

    auto& color_buf = static_cast<SdlColorBuffer&>(m_render_target->get_color_buffer());
    SDL_Renderer* renderer = color_buf.get_renderer();

    SDL_SetRenderDrawColor(renderer, 150, 0, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, points, 4);
}

void SdlSoftwareDevice::draw_tri_fill(const DevicePoint& p0, const DevicePoint& p1, const DevicePoint& p2)
{
}
