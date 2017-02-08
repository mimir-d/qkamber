
#include "stdafx.h"
#include "win32_input_system.h"

constexpr USHORT HID_USAGE_PAGE_GENERIC = 0x01;
constexpr USHORT HID_USAGE_GENERIC_MOUSE = 0x02;
constexpr USHORT HID_USAGE_GENERIC_KEYBOARD = 0x06;

///////////////////////////////////////////////////////////////////////////////
// Win32MouseDevice
///////////////////////////////////////////////////////////////////////////////
void Win32MouseDevice::win32_init(HWND window_handle)
{
    flog();

    RAWINPUTDEVICE ridev[1];
    ridev[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    ridev[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    ridev[0].dwFlags = RIDEV_INPUTSINK;
    ridev[0].hwndTarget = window_handle;
    RegisterRawInputDevices(ridev, 1, sizeof(ridev[0]));
}

vec2 Win32MouseDevice::get_delta()
{
    vec2 ret = m_mouse_abs - m_mouse_last;
    m_mouse_last = m_mouse_abs;
    return ret;
}

void Win32MouseDevice::feed_input(const RAWMOUSE& raw_input)
{
    m_mouse_abs.x() += static_cast<float>(raw_input.lLastX);
    m_mouse_abs.y() += static_cast<float>(raw_input.lLastY);
}

///////////////////////////////////////////////////////////////////////////////
// Win32KeyboardDevice
///////////////////////////////////////////////////////////////////////////////
void Win32KeyboardDevice::win32_init(HWND window_handle)
{
    flog();

    RAWINPUTDEVICE ridev[1];
    ridev[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    ridev[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    ridev[0].dwFlags = RIDEV_INPUTSINK;
    ridev[0].hwndTarget = window_handle;
    RegisterRawInputDevices(ridev, 1, sizeof(ridev[0]));
}

bool Win32KeyboardDevice::get_key_pressed(int key_code)
{
    return m_keymap[key_code];
}

void Win32KeyboardDevice::feed_input(const RAWKEYBOARD& raw_input)
{
    m_keymap[raw_input.VKey] = !(raw_input.Flags & KEY_BREAK);
}
