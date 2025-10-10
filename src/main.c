#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/image_processing.h"

#define DEFAULT_MAX_WIDTH 64
#define DEFAULT_MAX_HEIGHT 48


int main(int argc, char* argv[]) {
    // Parse arguments
    if (argc == 1) {
        printf(
            "Usage: %s [path/to/image] [max width (default %d)] [max height (default %d)] [dark mode (y/n, default=y)]\n",
            argv[0], DEFAULT_MAX_WIDTH, DEFAULT_MAX_HEIGHT
        );
        return 1;
    }

    char* file_path = argv[1];
    size_t max_width = DEFAULT_MAX_WIDTH, max_height = DEFAULT_MAX_HEIGHT;
    if (argc >= 3) max_width = (size_t) atoi(argv[2]);
    if (argc >= 4) max_height = (size_t) atoi(argv[3]);

    bool dark_mode = true;
    if (argc >= 5) dark_mode = (argv[4][0] == 'y');

    // Load image
    grayscale_image_t original = load_image_as_grayscale(file_path);

    grayscale_image_t resized = make_resized_grayscale(&original, max_width, max_height);
    
    print_image(&resized, dark_mode);

    return 0;
}