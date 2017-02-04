#pragma once

#include "render_queue.h"
#include "timer.h"
#include "camera.h"
#include "viewport.h"

struct RenderPrimitive;
class VertexDecl;
class VertexBuffer;
class IndexBuffer;

class RenderDevice
{
public:
    // drawing methods
    virtual void draw_primitive(const RenderPrimitive& primitive) = 0;
    virtual void draw_text(const std::string& text, float x, float y) = 0;

    // device state methods
    virtual void set_world_matrix(mat4 world_matrix) = 0;
    virtual void set_view_matrix(mat4 view_matrix) = 0;
    virtual void set_proj_matrix(mat4 proj_matrix) = 0;
    virtual void set_clip_matrix(mat3x4 clip_matrix) = 0;

    // resource management methods
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

    void render();

    RenderDevice& get_device();
    RenderQueue& get_queue();

    float get_fps() const;

private:
    void begin_frame();
    void end_frame();

protected:
    std::unique_ptr<RenderDevice> m_dev;
    RenderQueue m_queue;

    //TEMP:
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Viewport> m_viewport;

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

inline float Renderer::get_fps() const
{
    return m_fps;
}