
#include "precompiled.h"
#include "software_buffers.h"

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
        uint8_t* po = m_data.data();
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
