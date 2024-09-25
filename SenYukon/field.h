#pragma once

#include <array>

#include "card.h"

class Animation;
 
class Field {
    std::array<Card, yukon_size + foundation_count> internal = {};

public:
    Field();
    void push(int col, Card card);
    bool is_front(int position) const;
    int get_front(int col) const;
    void swap(int a, int b);
    void show_available();
    bool can_feed_foundation(int position) const;
    void feed_foundation(int position);
    void render();
    bool is_finished() const;
    void load_from_file(const std::string &filename);
    void save_to_file(const std::string &filename) const;

    size_t size() const {
        return internal.size();
    }

    Card &operator[](int index) {
        return internal[static_cast<size_t>(index)];
    }

    Card operator[](int index) const {
        return internal[static_cast<size_t>(index)];
    }

    bool operator==(const Field &field) const {
        return memcmp(internal.data(), field.internal.data(), sizeof internal) == 0;
    }

    bool operator!=(const Field &field) const {
        return !((*this) == field);
    }

    Card* debug_get_raw() {
        return internal.data();
    }
};
