#pragma once

#include <vector>

#include "field.h"

class Animation {
    std::vector<Field> frames;
    size_t iter = 0;

public:
    void record_frame(const Field &new_frame);
    bool has_next() const;
    Field &next();
};