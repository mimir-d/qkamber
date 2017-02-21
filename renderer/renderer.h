#pragma once

#include "render_buffers.h"
#include "render_queue.h"
#include "timer.h"
#include "camera.h"
#include "viewport.h"

struct RenderPrimitive;

enum class PolygonMode
{
    Point, Line, Fill
};

// TODO: window should be a rendertarget-type
class RenderTarget
{
public:
    RenderTarget(
        int width, int height,
        std::unique_ptr<ColorBuffer> color,
        std::unique_ptr<DepthBuffer> depth
    );
    ~RenderTarget() = default;

    // TODO: this always has these 2 attachments (should provide more in the future)
    ColorBuffer& get_color_buffer();
    DepthBuffer& get_depth_buffer();

    void set_width(int width)
    {
        m_width = width;
    }
    void set_height(int height)
    {
        m_height = height;
    }
    int get_width() const;
    int get_height() const;

private:
    std::unique_ptr<ColorBuffer> m_color_buffer;
    std::unique_ptr<DepthBuffer> m_depth_buffer;

    int m_width, m_height;
};

class RenderDevice
{
public:
    // drawing methods
    virtual void draw_primitive(const RenderPrimitive& primitive) = 0;
    virtual void draw_text(const std::string& text, int x, int y) = 0;

    // device state methods
    virtual void set_world_matrix(mat4 world_matrix) = 0;
    virtual void set_view_matrix(mat4 view_matrix) = 0;
    virtual void set_proj_matrix(mat4 proj_matrix) = 0;
    virtual void set_clip_matrix(mat3x4 clip_matrix) = 0;

    virtual void set_render_target(RenderTarget* target) = 0;
    virtual void set_polygon_mode(PolygonMode mode) = 0;

    // resource management methods
    virtual std::unique_ptr<RenderTarget> create_render_target() = 0;
    virtual std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) = 0;
    virtual std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) = 0;

    // framebuffer methods
    virtual void clear() = 0;
    virtual void swap_buffers() = 0;
};

class Renderer
{
public:
	void init(Timer* timer);
	void shutdown();

    void begin_frame();
    void render();
    // TODO: temp until some gui happens
    void render_text(const std::string& text, int x, int y);
    void end_frame();

    RenderDevice& get_device();
    RenderQueue& get_queue();

    void set_camera(Camera* camera);
    void set_viewport(Viewport* viewport);

    float get_fps() const;

protected:
    std::unique_ptr<RenderDevice> m_dev;
    RenderQueue m_queue;

    //TEMP:
    Camera* m_camera;
    Viewport* m_viewport;

	Timer* m_timer;
    uint64_t m_frame_number;

    float m_target_fps, m_fps;
    uint32_t m_fps_last_count;
    float m_fps_last_timestamp;
};

class RenderDeviceFactory
{
public:
    static std::unique_ptr<RenderDevice> create();
};

///////////////////////////////////////////////////////////////////////////////
// RenderTarget impl
///////////////////////////////////////////////////////////////////////////////
inline RenderTarget::RenderTarget(
    int width, int height,
    std::unique_ptr<ColorBuffer> color,
    std::unique_ptr<DepthBuffer> depth
) :
    m_width(width), m_height(height),
    m_color_buffer(std::move(color)),
    m_depth_buffer(std::move(depth))
{}

inline ColorBuffer& RenderTarget::get_color_buffer()
{
    return *m_color_buffer.get();
}

inline DepthBuffer& RenderTarget::get_depth_buffer()
{
    return *m_depth_buffer.get();
}

inline int RenderTarget::get_width() const
{
    return m_width;
}

inline int RenderTarget::get_height() const
{
    return m_height;
}

///////////////////////////////////////////////////////////////////////////////
// Renderer impl
///////////////////////////////////////////////////////////////////////////////
inline RenderDevice& Renderer::get_device()
{
    return *m_dev;
}

inline RenderQueue& Renderer::get_queue()
{
    return m_queue;
}

inline void Renderer::set_camera(Camera* camera)
{
    m_camera = camera;
}

inline void Renderer::set_viewport(Viewport* viewport)
{
    m_viewport = viewport;
}

inline float Renderer::get_fps() const
{
    return m_fps;
}