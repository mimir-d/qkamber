#pragma once

#include "engine.h"
#include "subsystem.h"

class MouseDevice;
class KeyboardDevice;

class InputSystem : public Subsystem
{
public:
    InputSystem(QkEngine::Context& context);
    ~InputSystem();

    void process() final {}

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