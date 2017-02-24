#pragma once

#include "math3.h"

class MouseDevice
{
public:
    enum Button { LMB, RMB };

    virtual bool get_button_pressed(Button button) = 0;
    virtual vec2 get_position() = 0;
};

class KeyboardDevice
{
public:
    virtual bool get_key_pressed(int key_code) = 0;
};

class InputSystem
{
public:
    InputSystem();
    ~InputSystem();

    MouseDevice& get_mouse();
    KeyboardDevice& get_keyboard();

private:
    std::unique_ptr<MouseDevice> m_mouse;
    std::unique_ptr<KeyboardDevice> m_keyboard;
};

///////////////////////////////////////////////////////////////////////////////
// Impl
///////////////////////////////////////////////////////////////////////////////
inline MouseDevice& InputSystem::get_mouse()
{
    return *m_mouse;
}

inline KeyboardDevice& InputSystem::get_keyboard()
{
    return *m_keyboard;
}

// win32_input_device for mouse + keyboard