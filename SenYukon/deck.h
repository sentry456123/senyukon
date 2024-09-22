#pragma once

#include <array>
#include <random>

#include "card.h"

class Deck {
    std::array<Card, pips_per_suit * suit_count> internal = {};
    int iter = 0;

public:
    Deck() {
        for (int i = 0; i < internal.size(); i++) {
            internal[i] = Card(i);
        }
    }

    void shuffle() {
        std::shuffle(internal.begin(), internal.end(), std::mt19937(std::random_device()()));
    }

    Card next() {
        if (iter >= internal.size()) {
            return Card(nil);
        }
        return internal[iter++];
    };
};
