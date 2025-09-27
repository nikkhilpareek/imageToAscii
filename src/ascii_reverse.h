#pragma once
#include <string>
#include <opencv2/opencv.hpp>

struct AsciiReverseOptions {
    int charWidth = 8;          // character cell width in pixels
    int charHeight = 16;        // character cell height in pixels
    bool parseColor = false;    // parse ANSI 24-bit color escape sequences (\x1b[38;2;R;G;Bm)
};

// Reconstruct image from ASCII. If parseColor is true and ANSI 24-bit color codes
// are present, a 3-channel BGR image is produced; otherwise a single-channel grayscale.
cv::Mat asciiToImage(const std::string& ascii, const std::string& ramp, const AsciiReverseOptions& opts);
