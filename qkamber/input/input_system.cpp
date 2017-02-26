
#include "precompiled.h"
#include "input_system.h"

#include "platform.h"

InputSystem::InputSystem(QkEngine::Context& context) :
    Subsystem(context)
{
    flog("id = %#x", this);
    m_mouse = MouseDeviceFactory::create();
    m_keyboard = KeyboardDeviceFactory::create();

    log_info("Finished creating input system");
}

InputSystem::~InputSystem()
{
    flog();
    log_info("Destroyed input system");
}
