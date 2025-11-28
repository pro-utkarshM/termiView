# TermiView

> A comprehensive command-line toolkit for learning and implementing image and video processing algorithms - from fundamentals to advanced techniques.

---

## Overview

**TermiView** is an educational CLI toolkit built for students, researchers, and developers who want to learn and implement image and video processing algorithms from the ground up.

Starting with **colorized ASCII art rendering**, TermiView is evolving into a **complete implementation** of image and video processing fundamentals, covering everything from basic transforms to advanced compression and motion estimation - all with practical, hands-on code you can study and modify.

---

## Features

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
* **FFTW** – For Fourier transforms

---

For detailed usage instructions and examples, please refer to [USAGE.md](USAGE.md).

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


> For the goals of this project, please refer to [GOALS.md](GOALS.md)

> For the detailed project roadmap, please refer to [TODO.md](TODO.md).

> For detailed version history, please refer to [VERSION.md](VERSION.md).

---
## Contributing

Contributions are welcome! If you're learning signal processing or graphics programming, this is a great place to experiment and contribute.

```bash
git checkout -b feature/your-feature
git commit -m "Add new filter or feature"
git push origin feature/your-feature
```

---

## Author

Created by [pro-utkarshM](https://github.com/pro-utkarshM)

**TermiView** - See the world in ASCII, one character at a time. 🎨
