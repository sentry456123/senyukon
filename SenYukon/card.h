#pragma once

#include <string>

#include "util.h"
#include "defs.h"

class Card {
    int internal = nil;

public:
    Card() = default;

    explicit Card(int source)
        : internal{source} {
    }

    int get_raw() const {
        return internal;
    }

    bool is_hidden() const {
        return internal / hidden;
    }

    bool is_nil() const {
        return internal == nil;
    }

    Card show() const {
        return Card{internal / hidden ? internal - hidden : internal};
    }

    Card hide() const {
        return Card{internal / hidden ? internal : internal + hidden};
    }

    int get_pip() const {
        return (show().internal % pips_per_suit) + 1;
    }

    Suit get_suit() const {
        return static_cast<Suit>(show().internal / pips_per_suit);
    }

    SuitColor get_color() const {
        switch (get_suit()) {
        case Suit::heart:
        case Suit::diamond:
            return SuitColor::red;
        case Suit::spade:
        case Suit::club:
            return SuitColor::black;
        }
        SEN_UNREACHABLE();
    }

    std::string to_string() const {
        std::string suit_string = {};
        std::string pip_string = {};
        

        if (is_nil()) {
            return "";
        }
        if (is_hidden()) {
            return "HID";
        }
        switch (get_suit()) {
        case Suit::spade:
            suit_string = "S";
            break;
        case Suit::heart:
            suit_string = "H";
            break;
        case Suit::diamond:
            suit_string = "D";
            break;
        case Suit::club:
            suit_string = "C";
            break;
        default:
            suit_string = "?";
            break;
        }
        switch (get_pip()) {
        case 1:
            pip_string = "A";
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            pip_string = std::to_string(get_pip());
            break;
        case 11:
            pip_string = "J";
            break;
        case 12:
            pip_string = "Q";
            break;
        case 13:
            pip_string = "K";
            break;
        default:
            pip_string = "?";
            break;
        }

        return suit_string + pip_string;
    }
};
