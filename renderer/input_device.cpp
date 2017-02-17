
#include "precompiled.h"
#include "input_system.h"
#include "win32/win32_input_system.h"

using namespace std;

unique_ptr<MouseDevice> InputDeviceFactory::create_mouse()
{
#ifdef WIN32
    log_info("InputDeviceFactory creating a Win32MouseDevice...");
    return unique_ptr<MouseDevice>(new Win32MouseDevice);
#endif
    return nullptr;
}

unique_ptr<KeyboardDevice> InputDeviceFactory::create_keyboard()
{
#ifdef WIN32
    log_info("InputDeviceFactory creating a Win32KeyboardDevice...");
    return unique_ptr<KeyboardDevice>(new Win32KeyboardDevice);
#endif
    return nullptr;
}
