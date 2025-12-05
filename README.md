# TermiView

> A command-line toolkit for learning and implementing image and video processing algorithms — from fundamentals to advanced techniques.

---

## Contents

* [Overview](#overview)
* [What It Does](#what-it-does)
* [Features](#features)
* [Installation](#installation)
* [Development](#development)
* [Roadmap](#roadmap)
* [Contributing](#contributing)
* [Author](#author)

---

## Overview

**TermiView** is an open-source **command-line toolkit** that transforms your terminal into a complete lab for **image and video processing**.

It helps you learn how digital images and videos are processed at the algorithmic level — by letting you build, visualize, and experiment with actual implementations in **C**.

Starting with **colorized ASCII rendering**, TermiView expands into a suite that demonstrates:

* Core image processing fundamentals
* Frequency-domain transforms (DFT, DCT, DWT)
* Compression algorithms (Huffman, JPEG, wavelet)
* Motion estimation and video coding concepts

Every module is implemented transparently, making it ideal for study, experimentation, and learning.

---

## What It Does

* Converts images into ASCII art with configurable color modes (grayscale to truecolor).
* Applies convolution-based filters such as Gaussian blur, sharpen, and edge detection.
* Demonstrates foundational topics like sampling, quantization, and transforms.
* Provides implementations of compression algorithms and motion analysis for video.

In short, TermiView bridges **theory and code**, offering a hands-on way to understand how visual data is processed at every stage.

---

## Features

### Core Capabilities

* **ASCII Renderer:** Brightness-based mapping with color output (16, 256, truecolor).
* **Image Filtering:** Kernel-based convolution with multiple filters.
* **Transforms:** DFT, DCT, and DWT for frequency-domain analysis.
* **Compression:** Implementations of Huffman, LZW, and JPEG.
* **Video Basics:** Frame extraction, temporal filtering, and motion modeling.

### CLI and Build

* Modern command-line interface using `getopt`.
* Comprehensive error handling and memory safety.
* Modular Makefile with `install`, `uninstall`, and `test` targets.

---

## Installation

```bash
git clone https://github.com/pro-utkarshM/termiview.git
cd termiview
make
sudo make install
```

**Dependencies**

* `gcc` or `clang` — compiler
* `stb_image.h` — image loader (included)
* `fftw` — for Fourier transforms

---

## Development

```bash
make           # Build release version
make debug     # Build with debug flags
make test      # Run tests
make clean     # Clean build artifacts
```

---

## Roadmap

### Foundation

* [x] ASCII renderer with brightness mapping
* [x] Colorized terminal support (16, 256, Truecolor)
* [x] RGB image processing and CLI integration
* [x] Kernel-based filtering (blur, sharpen, edge detection)
* [x] Convolution engine and Makefile system

### Fundamentals

* [x] Sampling and quantization demonstrations
* [x] Pixel relationship operations
* [x] 2D DFT, DCT, and DWT implementations

### Processing Techniques

* [x] Histogram equalization and processing
* [x] Complete spatial and frequency-domain filtering
* [x] Edge detection (Prewitt, Roberts, Canny)
* [x] Thresholding (Otsu, adaptive)
* [x] Region-based segmentation

### Compression

* [x] Huffman, Arithmetic, LZW, and Run-length encoding
* [x] Transform coding (DCT-based)
* [x] JPEG encoder/decoder
* [x] Wavelet-based compression

### Video Processing

* [x] Video file I/O and frame extraction
* [x] Temporal filtering and motion models
* [x] Video sampling demonstrations

### Motion & Video Coding

* [x] Optical flow computation
* [ ] Block matching algorithms
* [ ] Motion estimation and predictive coding
* [ ] Motion-compensated video compression

---

## Contributing

Contributions are welcome.
If you’re exploring **image processing**, **signal processing**, or **graphics programming**, TermiView provides a clean foundation to learn and contribute.

```bash
git checkout -b feature/your-feature
git commit -m "Add new filter or feature"
git push origin feature/your-feature
```

---

## Author

Created by [pro-utkarshM](https://github.com/pro-utkarshM)
“See the world in ASCII — one character at a time.”

---
