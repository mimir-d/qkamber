#pragma once

#include "input_system.h"

// class Win32InputDevice
// {
// public:
//     virtual void win32_init(HWND window_handle) = 0;
//     virtual void feed_input(const RAWINPUT& raw_input) = 0;
// };

class Win32MouseDevice : public MouseDevice
{
public:
    void win32_init(HWND window_handle);
    void feed_input(const RAWMOUSE& raw_input);

public:
    bool get_button_pressed(Button button) final;
    vec2 get_position() final;

private:
    vec2 m_mouse_abs = { 0, 0 };
    bool m_buttons[2] = { 0 };
};

class Win32KeyboardDevice : public KeyboardDevice
{
public:
    void win32_init(HWND window_handle);
    void feed_input(const RAWKEYBOARD& raw_input);

public:
    bool get_key_pressed(int key_code) final;

private:
    bool m_keymap[256] = { 0 };
};
