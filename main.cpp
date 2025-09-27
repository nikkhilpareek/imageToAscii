#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "src/ascii_converter.h"
#include "src/ascii_reverse.h"

struct Args {
    std::string input;
    int width = 120;
    bool invert = false;
    std::string ramp = "@%#*+=-:. ";
    std::string output; // optional output file path
    bool color = false; // enable ANSI color output
    bool reverse = false; // ascii -> image
    std::string asciiInput; // ascii text file path (if reverse)
    int cellW = 8;
    int cellH = 16;
    bool reverseParseColor = false; // parse ANSI color when reversing
};

static void printHelp() {
    std::cout << "Usage: imagetoascii (forward)  -i <image> [--width N] [--invert] [--charset CHARS] [--output FILE] [--color]\n";
    std::cout << "       imagetoascii (reverse) --reverse --ascii-input <ascii.txt> [--charset CHARS] [--cellw N] [--cellh N] [--reverse-parse-color] -o out.png\n";
}

Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
    if ((s == "-i" || s == "--input") && i + 1 < argc) a.input = argv[++i];
        else if ((s == "-w" || s == "--width") && i + 1 < argc) a.width = std::stoi(argv[++i]);
        else if (s == "--invert") a.invert = true;
    else if ((s == "-c" || s == "--charset") && i + 1 < argc) a.ramp = argv[++i];
    else if ((s == "-o" || s == "--output") && i + 1 < argc) a.output = argv[++i];
    else if (s == "--color") a.color = true;
    else if (s == "--reverse") a.reverse = true;
    else if (s == "--ascii-input" && i + 1 < argc) a.asciiInput = argv[++i];
    else if (s == "--cellw" && i + 1 < argc) a.cellW = std::stoi(argv[++i]);
    else if (s == "--cellh" && i + 1 < argc) a.cellH = std::stoi(argv[++i]);
    else if (s == "--reverse-parse-color") a.reverseParseColor = true;
        else if (s == "-h" || s == "--help") { printHelp(); std::exit(0); }
        else {
            std::cerr << "Unknown option: " << s << "\n"; printHelp(); std::exit(1);
        }
    }
    return a;
}

int main(int argc, char** argv) {
    Args args = parseArgs(argc, argv);
    if (args.reverse) {
        if (args.asciiInput.empty() || args.output.empty()) {
            std::cerr << "Reverse mode requires --ascii-input <file> and -o <output_image>\n"; printHelp();
            return 1;
        }
        std::ifstream ifs(args.asciiInput);
        if (!ifs) { std::cerr << "Failed to open ASCII file: " << args.asciiInput << "\n"; return 1; }
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    AsciiReverseOptions ropts; ropts.charWidth = args.cellW; ropts.charHeight = args.cellH; ropts.parseColor = args.reverseParseColor;
        cv::Mat recon = asciiToImage(content, args.ramp, ropts);
        if (recon.empty()) { std::cerr << "Reconstruction failed (empty result)\n"; return 1; }
        if (!cv::imwrite(args.output, recon)) { std::cerr << "Failed to write output image: " << args.output << "\n"; return 1; }
        std::cerr << "Reconstructed image saved to " << args.output << "\n";
        return 0;
    } else {
        if (args.input.empty()) {
            std::cerr << "Error: --input <path> required\n"; printHelp();
            return 1;
        }
        cv::Mat img = cv::imread(args.input, cv::IMREAD_UNCHANGED);
        if (img.empty()) {
            std::cerr << "Failed to load: " << args.input << "\n";
            return 1;
        }
        Palette p(args.ramp);
        p.invert = args.invert;
        AsciiOptions opts;
        opts.targetWidth = args.width;
        opts.color = args.color;
        AsciiConverter conv(p, opts);
        std::string ascii = conv.convert(img);
        if (!args.output.empty()) {
            std::ofstream ofs(args.output, std::ios::out | std::ios::trunc);
            if (!ofs) {
                std::cerr << "Failed to open output file: " << args.output << "\n";
                return 1;
            }
            ofs << ascii;
            if (!ofs.good()) {
                std::cerr << "Failed to write output file: " << args.output << "\n";
                return 1;
            }
            std::cerr << "Wrote ASCII art to " << args.output << "\n"; // status message to stderr
        } else {
            std::cout << ascii;
        }
        return 0;
    }
}