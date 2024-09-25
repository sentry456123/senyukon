#include <cassert>
#include <fstream>

#include "raylib.h"
#include "animation.h"

#include "deck.h"
#include "field.h"
#include "sound_manager.h"

Field::Field() {
    Deck source = {};

    source.shuffle();

    for (int y = 0; y < raw_size; y++) {
        for (int x = y; x < raw_size; x++) {
            if (x == y) {
                push(x, source.next().show());
            } else {
                push(x, source.next().hide());
            }
        }
    }
    for (int x = 1;; x++) {
        if (x >= raw_size) {
            x = 1;
        }

        Card obtained = source.next();
        if (obtained.is_nil()) {
            break;
        }
        push(x, obtained.show());
    }
}

void Field::push(int col, Card card) {
    assert(col >= 0 && col < raw_size);
    for (int i = 0; i < yukon_height; i++) {
        if ((*this)[i * raw_size + col].is_nil()) {
            (*this)[i * raw_size + col] = card;
            return;
        }
    }
}

bool Field::is_front(int position) const {
    if (position >= yukon_size - raw_size && position < yukon_size) {
        return true;
    }
    if (position >= 0 && position < yukon_size - raw_size) {
        return (*this)[position + raw_size].is_nil();
    }
    return false;
}

int Field::get_front(int col) const {
    assert(col >= 0 && col < raw_size);
    for (int i = col; i < yukon_size; i += raw_size) {
        if (is_front(i)) {
            return i;
        }
    }

    return nil;
}

void Field::swap(int a, int b) {
    for (; a >= 0 && b >= 0 && a < yukon_size && b < yukon_size; a += raw_size, b += raw_size) {
        if ((*this)[a].is_nil() && (*this)[b].is_nil()) {
            break;
        }

        std::swap((*this)[a], (*this)[b]);
    }
}

void Field::show_available() {
    int i;
    for (i = 0; i < yukon_size - raw_size; i++) {
        if ((*this)[i].is_nil()) {
            continue;
        }
        if (!((*this)[i]).is_hidden()) {
            continue;
        }
        if (!(*this)[i + raw_size].is_nil()) {
            continue;
        }
        (*this)[i] = (*this)[i].show();
    }
    for (; i < yukon_size; i++) {
        if (!(*this)[i].is_nil()) {
            (*this)[i] = (*this)[i].show();
        }
    }
}

bool Field::can_feed_foundation(int position) const {
    if (!is_front(position)) {
        return false;
    }

    Card card_to_feed = (*this)[position];
    if (card_to_feed.is_nil()) {
        return false;
    }
    if (card_to_feed.is_hidden()) {
        return false;
    }

    Card prev = (*this)[yukon_size + static_cast<int>(card_to_feed.get_suit())];
    if (prev.is_nil() && card_to_feed.get_pip() == pip_ace) {
        return true;
    }
    return prev.get_pip() + 1 == card_to_feed.get_pip();
}

void Field::feed_foundation(int position) {
    (*this)[yukon_size + static_cast<int>((*this)[position].get_suit())] = (*this)[position];
    (*this)[position] = Card{nil};
}

void Field::render() {
    for (int x = 0; x < yukon_width; x++) {
        for (int y = 0; y < yukon_height - 1; y++) {
            Card upper = (*this)[x + y * raw_size];
            Card lower = (*this)[x + (y+1) * raw_size];
            
            bool is_tied = true;
            is_tied = is_tied && !upper.is_nil();
            is_tied = is_tied && !lower.is_nil();
            is_tied = is_tied && !upper.is_hidden();
            is_tied = is_tied && !lower.is_hidden();
            is_tied = is_tied && (upper.get_color() != lower.get_color());
            is_tied = is_tied && (upper.get_pip() - 1 == lower.get_pip());
            is_tied = is_tied && (upper.get_pip() != 1);

            if (is_tied) {
                DrawRectangle(x * cell_width - 4, y * cell_height + cell_height + 10, 4, cell_height * 2 - 20, YELLOW);
            }
        }
    }

    for (int y = 0; y < yukon_height; y++) {
        for (int x = 0; x < yukon_width; x++) {
            Card target = (*this)[x + y * raw_size];
            std::string string = target.to_string();
            Color color;
            
            if (string == "HID") {
                color = LIGHTGRAY;
            } else if (target.get_color() == SuitColor::red) {
                color = RED;
            } else {
                color = WHITE;
            }
            if (can_feed_foundation(x + y * raw_size)) {
                color = Fade(color, sin(GetTime() * 5.0) + 1.0);
                DrawText(string.c_str(), x * cell_width, cell_height + y * cell_height, cell_height, color);
            } else {
                DrawText(string.c_str(), x * cell_width, cell_height + y * cell_height, cell_height, color);
            }
        }
    }

    for (int x = 0; x < foundation_count; x++) {
        DrawText((*this)[yukon_size + x].to_string().c_str(), x * cell_width, 0, cell_height, SKYBLUE);
    }

    if (is_finished()) {
        DrawText("Congratulations :)", 0, -cell_height, cell_height, WHITE);
    }
}

bool Field::is_finished() const {
    if ((*this)[yukon_size].get_pip() != pip_king) {
        return false;
    }
    if ((*this)[yukon_size + 1].get_pip() != pip_king) {
        return false;
    }
    if ((*this)[yukon_size + 2].get_pip() != pip_king) {
        return false;
    }
    if ((*this)[yukon_size + 3].get_pip() != pip_king) {
        return false;
    }
    return true;
}

void Field::load_from_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    file.read(reinterpret_cast<char *>(this), sizeof *this);
}

void Field::save_to_file(const std::string &filename) const {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(this), sizeof *this);
}
