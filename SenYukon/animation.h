#pragma once

#include <vector>

#include "field.h"

struct Movement {
    int from = nil;
    int to = nil;

    constexpr Movement() = default;
    constexpr Movement(int from, int to) : from(from), to(to) {}
};

class Animation {
    Field field;
    std::vector<Movement> frames;
    double time_this_created;
    double time_frame_take;
    int played_sound_index = -1;

public:
    Animation(const Field &field, double time_frame_take);
    void start();
    void record_frame(Movement &&movement);
    bool is_finished() const;
    void render();
};