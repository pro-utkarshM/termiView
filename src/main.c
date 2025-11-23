#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "../include/image_processing.h"
#include "../include/color_output.h"

#define VERSION "0.2.0"
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
    printf("  -v, --version          Show version information\n");
    printf("  --help                 Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s image.jpg\n", program_name);
    printf("  %s -w 80 -h 60 --color 256 image.png\n", program_name);
    printf("  %s --light --color 16 photo.jpeg\n", program_name);
    printf("  %s -c truecolor -o output.txt image.jpg\n\n", program_name);
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

    // Long options
    static struct option long_options[] = {
        {"width",   required_argument, 0, 'w'},
        {"height",  required_argument, 0, 'h'},
        {"color",   required_argument, 0, 'c'},
        {"dark",    no_argument,       0, 'd'},
        {"light",   no_argument,       0, 'l'},
        {"output",  required_argument, 0, 'o'},
        {"version", no_argument,       0, 'v'},
        {"help",    no_argument,       0,  0 },
        {0, 0, 0, 0}
    };

    // Parse options
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "w:h:c:dlo:v", long_options, &option_index)) != -1) {
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

        // Process grayscale image
        grayscale_image_t resized = make_resized_grayscale(&gray_original, max_width, max_height);
        if (resized.data == NULL) {
            free_grayscale_image(&gray_original);
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
        free_grayscale_image(&resized);
    } else {
        // Process RGB image
        rgb_image_t resized = make_resized_rgb(&rgb_original, max_width, max_height);
        if (resized.r_data == NULL) {
            free_rgb_image(&rgb_original);
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
            free_grayscale_image(&gray);
        } else {
            print_rgb_image(&resized, dark_mode, color_mode);
        }

        // Cleanup
        free_rgb_image(&rgb_original);
        free_rgb_image(&resized);
    }

    // Restore stdout if redirected
    if (output_file != NULL) {
        fclose(stdout);
        stdout = original_stdout;
        printf("Output saved to: %s\n", output_file);
    }

    return 0;
}
