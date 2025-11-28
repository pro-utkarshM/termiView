# TermiView

> A comprehensive command-line toolkit for learning and implementing image and video processing algorithms - from fundamentals to advanced techniques.

---

## Overview

**TermiView** is an educational CLI toolkit built for students, researchers, and developers who want to learn and implement image and video processing algorithms from the ground up.

Starting with **colorized ASCII art rendering**, TermiView is evolving into a **complete implementation** of image and video processing fundamentals, covering everything from basic transforms to advanced compression and motion estimation - all with practical, hands-on code you can study and modify.

---

## Features

### ✅ Implemented

#### Phase 1: Foundation & Color (Complete)
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

#### Phase 2: Image Filtering (In Progress)
- **Kernel-Based Convolution** – Apply convolution operations to images
- **Gaussian Blur Filter** – Smooth images with configurable kernel size and sigma
- **Sharpen Filter** – Enhance edges and details in images
- **Sobel Edge Detection** – Gradient magnitude edge detection
- **Prewitt Edge Detection** – Gradient-based edge detection
- **Roberts Cross Edge Detection** – Diagonal edge detection
- **Laplacian Edge Detection** – Omnidirectional edge detection
- **Filter Integration** – Seamlessly apply filters before ASCII conversion

### 🚧 Planned (Curriculum-Based Roadmap)

#### Unit 1: Fundamentals of Image Processing
- **Sampling and Quantization** – Demonstrate aliasing, resolution effects
- **Basic Pixel Relationships** – Neighbors, connectivity, distance measures
- **2D Discrete Fourier Transform (DFT)** – Frequency domain visualization
- **Discrete Cosine Transform (DCT)** – JPEG foundation
- **Discrete Wavelet Transform (DWT)** – Multi-resolution analysis

#### Unit 2: Image Processing Techniques

**Spatial Domain Enhancement:**
- **Histogram Processing** – Equalization, matching, specification
- **Spatial Filtering Fundamentals** – Convolution, correlation
- **Smoothing Filters** – Mean, Gaussian, median, bilateral
- **Sharpening Filters** – Laplacian, unsharp masking, high-boost

**Frequency Domain Enhancement:**
- **Frequency Domain Filtering** – Ideal, Butterworth, Gaussian filters
- **Image Smoothing** – Low-pass filtering
- **Image Sharpening** – High-pass filtering, high-frequency emphasis
- **Selective Filtering** – Band-pass, band-reject, notch filters

**Image Segmentation:**
- **Point Detection** – Isolated point detection
- **Line Detection** – Horizontal, vertical, diagonal lines
- **Edge Detection** – Prewitt, Roberts, Canny, Marr-Hildreth
- **Thresholding** – Global, adaptive, Otsu's method
- **Region-Based Segmentation** – Region growing, split-and-merge

#### Unit 3: Image Compression

**Compression Fundamentals:**
- **Coding Redundancy** – Variable-length coding
- **Spatial Redundancy** – Interpixel correlation
- **Temporal Redundancy** – Frame-to-frame correlation

**Compression Techniques:**
- **Huffman Coding** – Optimal prefix codes
- **Arithmetic Coding** – Fractional bit encoding
- **LZW Coding** – Dictionary-based compression
- **Run-Length Encoding** – Sequential data compression
- **Bit-Plane Coding** – Bit-level compression
- **Transform Coding** – DCT-based compression
- **Predictive Coding** – DPCM, delta modulation
- **Wavelet Coding** – Multi-resolution compression
- **JPEG Standard** – Complete JPEG encoder/decoder

#### Unit 4: Video Processing Fundamentals

**Video Basics:**
- **Analog vs Digital Video** – Conversion and representation
- **Time-Varying Image Formation** – 3D motion models
- **Geometric Image Formation** – Camera models, projection
- **Photometric Image Formation** – Lighting, shading, reflectance
- **Video Signal Sampling** – Temporal and spatial sampling
- **Video Filtering Operations** – Temporal filtering, noise reduction

#### Unit 5: Motion Estimation & Video Coding

**Motion Estimation:**
- **Optical Flow** – Dense motion field estimation
- **Pixel-Based Motion Estimation** – Differential methods
- **Block Matching Algorithm** – Full search, fast algorithms (TSS, 3SS, DS)
- **Mesh-Based Motion Estimation** – Triangular mesh warping
- **Global Motion Estimation** – Camera motion, affine models
- **Region-Based Motion Estimation** – Object-level motion
- **Multi-Resolution Motion Estimation** – Hierarchical approaches

**Video Coding:**
- **Waveform-Based Coding** – PCM, DPCM
- **Block-Based Transform Coding** – DCT for video
- **Predictive Coding** – Inter-frame prediction, motion compensation
- **Motion Estimation in Video Coding** – H.264/AVC, HEVC concepts

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
  -f, --filter <type>    Apply filter: blur, sharpen, sobel, prewitt, roberts, laplacian, salt-pepper (default: none)
  -q, --quantize <n>     Number of grayscale quantization levels (2-256)
  -N, --noise <density>  Apply salt-and-pepper noise (density: 0.0-1.0)
  -F, --dft              Compute and display the 2D DFT magnitude spectrum
  -D, --dct              Compute and display the 2D DCT magnitude spectrum
  -W, --dwt              Compute and display the 2D DWT magnitude spectrum
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

**Apply Gaussian blur filter:**
```bash
termiView --filter blur image.jpg
```

**Sharpen image before ASCII conversion:**
```bash
termiView -f sharpen --color 256 photo.png
```

**Edge detection with Sobel operator:**
```bash
termiView --filter sobel image.jpg -w 80 -h 60
```

**Edge detection with Prewitt operator:**
```bash
termiView --filter prewitt image.jpg -w 80 -h 60
```

**Edge detection with Roberts Cross operator:**
```bash
termiView --filter roberts image.jpg -w 80 -h 60
```

**Apply salt-and-pepper noise:**
```bash
termiView --filter salt-pepper --noise 0.1 image.jpg
```

**Compute the 2D DCT of an image:**
```bash
termiView --dct image.jpg
```

**Compute the 2D DWT of an image:**
```bash
termiView --dwt image.jpg
```

**Quantize a grayscale image:**
```bash
termiView --quantize 8 image.jpg
```

**Laplacian edge detection in grayscale:**
```bash
termiView -f laplacian --color none image.png
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
│   ├── filters.h               # Filter and convolution functions
│   ├── image_processing.h      # Image processing functions
│   └── stb_image.h             # Image loading library
└── src/                        # Source code
    ├── color_output.c          # ANSI color implementation
    ├── filters.c               # Convolution and filter implementation
    ├── image_processing.c      # Image processing implementation
    └── main.c                  # CLI and main logic
```

---

## Learning Goals

TermiView aims to serve as a **complete educational implementation** of image and video processing fundamentals, covering a full university-level curriculum in computer vision and multimedia processing.

**Educational Coverage:**

**Unit 1 - Fundamentals:**
- Image representation and digitization
- Sampling theory and quantization effects
- Transform domain analysis (DFT, DCT, DWT)
- Frequency domain concepts

**Unit 2 - Processing Techniques:**
- Spatial domain enhancement (histogram, filtering)
- Frequency domain enhancement (spectral filtering)
- Image segmentation (edges, regions, thresholding)
- Feature detection and extraction

**Unit 3 - Compression:**
- Information theory and redundancy
- Lossless compression (Huffman, LZW, RLE)
- Lossy compression (transform, predictive, wavelet)
- JPEG standard implementation

**Unit 4 - Video Fundamentals:**
- Video signal representation
- Motion models and image formation
- Temporal processing and filtering
- Video sampling and reconstruction

**Unit 5 - Motion & Video Coding:**
- Motion estimation algorithms
- Optical flow computation
- Video compression techniques
- Modern video coding standards

**Learning Approach:**
- ✅ **Practical Implementation** - Working code for every algorithm
- ✅ **Visual Feedback** - ASCII/terminal visualization of results
- ✅ **Modular Design** - Study individual components independently
- ✅ **Educational Comments** - Well-documented code explaining theory
- ✅ **Progressive Complexity** - Build from basics to advanced topics

---

## Roadmap

**Foundation (Complete):**
* [x] ASCII renderer with brightness-based mapping
* [x] Colorized terminal support (16, 256, truecolor)
* [x] RGB image processing
* [x] Professional CLI with getopt
* [x] Error handling and memory management
* [x] Build system with install/uninstall
* [x] Kernel-based filtering (blur, sharpen, edge detection)
* [x] Convolution engine for image processing

**Unit 1 - Fundamentals:**
* [x] Sampling and quantization demonstrations
* [x] Pixel relationship operations
* [x] 2D DFT implementation and visualization
* [x] DCT implementation
* [x] DWT implementation

**Unit 2 - Processing Techniques:**
* [ ] Histogram equalization and processing
* [x] Complete spatial filter library
* [ ] Frequency domain filtering
* [x] Advanced edge detection (Prewitt, Roberts)
* [ ] Advanced edge detection (Canny)
* [ ] Thresholding methods (Otsu, adaptive)
* [ ] Region-based segmentation

**Unit 3 - Compression:**
* [ ] Huffman coding
* [ ] Arithmetic coding
* [ ] LZW compression
* [ ] Run-length encoding
* [ ] Transform coding (DCT-based)
* [ ] JPEG encoder/decoder
* [ ] Wavelet-based compression

**Unit 4 - Video Processing:**
* [ ] Video file I/O
* [ ] Frame extraction and processing
* [ ] Temporal filtering
* [ ] Motion models
* [ ] Video sampling demonstrations

**Unit 5 - Motion & Video Coding:**
* [ ] Optical flow computation
* [ ] Block matching algorithms
* [ ] Motion estimation techniques
* [ ] Video compression (predictive coding)
* [ ] Motion-compensated prediction

---

## Version History

### v0.6.0 - Unit 1 Complete: Transforms (Current)
- ✅ 2D Discrete Cosine Transform (DCT)
- ✅ 2D Discrete Wavelet Transform (DWT)
- ✅ Unit tests for all transforms

### v0.5.0 - Phase 2: Quantization & Noise
- ✅ Grayscale quantization (`--quantize`)
- ✅ Salt-and-pepper noise filter (`--noise`)
- ✅ Unit testing framework and initial tests

### v0.4.0 - Phase 2: Advanced Edge Detection
- ✅ Sobel edge detection (gradient magnitude)
- ✅ Prewitt edge detection filter
- ✅ Roberts Cross edge detection filter

### v0.3.0 - Phase 2: Image Filtering
- ✅ Kernel-based convolution engine for grayscale and RGB images
- ✅ Gaussian blur filter with configurable parameters
- ✅ Sharpen filter for edge enhancement
- ✅ Initial Sobel edge detection (horizontal)
- ✅ Laplacian edge detection (omnidirectional)
- ✅ Filter CLI option (`--filter` / `-f`)
- ✅ Seamless integration with existing color modes

### v0.2.0 - Phase 1: Foundation & Color
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
