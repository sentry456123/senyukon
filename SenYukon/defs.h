#pragma once

enum class Suit : int {
    spade = 0,
    heart = 1,
    diamond = 2,
    club = 3,
};

class SuitColor {
public:
    enum Value : uint_fast32_t {
        black = 0,
        red = 1
    };

    SuitColor() = default;
    constexpr SuitColor(Value suit_color)
        : value(suit_color) {
    }

    // Allow switch and comparisons.
    constexpr operator Value() const {
        return value;
    }

    // Prevent usage: if(fruit)
    explicit operator bool() const = delete;


    constexpr SuitColor opposite() {
        return value == black ? red : black;
    }

private:
    Value value;
};

constexpr int pips_per_suit = 13;
constexpr int suit_count = 4;
constexpr int hidden = pips_per_suit * suit_count;
constexpr int nil = -1;
constexpr int raw_size = 7;

constexpr int yukon_width = raw_size;
constexpr int yukon_height = pips_per_suit * suit_count;

constexpr int yukon_size = yukon_width * yukon_height;
constexpr int foundation_count = suit_count;

constexpr int cell_width = 80;
constexpr int cell_height = 40;

constexpr float camera_speed = 2.5f;
constexpr float zoom_speed = 0.1f;

constexpr int pip_ace = 1;
constexpr int pip_king = 13;

constexpr int card_min = 0;
constexpr int card_max = hidden * 2 - 1;

static_assert(hidden != 0);
static_assert(pips_per_suit != 0);
