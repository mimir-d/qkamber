#pragma once

#include "input/input_device.h"

class SdlMouseDevice : public MouseDevice
{
public:
    SdlMouseDevice();
    ~SdlMouseDevice();

public:
    bool get_button_pressed(Button button) final;
    vec2 get_position() final;

private:
    vec2 m_mouse_abs = { 0, 0 };
};

class SdlKeyboardDevice : public KeyboardDevice
{
public:
    SdlKeyboardDevice();
    ~SdlKeyboardDevice();

public:
    bool get_key_pressed(int key_code) final;
};

///////////////////////////////////////////////////////////////////////////////
// impl
///////////////////////////////////////////////////////////////////////////////
inline SdlMouseDevice::SdlMouseDevice()
{
    flog();
    log_info("Created SDL mouse device");
}

inline SdlMouseDevice::~SdlMouseDevice()
{
    flog();
    log_info("Destroyed SDL mouse device");
}

inline SdlKeyboardDevice::SdlKeyboardDevice()
{
    flog();
    log_info("Created SDL keyboard device");
}

inline SdlKeyboardDevice::~SdlKeyboardDevice()
{
    flog();
    log_info("Destroyed SDL keyboard device");
}
