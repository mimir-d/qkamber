#pragma once

#include "math3.h"

class MouseDevice
{
public:
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
    static InputSystem& get_inst();

public:
    InputSystem();
    ~InputSystem() = default;

    MouseDevice& get_mouse();
    KeyboardDevice& get_keyboard();

private:
    std::unique_ptr<MouseDevice> m_mouse;
    std::unique_ptr<KeyboardDevice> m_keyboard;
};

class InputDeviceFactory
{
public:
    static std::unique_ptr<MouseDevice> create_mouse();
    static std::unique_ptr<KeyboardDevice> create_keyboard();
};

///////////////////////////////////////////////////////////////////////////////
// Impl
///////////////////////////////////////////////////////////////////////////////
// TODO: this is NOT ok, refactor when impl systems in engine
inline InputSystem& InputSystem::get_inst()
{
    static InputSystem is;
    return is;
}

inline InputSystem::InputSystem()
{
    m_mouse = InputDeviceFactory::create_mouse();
    m_keyboard = InputDeviceFactory::create_keyboard();
}

inline MouseDevice& InputSystem::get_mouse()
{
    return *m_mouse;
}

inline KeyboardDevice& InputSystem::get_keyboard()
{
    return *m_keyboard;
}

// win32_input_device for mouse + keyboard
// pause should also mean ignore input
// move camera on mouse down