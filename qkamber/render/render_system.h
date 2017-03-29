#pragma once

#include "subsystem.h"
#include "render_buffers.h"
#include "render_queue.h"
#include "render_cache.h"
#include "engine.h"

struct RenderPrimitive;
class Texture;

enum class PolygonMode
{
    Point, Line, Fill
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

    virtual void set_world_inv_matrix(mat4 world_inv_matrix) = 0;
    virtual void set_view_inv_matrix(mat4 view_inv_matrix) = 0;

    virtual void set_render_target(RenderTarget* target) = 0;
    virtual void set_polygon_mode(PolygonMode mode) = 0;

    virtual void set_material(const Material* material) = 0;

    // resource management methods
    virtual std::unique_ptr<RenderTarget> create_render_target(int width, int height) = 0;
    virtual std::unique_ptr<VertexBuffer> create_vertex_buffer(std::unique_ptr<VertexDecl> decl, size_t count) = 0;
    virtual std::unique_ptr<IndexBuffer> create_index_buffer(size_t count) = 0;
    virtual std::unique_ptr<Texture> create_texture(size_t width, size_t height, PixelFormat format) = 0;

    // framebuffer methods
    virtual void clear() = 0;
    virtual void swap_buffers() = 0;
};

class RenderSystem : public Subsystem
{
public:
	RenderSystem(QkEngine::Context& context);
	~RenderSystem();

    void process() final;

public:
    RenderDevice& get_device();
    RenderQueue& get_queue();
    RenderCache& get_cache();

protected:
    std::unique_ptr<RenderDevice> m_dev;
    RenderQueue m_queue;
    RenderCache m_cache;
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

inline RenderCache& RenderSystem::get_cache()
{
    return m_cache;
}
