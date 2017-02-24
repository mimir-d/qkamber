
#include "precompiled.h"
#include "camera.h"

#include "input_system.h"

void FpsCamera::set_proj_params(int width, int height)
{
    const float aspect = static_cast<float>(width) / height;
    m_proj = mat4::proj_perspective(PI_4, aspect, 0.1f, 100.0f);
}

void FpsCamera::update(float abs_time, float elapsed_time)
{
    // compute camera rotation
    m_rotation += get_rotation_delta();
    m_rotation.x() = clamp(m_rotation.x(), -PI_2, PI_2);
    m_rotation.y() = fmod(m_rotation.y(), 2 * PI);

    // TODO: also fix this, template rotate to produce matN
    mat4 r4 = mat4::rotate(m_rotation.x(), m_rotation.y(), 0.0f);
    const mat3 rotation {
        r4[0][0], r4[0][1], r4[0][2],
        r4[1][0], r4[1][1], r4[1][2],
        r4[2][0], r4[2][1], r4[2][2]
    };
    // TODO: can optimize these muls
    const vec3 ahead = rotation * vec3 { 0, 0, 1 };
    const vec3 up = rotation * vec3 { 0, 1, 0 };

    // compute camera movement
    const vec3 position_delta = get_position_delta(elapsed_time);
    m_position += rotation * position_delta;

    m_view = mat4::lookat(m_position, m_position - ahead, up);
}

vec2 FpsCamera::get_rotation_delta()
{
    auto& mouse = m_input.get_mouse();
    const vec2 mouse_abs = mouse.get_position();

    const vec2 delta = mouse_abs - m_mouse_last_abs;
    m_mouse_last_abs = mouse_abs;

    // smooth the mouse over last couple of frames
    const float smooth_factor = 1.0f / m_rotation_smooth_frames;
    m_mouse_delta = m_mouse_delta * (1.0f - smooth_factor) + delta * smooth_factor;

    return vec2 { -m_mouse_delta.y(), -m_mouse_delta.x() } * m_rotation_scaler;
}

vec3 FpsCamera::get_position_delta(float elapsed_time)
{
    auto& keyboard = m_input.get_keyboard();

    const vec3 accel = vec3
    {
        keyboard.get_key_pressed('A') ? -1.0f : keyboard.get_key_pressed('D') ? 1.0f : 0.0f,
        keyboard.get_key_pressed('Q') ? -1.0f : keyboard.get_key_pressed('E') ? 1.0f : 0.0f,
        keyboard.get_key_pressed('W') ? -1.0f : keyboard.get_key_pressed('S') ? 1.0f : 0.0f
    }.normalize() * m_accel_scaler;

    if (accel.length_sq() > 0)
    {
        // start the movement immediately and accel (this also applies when holding down keys)
        m_velocity = accel;
        m_acceleration = accel * (1.0f / m_accel_duration);
        m_accel_time = m_accel_duration;
    }
    else if (m_accel_time > 0)
    {
        // update velocity based on dragging acceleration
        // NOTE: in low fps, elapsed_time can be big and this overshoots, so if the case, just 0 the velocity
        vec3 old_velocity = m_velocity;
        m_velocity -= m_acceleration * elapsed_time;
        if (old_velocity * m_velocity < 0)
            m_velocity = 0.0f;

        m_accel_time -= elapsed_time;
    }
    else
    {
        // when acceleration time ended, zero everything to eliminate float errors
        m_velocity = 0.0f;
        m_accel_time = 0.0f;
    }

    return m_velocity * elapsed_time;
}
