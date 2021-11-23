
#include "precompiled.h"
#include "win32_software_device.h"

#include "win32_window.h"
#include "render/material.h"
#include "render/render_primitive.h"
#include "render/render_buffers.h"
#include "render/software_buffers.h"
#include "scene/light.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Win32SoftwareDevice
///////////////////////////////////////////////////////////////////////////////
Win32SoftwareDevice::Win32SoftwareDevice(QkEngine::Context& context) :
    m_context(context),
    m_font(reinterpret_cast<HFONT>(INVALID_HANDLE_VALUE))
{
    flog("id = %#x", this);

    // create drawing stuff
    m_clear_brush = CreateSolidBrush(RGB(0, 0, 0));
    m_fill_brush = CreateSolidBrush(0x009600c8);
    m_line_pen = CreatePen(PS_SOLID, 1, 0x009600c8);

    log_info("Created win32 software device");
}

Win32SoftwareDevice::~Win32SoftwareDevice()
{
    flog();

    DeleteObject(m_line_pen);
    DeleteObject(m_fill_brush);
    DeleteObject(m_clear_brush);
    DeleteObject(m_font);

    log_info("Destroyed win32 software device");
}

void Win32SoftwareDevice::set_render_target(RenderTarget* target)
{
    flog();

    SoftwareDevice::set_render_target(target);
    if (!target)
        return;

    // TODO: future, target might be a texture so extract just the DC
    auto window = static_cast<Win32Window*>(target);

    // create font
    LOGFONT font_desc = { 0 };

    font_desc.lfHeight = -MulDiv(9, GetDeviceCaps(window->get_dc(), LOGPIXELSY), 72);
    font_desc.lfQuality = ANTIALIASED_QUALITY;
    font_desc.lfPitchAndFamily = MONO_FONT;
    strcpy_s(font_desc.lfFaceName, LF_FACESIZE, "Courier New");

    m_font = CreateFontIndirect(&font_desc);

    // set up backbuffer objects
    auto& color_buf = static_cast<Win32ColorBuffer&>(target->get_color_buffer());
    HDC backbuffer = color_buf.get_dc();
    SelectObject(backbuffer, m_font);
    SelectObject(backbuffer, m_fill_brush);
    SelectObject(backbuffer, m_line_pen);
}

///////////////////////////////////////////////////////////////////////////////
// Drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_text(const std::string& text, int x, int y)
{
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    TextOut(color_buf.get_dc(), x, y, text.c_str(), static_cast<int>(text.size()));
}

///////////////////////////////////////////////////////////////////////////////
// Resource management methods
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<RenderTarget> Win32SoftwareDevice::create_render_target(int width, int height)
{
    flog();

    // TODO: or texture, etc..
    log_info("Creating win32 window render target...");
    return std::unique_ptr<RenderTarget>{ new Win32Window{ m_context, width, height } };
}

///////////////////////////////////////////////////////////////////////////////
// Frame methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::clear()
{
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    RECT rc = { 0, 0, m_render_target->get_width(), m_render_target->get_height() };
    FillRect(color_buf.get_dc(), &rc, m_clear_brush);

    // clear the zbuffer
    auto& depth_buf = static_cast<SoftwareDepthBuffer&>(m_render_target->get_depth_buffer());
    depth_buf.clear();
}

void Win32SoftwareDevice::swap_buffers()
{
    HDC surface = static_cast<Win32Window*>(m_render_target)->get_dc();
    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    BitBlt(
        surface,
        0, 0, m_render_target->get_width(), m_render_target->get_height(),
        color_buf.get_dc(),
        0, 0,
        SRCCOPY
    );
}

///////////////////////////////////////////////////////////////////////////////
// Low-level drawing methods
///////////////////////////////////////////////////////////////////////////////
void Win32SoftwareDevice::draw_points(const std::vector<DevicePoint>& points)
{
    const int r = 3;

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    HDC dc = color_buf.get_dc();

    for (const auto& p : points)
    {
        const auto& pos = p.position;
        const int x = static_cast<int>(pos.x()), y = static_cast<int>(pos.y());
        Ellipse(dc, x - r, y - r, x + r, y + r);
    }
}

void Win32SoftwareDevice::draw_lines(const std::vector<DevicePoint>& points)
{
    if (points.empty())
        return;

    auto& color_buf = static_cast<Win32ColorBuffer&>(m_render_target->get_color_buffer());
    HDC dc = color_buf.get_dc();

    const int x0 = static_cast<int>(points[0].position.x());
    const int y0 = static_cast<int>(points[0].position.y());
    MoveToEx(dc, x0, y0, nullptr);

    for (size_t i = 1; i < points.size(); i++)
    {
        const auto& pos = points[i].position;
        const int x = static_cast<int>(pos.x()), y = static_cast<int>(pos.y());
        LineTo(dc, x, y);
    }
}
