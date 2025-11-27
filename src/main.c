#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "../include/image_processing.h"
#include "../include/color_output.h"
#include "../include/filters.h"

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
    printf("  -d, --dark             Use dark mode (default)\n");
    printf("  -l, --light            Use light mode\n");
    printf("  -o, --output <file>    Save output to file instead of stdout\n");
    printf("  -f, --filter <type>    Apply filter: blur, sharpen, sobel, laplacian (default: none)\n");
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

    // Long options
    static struct option long_options[] = {
        {"width",   required_argument, 0, 'w'},
        {"height",  required_argument, 0, 'h'},
        {"color",   required_argument, 0, 'c'},
        {"dark",    no_argument,       0, 'd'},
        {"light",   no_argument,       0, 'l'},
        {"output",  required_argument, 0, 'o'},
        {"filter",  required_argument, 0, 'f'},
        {"version", no_argument,       0, 'v'},
        {"help",    no_argument,       0,  0 },
        {0, 0, 0, 0}
    };

    // Parse options
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "w:h:c:dlo:f:v", long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                // Long option with no short equivalent
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    print_usage(argv[0]);
                    return 0;
                }
                break;
            case 'w':
                max_width = (size_t) atoi(optarg);
                if (max_width == 0) {
                    fprintf(stderr, "Error: Invalid width value\n");
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
                case FILTER_EDGE_LAPLACIAN:
                    kernel = create_laplacian_kernel();
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
            } else if (filter_type == FILTER_EDGE_SOBEL && filtered.data == NULL) {
                // Sobel failed, so we should not proceed
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
        
        grayscale_image_t resized = make_resized_grayscale(to_resize, max_width, max_height);
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
            print_grayscale_colored(&resized, dark_mode, color_mode);
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
            if (filter_type == FILTER_EDGE_SOBEL || filter_type == FILTER_EDGE_LAPLACIAN) {
                // These filters work on grayscale images
                grayscale_image_t gray_temp = rgb_to_grayscale(&rgb_original);
                if (gray_temp.data != NULL) {
                    if (filter_type == FILTER_EDGE_SOBEL) {
                        filtered_gray = apply_sobel_edge_detection(&gray_temp);
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
            grayscale_image_t resized = make_resized_grayscale(to_print_gray, max_width, max_height);
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
                print_grayscale_colored(&resized, dark_mode, color_mode);
            }
            free(resized.data);
            free(filtered_gray.data);
        } else {
            // Process RGB image
            rgb_image_t resized = make_resized_rgb(to_resize_rgb, max_width, max_height);
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
                print_rgb_image(&resized, dark_mode, color_mode);
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
