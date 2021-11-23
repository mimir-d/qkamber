#pragma once

#include "render/software_device.h"
#include "render/software_buffers.h"
#include "engine.h"

class Win32SoftwareDevice : public SoftwareDevice
{
public:
    Win32SoftwareDevice(QkEngine::Context& context);
    ~Win32SoftwareDevice();

    void set_render_target(RenderTarget* target) final;

    // drawing methods
    void draw_text(const std::string& text, int x, int y) final;

    // resource management methods
    std::unique_ptr<RenderTarget> create_render_target(int width, int height) final;

    // framebuffer methods
    void clear() final;
    void swap_buffers() final;

protected:
    void draw_points(const std::vector<DevicePoint>& points) final;
    void draw_lines(const std::vector<DevicePoint>& points) final;

private:
    QkEngine::Context& m_context;

    HBRUSH m_clear_brush;
    HPEN m_line_pen;
    HBRUSH m_fill_brush;
    HFONT m_font;
};
