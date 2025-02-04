#pragma once

#include <vector>
#include <queue>

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

private:
    Path calculate_shortest_() const {
        if (next_paths.empty()) {
            return *this;
        }

        const Path *shortest_next = nullptr;
        int min_length = INT_MAX;

        for (const auto &next : next_paths) {
            Path candidate = next.calculate_shortest_();
            int length = 1;
            const Path *current = &candidate;
            while (!current->next_paths.empty()) {
                current = &current->next_paths[0];
                ++length;
            }

            if (length < min_length) {
                min_length = length;
                shortest_next = &next;
            }
        }

        Path result;
        result.position = position;
        if (shortest_next != nullptr) {
            Path next_shortest = shortest_next->calculate_shortest_();
            result.next_paths.push_back(next_shortest);
        }
        return result;
    }

public:
    std::vector<int> calculate_shortest() {
        Path shortest = calculate_shortest_();
        const Path *p = &shortest;
        std::vector<int> result{position};
        while (!p->next_paths.empty()) {
            p = &p->next_paths[0];
            result.push_back(p->position);
        }
        return result;
    }
};

struct DrawPathInfo;

class State {
public:
    // logic stuff
    std::string status_message = "";
    Color status_message_color = WHITE;
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
    int cached_volume = 100;

    // animation stuff
    std::queue<Animation> animation_queue;

    // rendering stuff
    static constexpr Rectangle reset_button = {10, 10, 150, 40};
    static constexpr Rectangle auto_button = {reset_button.x + reset_button.width + 10, 10, 150, 40};
    static constexpr Rectangle save_button = {auto_button.x + auto_button.width + 10, 10, 150, 40};
    static constexpr Rectangle load_button = {save_button.x + save_button.width + 10, 10, 150, 40};
    static constexpr Rectangle sound_toggle_button = {load_button.x + load_button.width + 10, 10, 150, 40};
    static constexpr Rectangle music_toggle_button = {sound_toggle_button.x + sound_toggle_button.width + 10, 10, 150, 40};

    // audio stuff
    bool main_field_is_finished_prev_frame = false;
    bool say_congratulations_when_ready = false;
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
    void auto_move();
    void super_auto();
    void mega_auto();

    Path collect_path(int cur, int depth=0, Path *prev=nullptr);
    bool delete_useless_paths(Path &path);
    bool can_update_path();
    void update_path();
    void draw_path(const Path &path, int depth = 0, DrawPathInfo *info = nullptr);

    void make_swap_animation(int selected, int front);
};

