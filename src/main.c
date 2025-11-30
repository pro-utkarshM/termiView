#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "../include/image_processing.h"
#include "../include/color_output.h"
#include "../include/filters.h"
#include "../include/frequency.h"
#include "../include/compression.h"

typedef enum {
    COMPRESSION_NONE,
    COMPRESSION_LZW,
    COMPRESSION_HUFFMAN,
    COMPRESSION_ARITHMETIC,
    COMPRESSION_RLE,
    COMPRESSION_DCT_BASED,
    COMPRESSION_JPEG,
    COMPRESSION_WAVELET
} compression_type_t;

#define VERSION "0.3.0"
#define DEFAULT_MAX_WIDTH 64
#define DEFAULT_MAX_HEIGHT 48

void print_usage(const char* program_name) {
    printf("TermiView v%s - Display images as colorized ASCII art in your terminal\n\n", VERSION);
    printf("Usage: %s [OPTIONS] <image_path>\n\n", program_name);
    printf("Options:\n");
    printf("  -w, --width <num>      Maximum width in characters (default: %d)\n", DEFAULT_MAX_WIDTH);
    printf("  -h, --height <num>     Maximum height in characters (default: %d)\n", DEFAULT_MAX_HEIGHT);
    printf("  -c, --color <mode>     Color mode: none, 16, 256, truecolor (default: truecolor)\n");
    printf("  -L, --levels <n>       Number of quantization levels per channel (2-256, for truecolor mode)\n");
    printf("  -q, --quantize <n>     Number of grayscale quantization levels (2-256)\n");
    printf("  -i, --interpolation <m> Interpolation method: nearest, average (default: average)\n");
    printf("  -C, --connectivity <t> Find connected components (4 or 8 connectivity)\n");
    printf("  -F, --dft              Compute and display the 2D DFT magnitude spectrum\n");
    printf("  -D, --dct              Compute and display the 2D DCT magnitude spectrum\n");
    printf("  -W, --dwt              Compute and display the 2D DWT magnitude spectrum\n");
    printf("  -E, --equalize         Apply histogram equalization to grayscale images\n");
    printf("  -d, --dark             Use dark mode (default)\n");
    printf("  -l, --light            Use light mode\n");
    printf("  -o, --output <file>    Save output to file instead of stdout\n");
    printf("  -f, --filter <type>    Apply filter: blur, sharpen, sobel, laplacian, salt-pepper, ideal-lowpass, ideal-highpass, gaussian-lowpass, gaussian-highpass (default: none)\n");
    printf("  -N, --noise <density>  Apply salt-and-pepper noise (density: 0.0-1.0)\n");
    printf("  --cutoff <value>     Cutoff frequency for frequency domain filters (e.g., 20.0)\n");
    printf("  -v, --version          Show version information\n");
    printf("  --help                 Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s image.jpg\n", program_name);
    printf("  %s -w 80 -h 60 --color 256 image.png\n", program_name);
    printf("  %s --light --color 16 photo.jpeg\n", program_name);
    printf("  %s -c truecolor -o output.txt image.jpg\n", program_name);
    printf("  %s --filter blur image.jpg\n", program_name);
    printf("  %s -f sharpen --color 256 image.png\n\n", program_name);
}

void print_version() {
    printf("TermiView version %s\n", VERSION);
    printf("A command-line tool for displaying images as colorized ASCII art\n");
}

color_mode_t parse_color_mode(const char* mode_str) {
    if (strcmp(mode_str, "none") == 0) {
        return COLOR_MODE_NONE;
    } else if (strcmp(mode_str, "16") == 0) {
        return COLOR_MODE_16;
    } else if (strcmp(mode_str, "256") == 0) {
        return COLOR_MODE_256;
    } else if (strcmp(mode_str, "truecolor") == 0 || strcmp(mode_str, "true") == 0) {
        return COLOR_MODE_TRUECOLOR;
    } else {
        fprintf(stderr, "Warning: Unknown color mode '%s', using truecolor\n", mode_str);
        return COLOR_MODE_TRUECOLOR;
    }
}

int main(int argc, char* argv[]) {
    // Default values
    size_t max_width = DEFAULT_MAX_WIDTH;
    size_t max_height = DEFAULT_MAX_HEIGHT;
    bool dark_mode = true;
    color_mode_t color_mode = COLOR_MODE_TRUECOLOR;
    char* output_file = NULL;
    char* input_file = NULL;
    filter_type_t filter_type = FILTER_NONE;
    int quantization_levels = 256;
    int grayscale_quantization_levels = 0;
    interpolation_method_t interpolation_method = INTERPOLATION_AVERAGE;
    int connectivity = 0;
    bool dft_mode = false;
    bool dct_mode = false;
    bool dwt_mode = false;
    bool equalize_mode = false;
    float noise_density = 0.0f;
    double cutoff = 20.0; // Default cutoff frequency
    compression_type_t compression_type = COMPRESSION_NONE;
    bool decompress_mode = false;
    int wavelet_levels = 1; // Default wavelet decomposition levels

    // Long options
    static struct option long_options[] = {
        {"width",   required_argument, 0, 'w'},
        {"height",  required_argument, 0, 'h'},
        {"color",   required_argument, 0, 'c'},
        {"levels",  required_argument, 0, 'L'},
        {"quantize", required_argument, 0, 'q'},
        {"interpolation", required_argument, 0, 'i'},
        {"connectivity", required_argument, 0, 'C'},
        {"dft",     no_argument,       0, 'F'},
        {"dct",     no_argument,       0, 'D'},
        {"dwt",     no_argument,       0, 'W'},
        {"equalize", no_argument,      0, 'E'},
        {"dark",    no_argument,       0, 'd'},
        {"light",   no_argument,       0, 'l'},
        {"output",  required_argument, 0, 'o'},
        {"filter",  required_argument, 0, 'f'},
        {"noise",   required_argument, 0, 'N'},
        {"version", no_argument,       0, 'v'},
        {"help",    no_argument,       0,  0 },
        {"cutoff",  required_argument, 0,  1 },
        {"compress", required_argument, 0, 2},
        {"decompress", required_argument, 0, 3},
        {"wavelet-levels", required_argument, 0, 4},
        {0, 0, 0, 0}
    };

    // Parse options
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "w:h:c:L:q:i:C:FdDWElo:f:N:v", long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                // Long option with no short equivalent
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    print_usage(argv[0]);
                    return 0;
                }
                break;
            case 1:
                if (strcmp(long_options[option_index].name, "cutoff") == 0) {
                    cutoff = atof(optarg);
                }
                break;
            case 2:
                if(strcmp(optarg, "lzw") == 0) {
                    compression_type = COMPRESSION_LZW;
                } else if (strcmp(optarg, "huffman") == 0) {
                    compression_type = COMPRESSION_HUFFMAN;
                } else if (strcmp(optarg, "arithmetic") == 0) {
                    compression_type = COMPRESSION_ARITHMETIC;
                } else if (strcmp(optarg, "rle") == 0) {
                    compression_type = COMPRESSION_RLE;
                } else if (strcmp(optarg, "dct_based") == 0) {
                    compression_type = COMPRESSION_DCT_BASED;
                } else if (strcmp(optarg, "jpeg") == 0) {
                    compression_type = COMPRESSION_JPEG;
                } else if (strcmp(optarg, "wavelet") == 0) {
                    compression_type = COMPRESSION_WAVELET;
                } else {
                    fprintf(stderr, "Error: Unknown compression type '%s'\\n", optarg);
                    return 1;
                }
                break;
            case 3:
                decompress_mode = true;
                if(strcmp(optarg, "lzw") == 0) {
                    compression_type = COMPRESSION_LZW;
                } else if (strcmp(optarg, "huffman") == 0) {
                    compression_type = COMPRESSION_HUFFMAN;
                } else if (strcmp(optarg, "arithmetic") == 0) {
                    compression_type = COMPRESSION_ARITHMETIC;
                } else if (strcmp(optarg, "rle") == 0) {
                    compression_type = COMPRESSION_RLE;
                } else if (strcmp(optarg, "dct_based") == 0) {
                    compression_type = COMPRESSION_DCT_BASED;
                } else if (strcmp(optarg, "jpeg") == 0) {
                    compression_type = COMPRESSION_JPEG;
                } else if (strcmp(optarg, "wavelet") == 0) {
                    compression_type = COMPRESSION_WAVELET;
                } else {
                    fprintf(stderr, "Error: Unknown compression type '%s'\\n", optarg);
                    return 1;
                }
                break;
            case 4:
                wavelet_levels = atoi(optarg);
                if (wavelet_levels < 1) {
                    fprintf(stderr, "Error: Wavelet levels must be at least 1\n");
                    return 1;
                }
                break;
            case 'w':
                max_width = (size_t) atoi(optarg);
                if (max_width == 0) {
                    fprintf(stderr, "Error: Invalid width value\\n");
                    return 1;
                }
                break;
            case 'h':
                max_height = (size_t) atoi(optarg);
                if (max_height == 0) {
                    fprintf(stderr, "Error: Invalid height value\n");
                    return 1;
                }
                break;
            case 'c':
                color_mode = parse_color_mode(optarg);
                break;
            case 'L':
                quantization_levels = atoi(optarg);
                if (quantization_levels < 2 || quantization_levels > 256) {
                    fprintf(stderr, "Error: Levels must be between 2 and 256\n");
                    return 1;
                }
                break;
            case 'q':
                grayscale_quantization_levels = atoi(optarg);
                if (grayscale_quantization_levels < 2 || grayscale_quantization_levels > 256) {
                    fprintf(stderr, "Error: Grayscale quantization levels must be between 2 and 256\n");
                    return 1;
                }
                break;
            case 'i':
                if (strcmp(optarg, "nearest") == 0) {
                    interpolation_method = INTERPOLATION_NEAREST;
                } else if (strcmp(optarg, "average") == 0) {
                    interpolation_method = INTERPOLATION_AVERAGE;
                } else {
                    fprintf(stderr, "Error: Unknown interpolation method '%s'\n", optarg);
                    return 1;
                }
                break;
            case 'C':
                connectivity = atoi(optarg);
                if (connectivity != 4 && connectivity != 8) {
                    fprintf(stderr, "Error: Connectivity must be 4 or 8\n");
                    return 1;
                }
                break;
            case 'F':
                dft_mode = true;
                break;
            case 'D':
                dct_mode = true;
                break;
            case 'W':
                dwt_mode = true;
                break;
            case 'E':
                equalize_mode = true;
                break;
            case 'd':
                dark_mode = true;
                break;
            case 'l':
                dark_mode = false;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'f':
                filter_type = parse_filter_type(optarg);
                break;
            case 'N':
                noise_density = atof(optarg);
                if (noise_density < 0.0f || noise_density > 1.0f) {
                    fprintf(stderr, "Error: Noise density must be between 0.0 and 1.0\n");
                    return 1;
                }
                break;
            case 'v':
                print_version();
                return 0;
            case '?':
                // getopt_long already printed an error message
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return 1;
            default:
                return 1;
        }
    }

    // Get input file (remaining argument)
    if (optind < argc) {
        input_file = argv[optind];
    } else {
        fprintf(stderr, "Error: No input image specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }

    if (compression_type != COMPRESSION_NONE) {
        if (!input_file || !output_file) {
            fprintf(stderr, "Error: Both input and output files must be specified for compression/decompression.\n");
            return 1;
        }

        FILE* in = fopen(input_file, "rb");
        if (!in) {
            fprintf(stderr, "Error: Cannot open input file '%s'.\n", input_file);
            return 1;
        }

        fseek(in, 0, SEEK_END);
        size_t input_len = ftell(in);
        fseek(in, 0, SEEK_SET);
        unsigned char* input_data = (unsigned char*)malloc(input_len);
        fread(input_data, 1, input_len, in);
        fclose(in);

        unsigned char* output_data = NULL;
        size_t output_len = 0;

        if (decompress_mode) {
            switch (compression_type) {
                case COMPRESSION_LZW:
                    output_data = lzw_decode(input_data, input_len, &output_len);
                    break;
                case COMPRESSION_HUFFMAN:
                    // Placeholder for Huffman
                    break;
                case COMPRESSION_ARITHMETIC:
                    // Placeholder for Arithmetic
                    break;
                case COMPRESSION_RLE:
                    output_data = rle_decode(input_data, input_len, &output_len);
                    break;
                case COMPRESSION_DCT_BASED:
                    {
                        int original_width, original_height;
                        memcpy(&original_width, input_data, sizeof(int));
                        memcpy(&original_height, input_data + sizeof(int), sizeof(int));
                        
                        grayscale_image_t* decoded_image = dct_based_decode(input_data, input_len, original_width, original_height);
                        if (decoded_image) {
                            output_len = decoded_image->width * decoded_image->height;
                            output_data = (unsigned char*)malloc(output_len);
                            if (output_data) {
                                memcpy(output_data, decoded_image->data, output_len);
                            }
                            free_grayscale_image(decoded_image);
                            free(decoded_image);
                        }
                    }
                    break;
                case COMPRESSION_WAVELET:
                    {
                        int original_width, original_height, levels;
                        size_t current_decoded_idx = 0;
                        memcpy(&original_width, input_data + current_decoded_idx, sizeof(int));
                        current_decoded_idx += sizeof(int);
                        memcpy(&original_height, input_data + current_decoded_idx, sizeof(int));
                        current_decoded_idx += sizeof(int);
                        memcpy(&levels, input_data + current_decoded_idx, sizeof(int));
                        current_decoded_idx += sizeof(int);

                        grayscale_image_t* decoded_image = wavelet_decode(input_data, input_len, original_width, original_height, wavelet_levels);
                        if (decoded_image) {
                            output_len = decoded_image->width * decoded_image->height;
                            output_data = (unsigned char*)malloc(output_len);
                            if (output_data) {
                                memcpy(output_data, decoded_image->data, output_len);
                            }
                            free_grayscale_image(decoded_image);
                            free(decoded_image);
                        }
                    }
                    break;
                default:
                    break;
            }
        } else {
            switch (compression_type) {
                case COMPRESSION_LZW:
                    output_data = lzw_encode(input_data, input_len, &output_len);
                    break;
                case COMPRESSION_HUFFMAN:
                    // Placeholder for Huffman
                    break;
                case COMPRESSION_ARITHMETIC:
                    // Placeholder for Arithmetic
                    break;
                case COMPRESSION_RLE:
                    output_data = rle_encode(input_data, input_len, &output_len);
                    break;
                case COMPRESSION_DCT_BASED:
                    {
                        grayscale_image_t original_image = load_image_as_grayscale(input_file);
                        if (original_image.data != NULL) {
                            output_data = dct_based_encode(&original_image, &output_len);
                            free_grayscale_image(&original_image);
                        }
                    }
                    break;
                case COMPRESSION_JPEG:
                    {
                        grayscale_image_t original_image = load_image_as_grayscale(input_file);
                        if (original_image.data != NULL) {
                            output_data = jpeg_encode(&original_image, 50, &output_len); // Default quality 50
                            free_grayscale_image(&original_image);
                        }
                    }
                    break;
                case COMPRESSION_WAVELET:
                    {
                        grayscale_image_t original_image = load_image_as_grayscale(input_file);
                        if (original_image.data != NULL) {
                            output_data = wavelet_encode(&original_image, wavelet_levels, &output_len); // Use wavelet_levels
                            free_grayscale_image(&original_image);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        if (output_data) {
            FILE* out = fopen(output_file, "wb");
            if (!out) {
                fprintf(stderr, "Error: Cannot open output file '%s'.\n", output_file);
                free(input_data);
                free(output_data);
                return 1;
            }
            fwrite(output_data, 1, output_len, out);
            fclose(out);
            free(output_data);
        }

        free(input_data);
        return 0;
    }

    // Handle connectivity mode separately
    if (connectivity > 0) {
        grayscale_image_t gray_original = load_image_as_grayscale(input_file);
        if (gray_original.data == NULL) {
            // Try loading as RGB and converting
            rgb_image_t rgb_original = load_image_as_rgb(input_file);
            if (rgb_original.r_data == NULL) {
                return 1; // Already printed error
            }
            gray_original = rgb_to_grayscale(&rgb_original);
            free_rgb_image(&rgb_original);
        }

        grayscale_image_t components = connected_components(&gray_original, connectivity);
        free_grayscale_image(&gray_original);

        if (components.data == NULL) {
            return 1;
        }

        grayscale_image_t resized = make_resized_grayscale(&components, max_width, max_height, interpolation_method);
        free(components.data);

        if (resized.data == NULL) {
            return 1;
        }

        if (color_mode == COLOR_MODE_NONE) {
            print_image(&resized, dark_mode);
        } else {
            print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
        }
        free(resized.data);
        return 0;
    }

    if (dft_mode) {
        grayscale_image_t gray_original = load_image_as_grayscale(input_file);
        if (gray_original.data == NULL) {
            // Try loading as RGB and converting
            rgb_image_t rgb_original = load_image_as_rgb(input_file);
            if (rgb_original.r_data == NULL) {
                return 1; // Already printed error
            }
            gray_original = rgb_to_grayscale(&rgb_original);
            free_rgb_image(&rgb_original);
        }

        grayscale_image_t dft_image = dft_grayscale(&gray_original);
        free_grayscale_image(&gray_original);

        if (dft_image.data == NULL) {
            return 1;
        }

        grayscale_image_t resized = make_resized_grayscale(&dft_image, max_width, max_height, interpolation_method);
        free(dft_image.data);

        if (resized.data == NULL) {
            return 1;
        }

        if (color_mode == COLOR_MODE_NONE) {
            print_image(&resized, dark_mode);
        } else {
            print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
        }
        free(resized.data);
        return 0;
    }

    if (dct_mode) {
        grayscale_image_t gray_original = load_image_as_grayscale(input_file);
        if (gray_original.data == NULL) {
            // Try loading as RGB and converting
            rgb_image_t rgb_original = load_image_as_rgb(input_file);
            if (rgb_original.r_data == NULL) {
                return 1; // Already printed error
            }
            gray_original = rgb_to_grayscale(&rgb_original);
            free_rgb_image(&rgb_original);
        }

        grayscale_image_t dct_image = dct_grayscale(&gray_original);
        free_grayscale_image(&gray_original);

        if (dct_image.data == NULL) {
            return 1;
        }

        grayscale_image_t resized = make_resized_grayscale(&dct_image, max_width, max_height, interpolation_method);
        free(dct_image.data);

        if (resized.data == NULL) {
            return 1;
        }

        if (color_mode == COLOR_MODE_NONE) {
            print_image(&resized, dark_mode);
        } else {
            print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
        }
        free(resized.data);
        return 0;
    }

    if (dwt_mode) {
        grayscale_image_t gray_original = load_image_as_grayscale(input_file);
        if (gray_original.data == NULL) {
            // Try loading as RGB and converting
            rgb_image_t rgb_original = load_image_as_rgb(input_file);
            if (rgb_original.r_data == NULL) {
                return 1; // Already printed error
            }
            gray_original = rgb_to_grayscale(&rgb_original);
            free_rgb_image(&rgb_original);
        }

        grayscale_image_t dwt_image = dwt_grayscale(&gray_original);
        free_grayscale_image(&gray_original);

        if (dwt_image.data == NULL) {
            return 1;
        }

        grayscale_image_t resized = make_resized_grayscale(&dwt_image, max_width, max_height, interpolation_method);
        free(dwt_image.data);

        if (resized.data == NULL) {
            return 1;
        }

        if (color_mode == COLOR_MODE_NONE) {
            print_image(&resized, dark_mode);
        } else {
            print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
        }
        free(resized.data);
        return 0;
    }

    // Redirect output to file if specified
    FILE* original_stdout = NULL;
    if (output_file != NULL) {
        original_stdout = stdout;
        stdout = fopen(output_file, "w");
        if (stdout == NULL) {
            fprintf(stderr, "Error: Failed to open output file '%s'\n", output_file);
            stdout = original_stdout;
            return 1;
        }
    }

    // Load and process image
    rgb_image_t rgb_original = load_image_as_rgb(input_file);
    if (rgb_original.r_data == NULL) {
        // Try loading as grayscale
        grayscale_image_t gray_original = load_image_as_grayscale(input_file);
        if (gray_original.data == NULL) {
            if (output_file != NULL) {
                fclose(stdout);
                stdout = original_stdout;
            }
            return 1;
        }

        if (equalize_mode) {
            equalize_histogram(&gray_original);
        }

        // Apply filter if specified
        grayscale_image_t filtered = {0};
        grayscale_image_t* to_resize = &gray_original;
        
        if (filter_type != FILTER_NONE) {
            kernel_t kernel = {0};
            switch (filter_type) {
                case FILTER_BLUR:
                    kernel = create_gaussian_blur_kernel(5, 1.0f);
                    break;
                case FILTER_SHARPEN:
                    kernel = create_sharpen_kernel();
                    break;
                case FILTER_EDGE_SOBEL:
                    filtered = apply_sobel_edge_detection(&gray_original);
                    to_resize = &filtered;
                    break;
                case FILTER_EDGE_PREWITT:
                    filtered = apply_prewitt_edge_detection(&gray_original);
                    to_resize = &filtered;
                    break;
                case FILTER_EDGE_ROBERTS:
                    filtered = apply_roberts_edge_detection(&gray_original);
                    to_resize = &filtered;
                    break;
                case FILTER_EDGE_LAPLACIAN:
                    kernel = create_laplacian_kernel();
                    break;
                case FILTER_SALT_PEPPER:
                    filtered = apply_salt_pepper_noise(&gray_original, noise_density);
                    to_resize = &filtered;
                    break;
                case FILTER_IDEAL_LOWPASS:
                case FILTER_IDEAL_HIGHPASS:
                case FILTER_GAUSSIAN_LOWPASS:
                case FILTER_GAUSSIAN_HIGHPASS:
                    filtered = apply_frequency_filter(&gray_original, filter_type, cutoff);
                    to_resize = &filtered;
                    break;
                default:
                    break;
            }
            
            if (kernel.data != NULL) {
                filtered = apply_convolution_grayscale(&gray_original, &kernel);
                free_kernel(&kernel);
                if (filtered.data != NULL) {
                    to_resize = &filtered;
                }
            } else if ((filter_type == FILTER_EDGE_SOBEL || filter_type == FILTER_SALT_PEPPER) && filtered.data == NULL) {
                // Sobel or Salt-Pepper failed, so we should not proceed
                to_resize = NULL;
            }
        }

        // Process grayscale image
        if (to_resize == NULL) {
            // Error occurred in filtering
            free_grayscale_image(&gray_original);
            if (output_file != NULL) {
                fclose(stdout);
                stdout = original_stdout;
            }
            return 1;
        }
        
        if (grayscale_quantization_levels > 0) {
            quantize_grayscale(to_resize->data, to_resize->width, to_resize->height, grayscale_quantization_levels);
        }
        grayscale_image_t resized = make_resized_grayscale(to_resize, max_width, max_height, interpolation_method);
        if (resized.data == NULL) {
            free_grayscale_image(&gray_original);
            if (filtered.data != NULL) free(filtered.data);
            if (output_file != NULL) {
                fclose(stdout);
                stdout = original_stdout;
            }
            return 1;
        }

        // Print image
        if (color_mode == COLOR_MODE_NONE) {
            print_image(&resized, dark_mode);
        } else {
            print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
        }

        // Cleanup
        free_grayscale_image(&gray_original);
        if (filtered.data != NULL) free(filtered.data);
        free(resized.data);
    } else {
        // Apply filter if specified
        rgb_image_t filtered_rgb = {0};
        grayscale_image_t filtered_gray = {0};
        rgb_image_t* to_resize_rgb = &rgb_original;
        grayscale_image_t* to_print_gray = NULL;

        if (filter_type != FILTER_NONE) {
            if (filter_type == FILTER_EDGE_SOBEL || filter_type == FILTER_EDGE_LAPLACIAN || filter_type == FILTER_EDGE_PREWITT || filter_type == FILTER_EDGE_ROBERTS) {
                // These filters work on grayscale images
                grayscale_image_t gray_temp = rgb_to_grayscale(&rgb_original);
                if (gray_temp.data != NULL) {
                    if (filter_type == FILTER_EDGE_SOBEL) {
                        filtered_gray = apply_sobel_edge_detection(&gray_temp);
                    } else if (filter_type == FILTER_EDGE_PREWITT) {
                        filtered_gray = apply_prewitt_edge_detection(&gray_temp);
                    } else if (filter_type == FILTER_EDGE_ROBERTS) {
                        filtered_gray = apply_roberts_edge_detection(&gray_temp);
                    } else { // LAPLACIAN
                        kernel_t kernel = create_laplacian_kernel();
                        filtered_gray = apply_convolution_grayscale(&gray_temp, &kernel);
                        free_kernel(&kernel);
                    }
                    free(gray_temp.data); // free temp grayscale

                    if (filtered_gray.data != NULL) {
                        to_print_gray = &filtered_gray;
                    }
                }
            } else {
                // Apply filter to RGB channels
                kernel_t kernel = {0};
                switch (filter_type) {
                    case FILTER_BLUR:
                        kernel = create_gaussian_blur_kernel(5, 1.0f);
                        break;
                    case FILTER_SHARPEN:
                        kernel = create_sharpen_kernel();
                        break;
                    case FILTER_SALT_PEPPER:
                        // Apply to grayscale conversion of RGB image
                        grayscale_image_t gray_temp = rgb_to_grayscale(&rgb_original);
                        if (gray_temp.data != NULL) {
                            filtered_gray = apply_salt_pepper_noise(&gray_temp, noise_density);
                            free(gray_temp.data);
                            to_print_gray = &filtered_gray;
                        }
                        break;
                    case FILTER_IDEAL_LOWPASS:
                    case FILTER_IDEAL_HIGHPASS:
                    case FILTER_GAUSSIAN_LOWPASS:
                    case FILTER_GAUSSIAN_HIGHPASS:
                        grayscale_image_t gray_temp2 = rgb_to_grayscale(&rgb_original);
                        if (gray_temp2.data != NULL) {
                            filtered_gray = apply_frequency_filter(&gray_temp2, filter_type, cutoff);
                            free(gray_temp2.data);
                            to_print_gray = &filtered_gray;
                        }
                        break;
                    default:
                        break;
                }
                
                if (kernel.data != NULL) {
                    filtered_rgb = apply_convolution_rgb(&rgb_original, &kernel);
                    free_kernel(&kernel);
                    if (filtered_rgb.r_data != NULL) {
                        to_resize_rgb = &filtered_rgb;
                    }
                }
            }
        }

        if (to_print_gray != NULL) {
            // Process and print the grayscale result from sobel/laplacian
            grayscale_image_t resized = make_resized_grayscale(to_print_gray, max_width, max_height, interpolation_method);
            if (resized.data == NULL) {
                free_rgb_image(&rgb_original);
                free(filtered_gray.data);
                if (output_file != NULL) {
                    fclose(stdout);
                    stdout = original_stdout;
                }
                return 1;
            }

            if (color_mode == COLOR_MODE_NONE) {
                print_image(&resized, dark_mode);
            } else {
                print_grayscale_colored(&resized, dark_mode, color_mode, quantization_levels);
            }
            free(resized.data);
            free(filtered_gray.data);
        } else {
            // Process RGB image
            rgb_image_t resized = make_resized_rgb(to_resize_rgb, max_width, max_height, interpolation_method);
            if (resized.r_data == NULL) {
                free_rgb_image(&rgb_original);
                if (filtered_rgb.r_data != NULL) free_rgb_image(&filtered_rgb);
                if (output_file != NULL) {
                    fclose(stdout);
                    stdout = original_stdout;
                }
                return 1;
            }

            // Print image
            if (color_mode == COLOR_MODE_NONE) {
                // Convert to grayscale for non-color output
                grayscale_image_t gray = rgb_to_grayscale(&resized);
                print_image(&gray, dark_mode);
                free(gray.data);
            } else {
                print_rgb_image(&resized, dark_mode, color_mode, quantization_levels);
            }
            free_rgb_image(&resized);
        }

        // Cleanup
        free_rgb_image(&rgb_original);
        if (filtered_rgb.r_data != NULL) free_rgb_image(&filtered_rgb);
    }

    // Restore stdout if redirected
    if (output_file != NULL) {
        fclose(stdout);
        stdout = original_stdout;
        printf("Output saved to: %s\n", output_file);
    }

    return 0;
}
