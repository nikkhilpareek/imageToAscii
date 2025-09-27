#include "ascii_converter.h"
#include <sstream>
#include <algorithm>

std::string AsciiConverter::convert(const cv::Mat& bgr) const {
    if (bgr.empty()) return "Error: empty image\n";
    cv::Mat gray;
    cv::Mat bgr3;
    if (bgr.channels() == 3) {
        bgr3 = bgr;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    } else if (bgr.channels() == 4) {
        cv::cvtColor(bgr, bgr3, cv::COLOR_BGRA2BGR);
        cv::cvtColor(bgr3, gray, cv::COLOR_BGR2GRAY);
    } else if (bgr.channels() == 1) {
        gray = bgr;
        cv::cvtColor(bgr, bgr3, cv::COLOR_GRAY2BGR);
    } else {
        return "Unsupported channel count\n";
    }

    int w = opts_.targetWidth;
    if (w <= 0) w = std::max(1, gray.cols);
    double scale = static_cast<double>(w) / gray.cols;
    int h = std::max(1, static_cast<int>(gray.rows * scale / opts_.charAspect));

    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(w, h), 0, 0, cv::INTER_AREA);
    cv::Mat colorResized;
    if (opts_.color) {
        cv::resize(bgr3, colorResized, cv::Size(w, h), 0, 0, cv::INTER_AREA);
    }

    std::string out;
    out.reserve(static_cast<size_t>((w + 1) * h));

    if (!opts_.color) {
        for (int y = 0; y < resized.rows; ++y) {
            const unsigned char* row = resized.ptr<unsigned char>(y);
            for (int x = 0; x < resized.cols; ++x) {
                out.push_back(palette_.map(row[x]));
            }
            out.push_back('\n');
        }
        return out;
    }

    // Color path: emit 24-bit ANSI foreground color per character.
    for (int y = 0; y < resized.rows; ++y) {
        const unsigned char* rowGray = resized.ptr<unsigned char>(y);
        const unsigned char* rowColor = colorResized.ptr<unsigned char>(y);
        for (int x = 0; x < resized.cols; ++x) {
            unsigned char g = rowGray[x];
            unsigned char b = rowColor[3 * x + 0];
            unsigned char g2 = rowColor[3 * x + 1];
            unsigned char r = rowColor[3 * x + 2];
            (void)g; // g only used for mapping
            char c = palette_.map(rowGray[x]);
            // ESC[38;2;R;G;Bm
            out += "\x1b[38;2;" + std::to_string((int)r) + ';' + std::to_string((int)g2) + ';' + std::to_string((int)b) + 'm';
            out.push_back(c);
        }
        out += "\x1b[0m\n"; // reset each line to avoid bleed
    }
    out += "\x1b[0m"; // final reset
    return out;
}
