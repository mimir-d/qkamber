#pragma once

#include "engine.h"

constexpr char SDL_WINDOW_DATA_PTR[] = "sdl_window_data_ptr";

class SdlApp : public App
{
public:
    SdlApp(QkEngine::Context& context);
    ~SdlApp() = default;

    int mainloop() final;

private:
    void render_one();
    void handle_event(const SDL_Event& e);

    void pause_timer(bool enable);
    void handle_window_resize_start(Uint32 window_id);
    void handle_window_resize_end(Uint32 window_id);
};
