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
  -f, --filter <type>    Apply filter: blur, sharpen, sobel, laplacian, salt-pepper, ideal-lowpass, ideal-highpass, gaussian-lowpass, gaussian-highpass (default: none)
  -q, --quantize <n>     Number of grayscale quantization levels (2-256)
  --cutoff <value>     Cutoff frequency for frequency domain filters (e.g., 20.0)
  -N, --noise <density>  Apply salt-and-pepper noise (density: 0.0-1.0)
  -F, --dft              Compute and display the 2D DFT magnitude spectrum
  -D, --dct              Compute and display the 2D DCT magnitude spectrum
  -W, --dwt              Compute and display the 2D DWT magnitude spectrum
  -E, --equalize         Apply histogram equalization to grayscale images
  -v, --version          Show version information
  --help                 Show this help message
  --compress <type>      Compress input file using specified algorithm: lzw, huffman, arithmetic, rle, dct_based, wavelet
  --decompress <type>    Decompress input file using specified algorithm: lzw, huffman, arithmetic, rle, dct_based, wavelet
  --wavelet-levels <num> Number of decomposition levels for wavelet transform (default: 1)
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

**Compress a file using LZW:**
```bash
termiView --compress lzw input.txt -o output.lzw
```

**Decompress a file using LZW:**
```bash
termiView --decompress lzw input.lzw -o output.txt
```

**Compress a file using RLE:**
```bash
termiView --compress rle input.bin -o output.rle
```

**Decompress a file using RLE:**
```bash
termiView --decompress rle input.rle -o output.bin
```

**Compress an image using DCT-based compression:**
```bash
termiView --compress dct_based input.png -o output.dct
```

**Decompress an image using DCT-based compression:**
```bash
termiView --decompress dct_based input.dct -o output.png
```

**Compress an image using Wavelet compression:**
```bash
termiView --compress wavelet --wavelet-levels 2 input.png -o output.wlt
```

**Decompress an image using Wavelet compression:**
```bash
termiView --decompress wavelet --wavelet-levels 2 input.wlt -o output.png
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

**Apply histogram equalization:**
```bash
termiView --equalize image.jpg
```

**Apply a Gaussian low-pass filter:**
```bash
termiView --filter gaussian-lowpass --cutoff 15 image.jpg
```

**Laplacian edge detection in grayscale:**
```bash
termiView -f laplacian --color none image.png
```