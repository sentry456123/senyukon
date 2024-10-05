#include <cassert>
#include <algorithm>
#include <sstream>

#include "state.h"

#include "raymath.h"
#include "sound_manager.h"
#include "resource_manager.h"

struct DrawPathInfo {
    std::array<int, yukon_width> final_depth_display_count;
};

static constexpr std::array pip_keys = {
    /* order is very important - do not change it! */
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_FOUR,
    KEY_FIVE,
    KEY_SIX,
    KEY_SEVEN,
    KEY_EIGHT,
    KEY_NINE,
    KEY_ZERO,
    KEY_J,
    KEY_Q,
    KEY_K,
};

static constexpr bool is_key_pip(int key) {
    for (int k : pip_keys) {
        if (key == k) {
            return true;
        }
    }
    return false;
}

static constexpr int key_to_pip(int key) {
    for (int i = 0; i < pip_keys.size(); i++) {
        if (key == pip_keys[i]) {
            return i + 1;
        }
    }
    return 0;
}

static_assert(is_key_pip(KEY_J) == true);
static_assert(is_key_pip(KEY_N) == false);
static_assert(key_to_pip(KEY_ONE) == 1);
static_assert(key_to_pip(KEY_J) == 11);
static_assert(key_to_pip(KEY_Z) == 0);

static bool is_placeable(Card prev, Card next) {
    if (prev.is_hidden()) {
        return false;
    }
    if (next.is_hidden()) {
        return false;
    }
    if (prev.get_color() == next.get_color()) {
        return false;
    }
    return prev.get_pip() + 1 == next.get_pip();
}

State::State() {
    ResourceManager::startup_singleton();
    SoundManager::startup_singleton(ResourceManager::get_singleton());
    bgm = LoadMusicStream("bgm.ogg");
    prev_time = GetTime();
}

State::~State() {
    UnloadMusicStream(bgm);
    SoundManager::shutdown_singleton();
    ResourceManager::shutdown_singleton();
}

void State::handle_input() {
    switch (mode) {
    case StateMode::waiting:
        handle_yukon_movement();
        break;
    case StateMode::animating:
        break;
    }

    handle_camera_movement();
}

void State::update() {
    if (field_when_path_created != main_field) {
        should_draw_path = false;
    }

    main_camera.offset = Vector2{GetRenderWidth() / 2.0f, GetRenderHeight() / 2.0f};

    if (mode == StateMode::waiting) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(GetMousePosition(), reset_button)) {
                main_field = {};
            }
            if (CheckCollisionPointRec(GetMousePosition(), auto_button)) {
                animation = std::make_unique<Animation>(main_field, 0.1);
                auto_feed();
                mode = StateMode::animating;
            }
            if (CheckCollisionPointRec(GetMousePosition(), music_toggle_button)) {
                if (IsMusicStreamPlaying(bgm)) {
                    PauseMusicStream(bgm);
                } else {
                    PlayMusicStream(bgm);
                }
            }
            if (CheckCollisionPointRec(GetMousePosition(), save_button)) {
                main_field.save_to_file("save");
                status_message = "INFO: Saved game data as \"save\"";
            }
            if (CheckCollisionPointRec(GetMousePosition(), load_button)) {
                main_field.load_from_file("save");
                status_message = "INFO: Loaded game data from \"save\"";
            }
        }
    }

    if (mode == StateMode::animating && animation->is_finished()) {
        mode = StateMode::waiting;
        animation.reset();
    }
    
    if (main_field.is_finished() && !main_field_is_finished_prev_frame) {
        say_conglatulations_when_ready = true;
    }

    if (say_conglatulations_when_ready && mode == StateMode::waiting) {
        SoundManager::get_singleton()->play_sound("sfx/conglatulations.wav");
        say_conglatulations_when_ready = false;
    }

    main_field_is_finished_prev_frame = main_field.is_finished();
    UpdateMusicStream(bgm);
}

void State::render() {
    BeginMode2D(main_camera);

    ClearBackground(BLACK);
    if (selected == nil) {
        DrawRectangle((cursor % raw_size) * cell_width, cell_height + (cursor / raw_size) * cell_height, cell_width, cell_height, BLUE);
    } else {
        DrawRectangle((cursor % raw_size) * cell_width, cell_height, cell_width, cell_height * pips_per_suit * suit_count, BLUE);
        DrawRectangle((selected % raw_size) * cell_width, cell_height + (selected / raw_size) * cell_height, cell_width, cell_height, GREEN);
    }

    switch (mode) {
    case StateMode::waiting:
        main_field.render();

        if (selected == nil) {
            Card cursor_card = main_field[cursor];
            if (!cursor_card.is_nil() && !cursor_card.is_hidden()) {
                for (int y = 0; y < yukon_height; y++) {
                    for (int x = 0; x < yukon_width; x++) {
                        Card card = main_field[y * yukon_width + x];
                        bool should_be_highlighted = true;
                        should_be_highlighted = should_be_highlighted && !card.is_nil();
                        should_be_highlighted = should_be_highlighted && !card.is_hidden();
                        should_be_highlighted = should_be_highlighted && cursor_card.get_color() != card.get_color();
                        should_be_highlighted = should_be_highlighted && cursor_card.get_pip() + 1 == card.get_pip();
                        if (should_be_highlighted) {
                            Color color = Color{0, 228, 48, (unsigned char)((sin(GetTime() * 5.0) + 1.0) * 25.0)};
                            DrawRectangle(x * cell_width, y * cell_height + cell_height, cell_width, cell_height, color);
                        }
                    }
                }
            }
            
            if (should_draw_path) {
                draw_path(base_path);
            }
        }
        break;
    case StateMode::animating:
        animation->render();
        break;
    }

    Vector2 world_mouse = GetScreenToWorld2D(GetMousePosition(), main_camera);
    int focus_x = world_mouse.x / cell_width;
    int focus_y = world_mouse.y / cell_height - 1;
    bool is_x_in = focus_x >= 0 && focus_x < yukon_width;
    bool is_y_in = focus_y >= 0 && focus_y < yukon_height;
    if (is_x_in && is_y_in) {
        DrawRectangle(focus_x * cell_width, focus_y * cell_height + cell_height, cell_width, cell_height, Color{102, 191, 255, 100});
    }

    EndMode2D();

    Camera2D ui_camera = {.zoom = 1.0f};
    BeginMode2D(ui_camera);

    float status_message_box_height = 50.0f;
    Rectangle status_message_box{0.0f, float(GetRenderHeight() - status_message_box_height), float(GetRenderWidth()), status_message_box_height};
    DrawRectangleRec(status_message_box, GRAY);

    DrawText(status_message.c_str(), int(status_message_box.x), int(status_message_box.y), int(status_message_box.height), WHITE);

    Vector2 mousePosition = GetMousePosition();

    bool reset_button_collision = CheckCollisionPointRec(mousePosition, reset_button);
    DrawRectangleRec(reset_button, reset_button_collision ? WHITE : GRAY);
    DrawText("Reset", int(reset_button.x + 5.0f), int(reset_button.y), int(reset_button.height), reset_button_collision ? BLACK : WHITE);

    bool auto_button_collision = CheckCollisionPointRec(mousePosition, auto_button);
    DrawRectangleRec(auto_button, auto_button_collision ? WHITE : GRAY);
    DrawText("Auto", int(auto_button.x + 5.0f), int(auto_button.y), int(auto_button.height), auto_button_collision ? BLACK : WHITE);

    bool music_toggle_button_collision = CheckCollisionPointRec(mousePosition, music_toggle_button);
    DrawRectangleRec(music_toggle_button, music_toggle_button_collision ? WHITE : GRAY);
    DrawText("Music", int(music_toggle_button.x + 5.0f), int(music_toggle_button.y), int(music_toggle_button.height), music_toggle_button_collision ? BLACK : WHITE);

    bool save_button_collision = CheckCollisionPointRec(mousePosition, save_button);
    DrawRectangleRec(save_button, save_button_collision ? WHITE : GRAY);
    DrawText("Save", int(save_button.x + 5.0f), int(save_button.y), int(save_button.height), save_button_collision ? BLACK : WHITE);

    bool load_button_collision = CheckCollisionPointRec(mousePosition, load_button);
    DrawRectangleRec(load_button, load_button_collision ? WHITE : GRAY);
    DrawText("Load", int(load_button.x + 5.0f), int(load_button.y), int(load_button.height), load_button_collision ? BLACK : WHITE);

    EndMode2D();
}

void State::handle_yukon_movement() {
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (IsKeyPressed(KEY_W)) {
        if (cursor / raw_size) {
            cursor -= raw_size;
        }
    }
    if (IsKeyPressed(KEY_S)) {
        if (!(cursor / (yukon_size - raw_size))) {
            cursor += raw_size;
        }
    }
    if (IsKeyPressed(KEY_A)) {
        cursor--;
        if (!((cursor + 1) % raw_size)) {
            cursor += raw_size;
        }
    }
    if (IsKeyPressed(KEY_D)) {
        cursor++;
        if (!(cursor % raw_size)) {
            cursor -= raw_size;
        }
    }
    if (IsKeyPressed(KEY_T)) {
        should_draw_path = false;
        if (shift) {
            cursor = cursor % yukon_width; // get the top of yukon col
        } else {
            cursor = cursor % yukon_width; // get the top of yukon col
            while (main_field[cursor].is_hidden()) {
                cursor += yukon_width;
            }
        }
    }

    if (IsKeyPressed(KEY_B)) {
        should_draw_path = false;
        if (shift) {
            cursor = yukon_width * (yukon_height - 1) + (cursor % yukon_width);
        } else {
            cursor = cursor % yukon_width; // get the top of yukon col
            cursor = main_field.get_front(cursor % yukon_width);
        }
    }

    if (path_base_position != cursor) {
        path_base_position = cursor;
        SoundManager::get_singleton()->play_sound("sfx/cursor_move.wav");
        if (can_update_path()) {
            update_path();

        } else {
            should_draw_path = false;
        }
    }

    if (IsKeyPressed(KEY_Z)) {
        if (can_update_path()) {
            update_path();
        }
    }

    main_camera.zoom = std::clamp(main_camera.zoom, 0.3f, 10.0f);
    cursor = std::clamp(cursor, 0, yukon_size - 1);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 world_mouse = GetScreenToWorld2D(GetMousePosition(), main_camera);
        int cursor_x = world_mouse.x / cell_width;
        int cursor_y = world_mouse.y / cell_height - 1;
        bool is_x_in = cursor_x >= 0 && cursor_x < yukon_width;
        bool is_y_in = cursor_y >= 0 && cursor_y < yukon_height;
        if (is_x_in && is_y_in) {
            cursor = cursor_y * raw_size + cursor_x;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (selected == nil) {
            if (!main_field[cursor].is_nil() && !main_field[cursor].is_hidden()) {
                selected = cursor;
                SoundManager::get_singleton()->play_sound("sfx/select.wav");
            }
        } else if (main_field[selected].get_pip() == pip_king) {
            do {
                if (cursor % raw_size == selected % raw_size) {
                    if (main_field.can_feed_foundation(cursor)) {
                        animation = std::make_unique<Animation>(main_field, 0.05);
                        animation->record_frame(Animation::Movement(cursor, yukon_size + (int)main_field[cursor].get_suit()));
                        mode = StateMode::animating;
                        main_field.feed_foundation(cursor);
                    }
                    selected = nil;
                    break;
                }
                int front = main_field.get_front(cursor % raw_size);
                if (front >= raw_size) {
                    break;
                }
                if (!main_field[front].is_nil()) {
                    break;
                }
                make_swap_animation(selected, front);
                main_field.swap(selected, front);
                selected = nil;
            } while (false);
        } else {
            do {
                if (cursor % raw_size == selected % raw_size) {
                    if (main_field.can_feed_foundation(cursor)) {
                        animation = std::make_unique<Animation>(main_field, 0.05);
                        animation->record_frame(Animation::Movement(cursor, yukon_size + (int)main_field[cursor].get_suit()));
                        mode = StateMode::animating;
                        main_field.feed_foundation(cursor);
                    }
                    selected = nil;
                    break;
                }
                int front = main_field.get_front(cursor % raw_size);
                if (!is_placeable(main_field[selected], main_field[front])) {
                    break;
                }
                make_swap_animation(selected, front);
                main_field.swap(selected, front + raw_size);
                selected = nil;
            } while (false);
        }

        if (selected == nil) {
            main_field.show_available();
        }
    }

    if (IsKeyPressed(KEY_F5)) {
        status_message = (std::stringstream() << "Current Yukon address: 0x" << std::hex << std::uppercase << (uintptr_t)main_field.debug_get_raw()).str();
    }

    {
        int key = GetKeyPressed();
        int pip = key_to_pip(key);
        if (is_key_pip(key)) {

            bool found = false;
            int c = cursor + yukon_width;
            for (;;) {
                if (c >= yukon_size) {
                    c = c % yukon_width;
                }

                found = true;
                found = found && !main_field[c].is_nil();
                found = found && !main_field[c].is_hidden();
                found = found && main_field[c].get_pip() == pip;

                if (found) {
                    cursor = c;
                    break;
                }

                if (c == cursor) {
                    break;
                }

                c += yukon_width;
            }

            if (found) {
                status_message = "";
            } else {
                status_message = "ERROR: Pip not found";
                SoundManager::get_singleton()->play_sound("sfx/error.wav");
            }
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (selected != nil) {
            selected = nil;
            SoundManager::get_singleton()->play_sound("sfx/cancel.wav");
        }
    }
}

void State::handle_camera_movement() {
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyDown(KEY_UP)) {
            main_camera.zoom += zoom_speed * GetFrameTime();
        }
        if (IsKeyDown(KEY_DOWN)) {
            main_camera.zoom -= zoom_speed * GetFrameTime();
        }
    } else {
        float speed_mod = shift ? 3.0f : 1.0f;
        if (IsKeyDown(KEY_UP)) {
            main_camera.target.y -= camera_speed * speed_mod * GetFrameTime();
        }
        if (IsKeyDown(KEY_DOWN)) {
            main_camera.target.y += camera_speed * speed_mod * GetFrameTime();
        }
        if (IsKeyDown(KEY_LEFT)) {
            main_camera.target.x -= camera_speed * speed_mod * GetFrameTime();
        }
        if (IsKeyDown(KEY_RIGHT)) {
            main_camera.target.x += camera_speed * speed_mod * GetFrameTime();
        }
    }
}

void State::auto_feed() {
    bool cont = true;

    while (cont) {
        cont = false;
        for (int col = 0; col < raw_size; col++) {
            int front = main_field.get_front(col % raw_size);
            if (main_field.can_feed_foundation(front)) {
                Animation::Movement movement = { 
                    /* from */ front,
                    /* to */   yukon_size + (int)main_field[front].get_suit()
                };
                animation->record_frame(std::move(movement));
                main_field.feed_foundation(front);
                cont = true;
            }
        }
    }

    main_field.show_available();
}

Path State::collect_path(int cur, int depth, Path *prev) {
    Path result;

    result.position = cur;
    if (prev) {
        result.previous_path = prev;
    }

    if (depth >= max_path_depth || depth < 0) {
        return result;
    }

    Card card = main_field[cur];
    if (card.is_nil() || card.is_hidden()) {
        return result;
    }

    std::vector<int> next;

    int next_pip = card.get_pip() + 1;
    SuitColor next_color = card.get_color().opposite();

    do {
        if (next_pip == 14) {
            if (cur % yukon_width == cur) {
                break;
            }
            for (int x = 0; x < yukon_width; x++) {
                if (main_field[x].is_hidden())
                    continue;
                if (main_field[x].get_pip() == pip_king)
                    continue;
                next.push_back(x);
            }
            break;
        }

        for (int i = 0; i < yukon_size; i++) {
            Card ca = main_field[i];

            if (ca.is_nil())
                continue;
            if (ca.is_hidden())
                continue;
            if (ca.get_color() != next_color)
                continue;
            if (ca.get_pip() != next_pip)
                continue;
            if (!main_field[i + yukon_width].is_nil()) {
                main_field[i + yukon_width];
            }
            if ((i % yukon_width) == (cur % yukon_width)) {
                if (i + yukon_width >= yukon_size || main_field[i + yukon_width].is_nil()) {
                    next.push_back(i);
                }
            } else {
                if (i + yukon_width < yukon_size) {
                    next.push_back(i + yukon_width);
                }
            }
        }
    } while (0);

    for (auto c : next) {
        bool to_continue = false;
        Path *p = &result;
        while (p->previous_path) {
            p = p->previous_path;
            
            if (c == p->position) {
                to_continue = true;
                break;
            }
        }
        if (to_continue) {
            continue;
        }

        auto path = collect_path(c, depth + 1, &result);
        result.next_paths.push_back(path);
    }

    return result;
}

bool State::delete_useless_paths(Path &path) {
    std::vector<std::vector<Path>::iterator> pathToDelete;

    for (int i = 0; i < path.next_paths.size(); i++) {
        if (delete_useless_paths(path.next_paths[i])) {
            path.next_paths.erase(path.next_paths.begin() + i);
            i--;
        }
    }

    if (!path.next_paths.empty())
        return false;
    if (path.position == nil)
        return true;
    if (main_field[path.position].is_nil())
        return false;
    return true;
}

bool State::can_update_path() {
    auto card = main_field[cursor];
    bool should_enable = true;
    should_enable = should_enable && !card.is_nil();
    should_enable = should_enable && !card.is_hidden();
    return should_enable;
}

void State::update_path() {
    should_draw_path = true;
    base_path = collect_path(cursor);
    delete_useless_paths(base_path);
    time_path_created = GetTime();
    field_when_path_created = main_field;
    path_depth_tracker = 0;
}

static constexpr double animation_speed = 15.0;

void State::draw_path(const Path &path, int depth, DrawPathInfo *info) {
    if (depth > path_depth_tracker) {
        path_depth_tracker = depth;
        SoundManager::get_singleton()->play_sound("sfx/path.wav");
    }
    
    bool created_info = false;

    if (info == nullptr) {
        info = new DrawPathInfo();
        created_info = true;
    }

    if (GetTime() - ((double)depth / animation_speed) < time_path_created) {
        return;
    }

    auto position_to_vec = [](int position) -> Vector2 {
        Vector2 result{};
        result.x = (position % yukon_width) * cell_width + (cell_width * 0.5);
        result.y = (position / yukon_width) * cell_height + cell_height + (cell_height * 0.5);
        return result;
    };

    for (auto &next : path.next_paths) {
        Vector2 from = position_to_vec(path.position);
        Vector2 to = position_to_vec(next.position);

        Color color = LIME;
        float normalized_depth = -((float)depth / (float)max_path_depth) + 1.0f;
        float time_since_path_created = GetTime() - time_path_created;
        Vector2 animated_point = Vector2Lerp(from, to, Clamp((time_since_path_created - (((float)depth) / animation_speed)) * animation_speed, 0.0f, 1.0f));
        color = Fade(color, normalized_depth);
        DrawLineEx(from, animated_point, 4.0f * normalized_depth, color);

        if (main_field[next.position].is_nil()) {
            auto &count = info->final_depth_display_count[next.position % yukon_width];
            DrawTextEx(GetFontDefault(), std::to_string(depth + 1).c_str(), Vector2Add(to, Vector2{1.0f, 30.0f + 30.0f * count}), 30, 1.0f, LIME);
            count++;
        }

        draw_path(next, depth + 1, info);
    }

    if (created_info) {
        delete info;
    }
}

void State::make_swap_animation(int selected, int front) {
    using Movement = Animation::Movement;

    animation = std::make_unique<Animation>(main_field, 0.05);

    if (main_field[selected].get_pip() != pip_king) {
        front += yukon_width;     
    }

    while (selected < yukon_size && !main_field[selected].is_nil()) {
        animation->record_frame(Movement(selected, front));
        selected += yukon_width;
        front += yukon_width;
    }
    mode = StateMode::animating;
}
