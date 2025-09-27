#pragma once
#include <string>

struct Palette {
    std::string ramp;
    bool invert = false;
    explicit Palette(std::string r = "@%#*+=-:. ") : ramp(std::move(r)) {}
    char map(unsigned char v) const {
        if (ramp.empty()) return ' ';
        size_t n = ramp.size();
        size_t idx = static_cast<size_t>((v / 255.0) * (n - 1));
        if (invert) idx = (n - 1) - idx;
        return ramp[idx];
    }
};
