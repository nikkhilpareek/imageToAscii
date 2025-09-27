#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include "palette.h"

struct AsciiOptions {
    int targetWidth = 120;
    double charAspect = 2.0; // height/width ratio compensation
    bool color = false;      // enable 24-bit ANSI color output
};

class AsciiConverter {
public:
    AsciiConverter(Palette p, AsciiOptions o): palette_(std::move(p)), opts_(o) {}
    std::string convert(const cv::Mat& bgr) const;
private:
    Palette palette_;
    AsciiOptions opts_;
};
