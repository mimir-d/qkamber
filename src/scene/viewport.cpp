
#include "precompiled.h"
#include "viewport.h"

void RectViewport::set_params(int width, int height)
{
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);

    // NOTE: the window has reversed y coordinate
    m_clip = mat4::clip(0, h, w, -h, 0.0f, 1.0f);
}
