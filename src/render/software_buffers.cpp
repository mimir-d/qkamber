
#include "precompiled.h"
#include "software_buffers.h"

///////////////////////////////////////////////////////////////////////////////
// SoftwareDepthBuffer impl
///////////////////////////////////////////////////////////////////////////////
SoftwareDepthBuffer::SoftwareDepthBuffer(int width, int height) :
    BufferStorage(width * height)
{
    resize(width, height);
    log_info("Created software depth buffer");
}

void SoftwareDepthBuffer::resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    // update dimensions
    m_width = width;
    m_height = height;

    m_data.reset(new float[height * width]);
}

void SoftwareDepthBuffer::clear()
{
    float* data = lock();
    std::fill(data, data + m_width * m_height, std::numeric_limits<float>::max());
    unlock();
}

///////////////////////////////////////////////////////////////////////////////
// SoftwareTexture impl
///////////////////////////////////////////////////////////////////////////////
SoftwareTexture::SoftwareTexture(size_t width, size_t height, PixelFormat format) :
    BufferStorage(width * height * 4),
    m_width(width),
    m_height(height),
    m_format(format)
{}

uint8_t* SoftwareTexture::lock()
{
    if (m_format == PixelFormat::RgbaU8)
        return BufferStorage::lock();

    // NOTE: simplification for rasterizer
    m_rgb8u_data.resize(m_width * m_height * 3);
    return m_rgb8u_data.data();
}

void SoftwareTexture::unlock()
{
    // TODO: gamma correction, texture degamma + lighting gamma
    // NOTE: simplification for rasterizer, convert to rgba
    if (m_format == PixelFormat::RgbU8)
    {
        uint8_t* pi = m_rgb8u_data.data();
        uint8_t* po = m_data.get();
        for (; pi != m_rgb8u_data.data() + m_height * m_width * 3; pi += 3, po += 4)
        {
            po[0] = pi[0];
            po[1] = pi[1];
            po[2] = pi[2];
            po[3] = 0xff;
        }

        m_rgb8u_data.clear();
    }
}
