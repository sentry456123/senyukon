#include <cassert>

#include "raylib.h"
#include "raymath.h"

#include "animation.h"
#include "field.h"
#include "sound_manager.h"

static void apply_movement(Field &field, const Animation::Movement &movement) {
    field[movement.to] = field[movement.from];
    field[movement.from] = Card{nil};
}

static Vector2 position_to_vector(int position) {
    int x = position % yukon_width;
    int y = position / yukon_width;

    if (position >= yukon_size) {
        return Vector2{(float)(x * cell_width), 0};
    }
    return Vector2{(float)(x * cell_width), (float)(cell_height + y * cell_height)};
}

Animation::Animation(const Field &field, double time_frame_take)
    : field(field), time_frame_take(time_frame_take) {
    time_this_created = GetTime();
}

void Animation::record_frame(Movement &&movement) {
    frames.push_back(std::move(movement));
}

bool Animation::is_finished() const {
    return (GetTime() - time_this_created) >= frames.size() * time_frame_take;
}

void Animation::render() {
    Field dummy_field{};

    dummy_field = field;

    int index = (int)((GetTime() - time_this_created) / time_frame_take);
    if (played_sound_index != index) {
        SoundManager::get_singleton()->play_sound("resources/sfx/move.wav");
        played_sound_index = index;
    }

    for (int i = 0; i < index; i++) {
        apply_movement(dummy_field, frames[i]);
    }

    Movement current_movement;

    if (index >= frames.size()) {
        current_movement = *(frames.end()-1);
    } else {
        current_movement = frames[index];
    }

    int from = current_movement.from;
    int to = current_movement.to;

    for (int y = 0; y < yukon_height; y++) {
        for (int x = 0; x < yukon_width; x++) {
            Card target = dummy_field[x + y * raw_size];
            std::string string = target.to_string();
            Color color;

            if (string == "HID") {
                color = LIGHTGRAY;
            } else if (target.get_color() == SuitColor::red) {
                color = RED;
            } else {
                color = WHITE;
            }

            if (from == (y * yukon_width + x)) {
                Vector2 vfrom = position_to_vector(y*yukon_width+x);
                Vector2 vto = position_to_vector(to);
                double time_since_this_created = GetTime() - time_this_created;
                float amount = time_since_this_created;
                amount = (time_since_this_created - (((float)index) / (1.0f / time_frame_take))) * (1.0f / time_frame_take);
                Vector2 result = Vector2Lerp(vfrom, vto, amount);
                DrawTextEx(GetFontDefault(), string.c_str(), result, cell_height, 1.0f, color);
            } else {
                DrawText(string.c_str(), x * cell_width, cell_height + y * cell_height, cell_height, color);
            }
        }
    }

    for (int x = 0; x < foundation_count; x++) {
        DrawText(dummy_field[yukon_size + x].to_string().c_str(), x * cell_width, 0, cell_height, SKYBLUE);
    }
}

