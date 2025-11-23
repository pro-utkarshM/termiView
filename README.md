# TermiView

> A command-line tool that displays images as colorized ASCII art in your terminal - and a playground for experimenting with image signal processing.

---

## Overview

**TermiView** is a lightweight CLI tool built for developers, artists, and learners who want to explore how images can be represented, transformed, and visualized - all inside a terminal.

Originally designed to render **colorized ASCII art**, TermiView is evolving into a **visual lab** for learning and experimenting with **signal processing techniques** applied to images.

---

## Features

### ✅ Implemented (Phase 1 Complete)

- **ASCII Renderer** – Convert images into ASCII art with adjustable resolution
- **Colorized Output** – Full color support with multiple modes:
  - **None** – Classic grayscale ASCII
  - **16-color** – ANSI 16-color terminal support
  - **256-color** – Extended ANSI 256-color palette
  - **Truecolor** – 24-bit RGB color (16.7 million colors)
- **RGB Image Support** – Full color image processing with separate channel handling
- **Dark/Light Mode** – Optimized for both dark and light terminal backgrounds
- **Professional CLI** – Modern argument parsing with long and short options
- **File Output** – Save ASCII art to text files
- **Error Handling** – Comprehensive error checking and memory management
- **Build System** – Complete Makefile with install/uninstall targets

### 🚧 Planned (Future Phases)

- **Image Filters** – Apply kernels like blur, sharpen, or edge-detection (Sobel, Laplacian, etc.)
- **Frequency Analysis** – View Fourier transform magnitude & phase as ASCII plots
- **Histogram Visualization** – Display intensity or color histograms right in the terminal
- **Animation Support** – Step through filter applications frame by frame
- **Real-time Video** – Process camera/video input in real-time
- **Plugin System** – Extensible architecture for custom filters

---

## Installation

### From Source

```bash
git clone https://github.com/pro-utkarshM/termiview.git
cd termiview
make
sudo make install
```

### Dependencies

* **GCC / Clang** – C compiler
* **stb_image.h** – Image loading (included)
* **FFTW** – For Fourier transforms *(optional, for future features)*

---

## Usage

### Basic Usage

Convert and view an image as colorized ASCII:

```bash
termiView image.jpg
```

### Advanced Options

```bash
termiView [OPTIONS] <image_path>

Options:
  -w, --width <num>      Maximum width in characters (default: 64)
  -h, --height <num>     Maximum height in characters (default: 48)
  -c, --color <mode>     Color mode: none, 16, 256, truecolor (default: truecolor)
  -d, --dark             Use dark mode (default)
  -l, --light            Use light mode
  -o, --output <file>    Save output to file instead of stdout
  -v, --version          Show version information
  --help                 Show this help message
```

### Examples

**Truecolor output with custom dimensions:**
```bash
termiView -w 80 -h 60 --color truecolor image.png
```

**256-color mode for wider terminal compatibility:**
```bash
termiView --color 256 photo.jpeg
```

**Light mode for light terminal backgrounds:**
```bash
termiView --light --color 16 image.jpg
```

**Save to file:**
```bash
termiView -c truecolor -o output.txt image.jpg
```

**Classic grayscale ASCII:**
```bash
termiView --color none image.png
```

---

## Development

### Building

**Release build (optimized):**
```bash
make
```

**Debug build:**
```bash
make debug
```

**Run tests:**
```bash
make test
```

**Clean build artifacts:**
```bash
make clean
```

### Project Structure

```
termiView/
├── Makefile                    # Build system
├── README.md                   # Documentation
├── assets/                     # Sample images
│   ├── dragon_warrior.jpeg
│   └── kitty.jpeg
├── include/                    # Header files
│   ├── color_output.h          # Color rendering functions
│   ├── image_processing.h      # Image processing functions
│   └── stb_image.h             # Image loading library
└── src/                        # Source code
    ├── color_output.c          # ANSI color implementation
    ├── image_processing.c      # Image processing implementation
    └── main.c                  # CLI and main logic
```

---

## Learning Goals

TermiView aims to serve as a **hands-on toolkit** for exploring how computers *see* and *process* images - through both **spatial** and **frequency domain** techniques.

Planned educational modules:

* Convolution & Filters
* Image Quantization & Sampling
* Frequency-Domain Filtering (FFT/IFFT)
* Edge Detection
* Histogram Equalization

---

## Roadmap

* [x] ASCII renderer with brightness-based mapping
* [x] Colorized terminal support (16, 256, truecolor)
* [x] RGB image processing
* [x] Professional CLI with getopt
* [x] Error handling and memory management
* [x] Build system with install/uninstall
* [ ] Kernel-based filtering
* [ ] FFT-based visualization
* [ ] Histogram visualization
* [ ] Real-time video / camera input support
* [ ] Plugin system for custom filters

---

## Version History

### v0.2.0 - Phase 1 Complete (Current)
- ✅ Full colorized output support (16, 256, truecolor)
- ✅ RGB image processing with separate channels
- ✅ Enhanced CLI with getopt argument parsing
- ✅ Comprehensive error handling and memory management
- ✅ Professional build system with install targets
- ✅ File output support

### v0.1.0 - Initial Release
- ✅ Basic ASCII renderer with grayscale

---

## Contributing

Contributions are welcome! If you're learning signal processing or graphics programming, this is a great place to experiment and contribute.

```bash
git checkout -b feature/your-feature
git commit -m "Add new filter or feature"
git push origin feature/your-feature
```

---

## License

This project is open source. Feel free to use, modify, and distribute.

---

## Author

Created by [pro-utkarshM](https://github.com/pro-utkarshM)

**TermiView** - See the world in ASCII, one character at a time. 🎨
