#ifndef COLOR_OUTPUT
#define COLOR_OUTPUT
#include <stdbool.h>
#include "image_processing.h"

typedef enum {
    COLOR_MODE_NONE,      // No color, grayscale ASCII only
    COLOR_MODE_16,        // 16 ANSI colors
    COLOR_MODE_256,       // 256 ANSI colors
    COLOR_MODE_TRUECOLOR  // 24-bit RGB truecolor
} color_mode_t;

void print_rgb_image(rgb_image_t* image, bool dark_mode, color_mode_t color_mode);

void print_grayscale_colored(grayscale_image_t* image, bool dark_mode, color_mode_t color_mode);

#endif
