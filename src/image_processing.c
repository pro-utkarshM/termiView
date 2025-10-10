#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "../include/image_processing.h"
#include <stdbool.h>

#define LEVEL_CHARS " .-=+*x#$&X@"
#define N_LEVELS 12


grayscale_image_t load_image_as_grayscale(const char* file_path) {
    int width, height, _;
    unsigned char* data = stbi_load(file_path, &width, &height, &_, 1);

    return (grayscale_image_t) {
        .width = (size_t) width,
        .height = (size_t) height,
        .data = data
    };
}


unsigned char get_average(grayscale_image_t* image, size_t x1, size_t x2, size_t y1, size_t y2) {
    double total = 0;

    for (size_t x = x1; x < x2; x++) {
        for (size_t y = y1; y < y2; y++) {
            total += (double) image->data[x + y * image->width];
        }
    }

    double n = (x2 - x1) * (y2 - y1);

    return (char) (total / n);
}


grayscale_image_t make_resized_grayscale(grayscale_image_t* original, size_t max_width, size_t max_height) {
    size_t width, height;

    size_t proposed_height = (original->height * max_width) / (2 * original->width);
    if (proposed_height <= max_height) {
        width = max_width, height = proposed_height;
    } else {
        width = (2 * original->width * max_height) / (original->height);
        height = max_height;
    }

    unsigned char* data = calloc(width * height, sizeof(data));

    for (size_t i = 0; i < width; i++) {
        size_t x1 = (i * original->width) / (width);
        size_t x2 = ((i + 1) * original->width) / (width);
        for (size_t j = 0; j < height; j++) {
            size_t y1 = (j * original->height) / (height);
            size_t y2 = ((j + 1) * original->height) / (height);

            data[i + j * width] = get_average(original, x1, x2, y1, y2);
        }
    }

    return (grayscale_image_t) {
        .width = width,
        .height = height,
        .data = data
    };
}


void print_image(grayscale_image_t* image, bool dark_mode) {
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t level = (image->data[y * image->width + x] * N_LEVELS) / 256;
            if (!dark_mode) level = N_LEVELS - level - 1;
            putchar(LEVEL_CHARS[level]);
        }
        printf("\n");
    }
}