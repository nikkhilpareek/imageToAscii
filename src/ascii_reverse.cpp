#include "ascii_reverse.h"
#include <vector>
#include <algorithm>
#include <cctype>

struct Cell {
    char ch{' '};
    bool hasColor{false};
    unsigned char r{0}, g{0}, b{0}; // stored as 0-255
};

// Map char -> approximate luminance based on index in ramp.
static unsigned char charToGray(char c, const std::string& ramp) {
    if (ramp.empty()) return 0;
    auto pos = ramp.find(c);
    if (pos == std::string::npos || ramp.size() == 1) {
        return static_cast<unsigned char>((static_cast<unsigned int>(static_cast<unsigned char>(c))));
    }
    double t = static_cast<double>(pos) / (ramp.size() - 1);
    return static_cast<unsigned char>(t * 255.0 + 0.5);
}

// Very small parser for ANSI 24-bit color foreground sequences: ESC[38;2;R;G;Bm
// Resets on ESC[0m or ESC[m.
cv::Mat asciiToImage(const std::string& ascii, const std::string& ramp, const AsciiReverseOptions& opts) {
    std::vector<std::vector<Cell>> lines;
    lines.emplace_back();

    bool anyColor = false;
    unsigned char curR = 255, curG = 255, curB = 255; // default white
    bool haveExplicitColor = false;

    const char ESC = '\x1b';
    size_t i = 0;
    while (i < ascii.size()) {
        char c = ascii[i];
        if (c == '\n') {
            lines.emplace_back();
            ++i; continue;
        }
        if (opts.parseColor && c == ESC) {
            // Parse CSI sequence: ESC [ ... m
            if (i + 1 < ascii.size() && ascii[i+1] == '[') {
                size_t mpos = ascii.find('m', i + 2);
                if (mpos != std::string::npos) {
                    std::string seq = ascii.substr(i + 2, mpos - (i + 2));
                    // Split by ';'
                    std::vector<int> nums; nums.reserve(8);
                    size_t start = 0; while (start <= seq.size()) { size_t sc = seq.find(';', start); if (sc==std::string::npos) { if (start < seq.size()) nums.push_back(std::atoi(seq.substr(start).c_str())); break; } else { if (sc>start) nums.push_back(std::atoi(seq.substr(start, sc-start).c_str())); start = sc+1; } }
                    if (nums.empty()) { haveExplicitColor = false; curR = curG = curB = 255; }
                    else {
                        // Look for 38;2;R;G;B
                        for (size_t k = 0; k + 4 < nums.size(); ++k) {
                            if (nums[k] == 38 && nums[k+1] == 2) {
                                int R = nums[k+2], G = nums[k+3], B = nums[k+4];
                                curR = static_cast<unsigned char>(std::clamp(R,0,255));
                                curG = static_cast<unsigned char>(std::clamp(G,0,255));
                                curB = static_cast<unsigned char>(std::clamp(B,0,255));
                                haveExplicitColor = true; anyColor = true;
                                break;
                            }
                        }
                        // Reset detection
                        for (int val : nums) {
                            if (val == 0) { haveExplicitColor = false; curR = curG = curB = 255; break; }
                        }
                    }
                    i = mpos + 1; // advance past 'm'
                    continue;
                }
            }
        }
        // Regular printable char
        if (lines.back().empty() && c == '\r') { ++i; continue; }
        if (c != '\r') {
            Cell cell; cell.ch = c; if (haveExplicitColor) { cell.hasColor = true; cell.r = curR; cell.g = curG; cell.b = curB; }
            lines.back().push_back(cell);
        }
        ++i;
    }
    // Remove last line if empty and there was a trailing newline
    if (!lines.empty() && lines.back().empty()) lines.pop_back();
    if (lines.empty()) return cv::Mat();

    int textH = static_cast<int>(lines.size());
    int textW = 0; for (auto &ln : lines) textW = std::max<int>(textW, static_cast<int>(ln.size()));
    if (textW == 0) return cv::Mat();

    int imgW = textW * opts.charWidth;
    int imgH = textH * opts.charHeight;
    cv::Mat out;
    if (opts.parseColor && anyColor) out = cv::Mat(imgH, imgW, CV_8UC3, cv::Scalar(0,0,0));
    else out = cv::Mat(imgH, imgW, CV_8UC1, cv::Scalar(0));

    for (int row = 0; row < textH; ++row) {
        auto &ln = lines[row];
        for (int col = 0; col < static_cast<int>(ln.size()); ++col) {
            const Cell &cell = ln[col];
            unsigned char g = charToGray(cell.ch, ramp);
            int x0 = col * opts.charWidth;
            int y0 = row * opts.charHeight;
            if (out.channels() == 1) {
                cv::rectangle(out, cv::Rect(x0, y0, opts.charWidth, opts.charHeight), cv::Scalar(g), cv::FILLED);
            } else {
                // Blend char luminance with its color (scale color by g/255 to approximate shading)
                double scale = g / 255.0;
                cv::Scalar colBGR(cell.b * scale, cell.g * scale, cell.r * scale);
                cv::rectangle(out, cv::Rect(x0, y0, opts.charWidth, opts.charHeight), colBGR, cv::FILLED);
            }
        }
    }
    return out;
}
