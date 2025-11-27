#include "../include/color_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LEVEL_CHARS " .-=+*x#$&X@"
#define N_LEVELS 12

// ANSI escape codes
#define ANSI_RESET "\033[0m"
#define ANSI_FG_256 "\033[38;5;%dm"
#define ANSI_FG_RGB "\033[38;2;%d;%d;%dm"

// Convert RGB to ANSI 16-color code (30-37 for standard, 90-97 for bright)
static int rgb_to_ansi16(unsigned char r, unsigned char g, unsigned char b) {
    // Calculate brightness
    int brightness = (r + g + b) / 3;
    
    // Determine if bright variant
    int bright = (brightness > 128) ? 60 : 0;
    
    // Find dominant color
    int max_val = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    
    // If very dark, return black
    if (max_val < 64) {
        return 30; // Black
    }
    
    // If nearly white/gray
    if (r > 200 && g > 200 && b > 200) {
        return 37 + bright; // White
    }
    
    // Determine color based on dominant channel
    if (r > g && r > b) {
        return 31 + bright; // Red
    } else if (g > r && g > b) {
        return 32 + bright; // Green
    } else if (b > r && b > g) {
        return 34 + bright; // Blue
    } else if (r > 150 && g > 150 && b < 100) {
        return 33 + bright; // Yellow
    } else if (r > 150 && b > 150 && g < 100) {
        return 35 + bright; // Magenta
    } else if (g > 150 && b > 150 && r < 100) {
        return 36 + bright; // Cyan
    }
    
    return 37 + bright; // Default to white
}

// Convert RGB to ANSI 256-color code (0-255)
static int rgb_to_ansi256(unsigned char r, unsigned char g, unsigned char b) {
    // Check if grayscale
    if (abs(r - g) < 10 && abs(g - b) < 10 && abs(r - b) < 10) {
        // Use grayscale ramp (232-255)
        if (r < 8) return 16; // Black
        if (r > 247) return 231; // White
        return 232 + (r - 8) / 10;
    }
    
    // Use 6x6x6 color cube (16-231)
    int ir = (r * 5) / 255;
    int ig = (g * 5) / 255;
    int ib = (b * 5) / 255;
    
    return 16 + 36 * ir + 6 * ig + ib;
}

// Get ASCII character based on brightness
static char get_ascii_char(unsigned char brightness, bool dark_mode) {
    size_t level = (brightness * N_LEVELS) / 256;
    if (!dark_mode) level = N_LEVELS - level - 1;
    return LEVEL_CHARS[level];
}

// Print with ANSI color code
static void print_colored_char(char ch, unsigned char r, unsigned char g, unsigned char b, color_mode_t mode) {
    switch (mode) {
        case COLOR_MODE_NONE:
            putchar(ch);
            break;
            
        case COLOR_MODE_16: {
            int color = rgb_to_ansi16(r, g, b);
            printf("\033[%dm%c" ANSI_RESET, color, ch);
            break;
        }
        
        case COLOR_MODE_256: {
            int color = rgb_to_ansi256(r, g, b);
            printf(ANSI_FG_256 "%c" ANSI_RESET, color, ch);
            break;
        }
        
        case COLOR_MODE_TRUECOLOR:
            printf(ANSI_FG_RGB "%c" ANSI_RESET, r, g, b, ch);
            break;
    }
}

void print_rgb_image(const rgb_image_t* image, bool dark_mode, color_mode_t color_mode, int levels) {
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t idx = y * image->width + x;
            unsigned char r = image->r_data[idx];
            unsigned char g = image->g_data[idx];
            unsigned char b = image->b_data[idx];
            
            // Quantize colors if levels are specified
            if (color_mode == COLOR_MODE_TRUECOLOR && levels < 256) {
                r = (unsigned char)((int)((r / 255.0) * (levels - 1)) * (255.0 / (levels - 1)));
                g = (unsigned char)((int)((g / 255.0) * (levels - 1)) * (255.0 / (levels - 1)));
                b = (unsigned char)((int)((b / 255.0) * (levels - 1)) * (255.0 / (levels - 1)));
            }

            // Calculate brightness for ASCII character selection
            unsigned char brightness = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
            char ch = get_ascii_char(brightness, dark_mode);
            
            print_colored_char(ch, r, g, b, color_mode);
        }
        printf("\n");
    }
}

void print_grayscale_colored(const grayscale_image_t* image, bool dark_mode, color_mode_t color_mode, int levels) {
    (void)levels; // Unused parameter
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t idx = y * image->width + x;
            unsigned char gray = image->data[idx];
            char ch = get_ascii_char(gray, dark_mode);
            
            // Use grayscale value for all RGB channels
            print_colored_char(ch, gray, gray, gray, color_mode);
        }
        printf("\n");
    }
}
