#pragma once

#include "math3.h"

#define KEY_ESCAPE 0x1ff

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
