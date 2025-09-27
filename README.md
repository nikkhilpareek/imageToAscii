# imagetoascii

High-performance C++17 command‑line tool to convert images → ASCII art (mono or 24‑bit ANSI color) and reconstruct images back from ASCII (including color when ANSI sequences are present).

## Features
- Forward conversion: image (PNG/JPG/etc via OpenCV) → ASCII text
- Configurable target width, character ramp, inversion, color output (ANSI 24‑bit `ESC[38;2;R;G;Bm`)
- Reverse conversion: ASCII (plain or ANSI color) → grayscale or color image
- Adjustable character cell geometry (`--cellw`, `--cellh`) for reverse mode
- Color round‑trip support (text file with ANSI → approximate colored image)
- Simple, extensible architecture (converter + reverse + palette modules)

## Build
### Dependencies
- CMake >= 3.12
- C++17 compiler (Clang / GCC / MSVC)
- OpenCV (core, imgcodecs, imgproc)

### macOS (Homebrew)
```bash
brew install opencv
cmake -S . -B build -DOpenCV_DIR=$(brew --prefix opencv)/lib/cmake/opencv4
cmake --build build -j
```

### Linux (apt example Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake libopencv-dev
cmake -S . -B build
cmake --build build -j
```

### Windows (vcpkg example)
```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh # or .bat
./vcpkg install opencv
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Usage
Executable: `imagetoascii`

### Forward (image → ASCII)
```bash
./build/imagetoascii -i test.jpg --width 100 > out.txt
```

Options:
- `-i, --input <file>` input image (required in forward mode)
- `-w, --width <N>` target character width (default 120)
- `--invert` invert brightness mapping
- `-c, --charset <CHARS>` custom character ramp (densest → lightest)
- `--color` emit 24‑bit ANSI color sequences
- `-o, --output <file>` write ASCII to file instead of stdout

Example with color saved to file:
```bash
./build/imagetoascii -i test.jpg --width 100 --color -o colored.txt
```
(View color in a terminal that supports truecolor.)

### Reverse (ASCII → image)
```bash
./build/imagetoascii --reverse --ascii-input out.txt --cellw 8 --cellh 16 -o restored.png
```

Additional reverse options:
- `--reverse` enable reverse mode
- `--ascii-input <file>` ASCII art input (required in reverse mode)
- `--cellw <N>` assumed character cell pixel width (default 8)
- `--cellh <N>` assumed character cell pixel height (default 16)
- `--reverse-parse-color` parse ANSI 24‑bit foreground color sequences

Color round‑trip:
```bash
./build/imagetoascii -i test.jpg --width 100 --color -o colored.txt
./build/imagetoascii --reverse --ascii-input colored.txt --cellw 8 --cellh 16 --reverse-parse-color -o restored_color.png
```

## Character Mapping
Intensity (0–255) is mapped to an index in the ramp string. For performance a simple proportional mapping is used. (Future: perceptual weighting, density calibration.) Default ramp: `@%#*+=-:. ` (dense → sparse).

## Color Handling
Forward: each character preceded by ANSI 24-bit foreground color escape and line reset at end. Reverse parser detects sequences of the form `ESC[38;2;R;G;Bm` and reconstructs an approximate pixel block (scaled by luminance for shading).

## Reverse Reconstruction
- Each ASCII character becomes a solid rectangle (`cellW × cellH`).
- Grayscale mode: rectangle value derived from ramp index.
- Color mode: ramp luminance scales original foreground color to preserve basic contrast.

## Current Limitations
- No dithering (future enhancement) so flat regions may lose subtle gradients.
- Reverse color ignores background & style attributes (bold, underline).
- Cell size must be supplied (cannot auto-detect yet).

## Roadmap / Ideas
| Category | Planned Enhancements |
|----------|----------------------|
| Quality  | Floyd–Steinberg or ordered dithering, histogram equalization (CLAHE) |
| Color    | True per-pixel color retention (skip luminance scaling) |
| Reverse  | Auto cell size detection, background color parsing, SVG output |
| Performance | LUT precomputation (0–255→char), multithreading, streaming mode |
| Formats  | HTML / SVG / PNG with overlay, video (frames → animated ASCII) |
| Testing  | Unit tests (palette, converter, reverse), golden samples, benchmarks |
| Packaging | `install` target, Homebrew tap, GitHub Actions CI + coverage |

## Performance Tips
- Reduce `--width` to speed up (complexity roughly O(width * height)).
- Use a denser ramp to preserve detail at smaller widths.
- Future optimization: parallel row processing + cached char LUT.

## Project Structure
```
CMakeLists.txt
main.cpp                # CLI
src/palette.h           # Character palette & mapping
src/ascii_converter.*   # Forward conversion
src/ascii_reverse.*     # Reverse conversion (ASCII -> image)
```

## Example Snippet (Forward Conversion Logic)
```cpp
char Palette::map(unsigned char v) const {
    size_t n = ramp.size();
    size_t idx = static_cast<size_t>((v / 255.0) * (n - 1));
    if (invert) idx = (n - 1) - idx;
    return ramp[idx];
}
```

## Contributing
Planned guidelines (future): code style (clang-format), tests required for new features, PR template.
Suggestions & feature requests welcome via Issues.

## License
Choose a permissive license (MIT / Apache-2.0). Add a `LICENSE` file before publishing.

## Attribution / Inspiration
Classic ASCII art techniques; OpenCV used for image loading, color space transforms, resizing.

---
Feel free to open an issue or PR for any feature on the roadmap.
