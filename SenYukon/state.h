#pragma once

#include <vector>
#include <memory>

#include <raylib.h>

#include "animation.h"
#include "field.h"

enum class StateMode {
    waiting,
    animating,
};

static constexpr int max_path_depth = 16;

struct Path {
    int position = nil;
    std::vector<Path> next_paths{};
    Path *previous_path = nullptr;
};

struct DrawPathInfo;

class State {
public:
    // logic stuff
    std::string status_message = "";
    int cursor = 0;
    int selected = nil;
    Field main_field;
    Camera2D main_camera = {.zoom = 1.0f};
    StateMode mode = StateMode::waiting;
    Path base_path;
    int path_base_position = nil;
    double time_path_created = 0.0;
    bool should_draw_path = false;
    Field field_when_path_created;
    int path_depth_tracker = 0;

    // animation stuff
    std::unique_ptr<Animation> animation;

    // rendering stuff
    static constexpr Rectangle reset_button = {10, 10, 130, 40};
    static constexpr Rectangle auto_button = {reset_button.x + reset_button.width + 10, 10, 130, 40};
    static constexpr Rectangle music_toggle_button = {auto_button.x + auto_button.width + 10, 10, 120, 40};
    static constexpr Rectangle save_button = {music_toggle_button.x + music_toggle_button.width + 10, 10, 120, 40};
    static constexpr Rectangle load_button = {save_button.x + save_button.width + 10, 10, 120, 40};

    // audio stuff
    bool main_field_is_finished_prev_frame = false;
    bool say_conglatulations_when_ready = false;
    Music bgm = {};

public:
    State();
    ~State();
    void handle_input();
    void update();
    void render();

private:
    void handle_yukon_movement();
    void handle_camera_movement();

    void auto_feed();
    

    Path collect_path(int cur, int depth=0, Path *prev=nullptr);
    bool delete_useless_paths(Path &path);
    bool can_update_path();
    void update_path();
    void draw_path(const Path &path, int depth = 0, DrawPathInfo *info = nullptr);

    void make_swap_animation(int selected, int front);
};

