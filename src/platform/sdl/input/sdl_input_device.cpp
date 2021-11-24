
#include "precompiled.h"
#include "sdl_input_device.h"

///////////////////////////////////////////////////////////////////////////////
// SdlMouseDevice
///////////////////////////////////////////////////////////////////////////////
bool SdlMouseDevice::get_button_pressed(Button button)
{
    Uint32 buttons = SDL_GetMouseState(nullptr, nullptr);
    switch (button)
    {
        case LMB:
            return buttons & SDL_BUTTON_LMASK;

        case RMB:
            return buttons & SDL_BUTTON_RMASK;
    }

    return false;
}

vec2 SdlMouseDevice::get_position()
{
    int dx, dy;
    SDL_GetRelativeMouseState(&dx, &dy);
    m_mouse_abs.x() += dx;
    m_mouse_abs.y() += dy;

    return m_mouse_abs;
}

///////////////////////////////////////////////////////////////////////////////
// SdlKeyboardDevice
///////////////////////////////////////////////////////////////////////////////
bool SdlKeyboardDevice::get_key_pressed(int keycode)
{
    SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
    if (keycode == KEY_ESCAPE)
        scancode = SDL_SCANCODE_ESCAPE;

    const Uint8* states = SDL_GetKeyboardState(nullptr);
    return states[scancode];
}
