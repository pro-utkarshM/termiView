# TermiView

> A command-line tool that displays images as colorized ASCII art in your terminal - and a playground for experimenting with image signal processing.

---

## Overview

**TermiView** is a lightweight CLI tool built for developers, artists, and learners who want to explore how images can be represented, transformed, and visualized - all inside a terminal.

Originally designed to render **colorized ASCII art**, TermiView is evolving into a **visual lab** for learning and experimenting with **signal processing techniques** applied to images.

---

## Features

- **ASCII Renderer** – Convert images into ASCII art with adjustable resolution and color modes (16-color / 256-color ANSI).
- **Colorized Output** – Retain color tones using terminal-compatible escape sequences.
- **Image Filters** – Apply kernels like blur, sharpen, or edge-detection (Sobel, Laplacian, etc.).
- **Frequency Analysis** – View Fourier transform magnitude & phase as ASCII plots.
- **Histogram Visualization** – Display intensity or color histograms right in the terminal.
- **Animation Support** – Step through filter applications frame by frame.
- **Educational Purpose** – Learn core concepts of image processing, convolution, and frequency analysis interactively.

---

## Installation

```bash
git clone https://github.com/pro-utkarshM/termiview.git
cd termiview
make
sudo make install
````

### Dependencies

* GCC / Clang
* libpng, libjpeg (for image loading)
* ncurses (for terminal control)
* FFTW (for Fourier transforms) *(optional, for advanced features)*

---

## Usage

Convert and view an image as ASCII:

```bash
termiview image.jpg
```

Apply filters or transformations:

```bash
termiview image.png --filter sobel --color
termiview image.png --fft
termiview image.png --histogram
```

Save output to a text file:

```bash
termiview image.jpg --output out.txt
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
* [ ] Colorized terminal support
* [ ] Kernel-based filtering
* [ ] FFT-based visualization
* [ ] Real-time video / camera input support
* [ ] Plugin system for custom filters

---

## Contributing

Contributions are welcome!
If you’re learning signal processing or graphics programming, this is a great place to experiment and contribute.

```bash
git checkout -b feature/your-feature
git commit -m "Add new filter or feature"
git push origin feature/your-feature
```

---

**Made with ❤️ and `printf()` by [Utkarsh Maurya](https://github.com/pro-utkarshM).**
