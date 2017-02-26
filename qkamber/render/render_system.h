#pragma once

#include "engine.h"
#include "render_buffers.h"
#include "render_queue.h"
#include "time/time_system.h"
#include "scene/camera.h"
#include "scene/viewport.h"

struct RenderPrimitive;

enum class PolygonMode
{
    Point, Line, Fill
};

class RenderTarget
{
public:
    virtual ~RenderTarget() = default;

    // NOTE: this always has these 2 attachments (should provide more in the future)
    virtual ColorBuffer& get_color_buffer() = 0;
    virtual DepthBuffer& get_depth_buffer() = 0;

    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
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
    virtual std::unique_ptr<RenderTarget> create_render_target(int width, int height) = 0;
    virtual std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) = 0;
    virtual std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) = 0;

    // framebuffer methods
    virtual void clear() = 0;
    virtual void swap_buffers() = 0;
};

class RenderSystem
{
public:
	RenderSystem(QkEngine::Context& context);
	~RenderSystem();

    void begin_frame();
    void render();
    // TODO: temp until some gui happens
    void render_text(const std::string& text, int x, int y);
    void end_frame();

    RenderDevice& get_device();
    RenderQueue& get_queue();

    void set_camera(Camera* camera);
    void set_viewport(Viewport* viewport);

protected:
    std::unique_ptr<RenderDevice> m_dev;
    RenderQueue m_queue;

    //TEMP:
    Camera* m_camera = nullptr;
    Viewport* m_viewport = nullptr;

    QkEngine::Context& m_context;
};

///////////////////////////////////////////////////////////////////////////////
// Renderer impl
///////////////////////////////////////////////////////////////////////////////
inline RenderDevice& RenderSystem::get_device()
{
    return *m_dev;
}

inline RenderQueue& RenderSystem::get_queue()
{
    return m_queue;
}

inline void RenderSystem::set_camera(Camera* camera)
{
    m_camera = camera;
}

inline void RenderSystem::set_viewport(Viewport* viewport)
{
    m_viewport = viewport;
}
