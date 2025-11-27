#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "../include/image_processing.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define LEVEL_CHARS " .-=+*x#$&X@"
#define N_LEVELS 12


grayscale_image_t load_image_as_grayscale(const char* file_path) {
    int width, height, _;
    unsigned char* data = stbi_load(file_path, &width, &height, &_, 1);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to load image '%s': %s\n", file_path, stbi_failure_reason());
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

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


grayscale_image_t make_resized_grayscale(grayscale_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method) {
    size_t width, height;

    size_t proposed_height = (original->height * max_width) / (2 * original->width);
    if (proposed_height <= max_height) {
        width = max_width, height = proposed_height;
    } else {
        width = (2 * original->width * max_height) / (original->height);
        height = max_height;
    }

    unsigned char* data = calloc(width * height, sizeof(unsigned char));
    if (data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for resized image\n");
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    if (method == INTERPOLATION_NEAREST) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                size_t x = (i * original->width) / width;
                size_t y = (j * original->height) / height;
                data[i + j * width] = original->data[x + y * original->width];
            }
        }
    } else { // INTERPOLATION_AVERAGE
        for (size_t i = 0; i < width; i++) {
            size_t x1 = (i * original->width) / (width);
            size_t x2 = ((i + 1) * original->width) / (width);
            for (size_t j = 0; j < height; j++) {
                size_t y1 = (j * original->height) / (height);
                size_t y2 = ((j + 1) * original->height) / (height);

                data[i + j * width] = get_average(original, x1, x2, y1, y2);
            }
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

void free_grayscale_image(grayscale_image_t* image) {
    if (image != NULL && image->data != NULL) {
        stbi_image_free(image->data);
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}


rgb_image_t load_image_as_rgb(const char* file_path) {
    int width, height, channels;
    unsigned char* data = stbi_load(file_path, &width, &height, &channels, 3);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to load image '%s': %s\n", file_path, stbi_failure_reason());
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    // Separate RGB channels
    size_t pixel_count = width * height;
    unsigned char* r_data = malloc(pixel_count);
    unsigned char* g_data = malloc(pixel_count);
    unsigned char* b_data = malloc(pixel_count);

    if (r_data == NULL || g_data == NULL || b_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for RGB channels\n");
        stbi_image_free(data);
        free(r_data);
        free(g_data);
        free(b_data);
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    for (size_t i = 0; i < pixel_count; i++) {
        r_data[i] = data[i * 3 + 0];
        g_data[i] = data[i * 3 + 1];
        b_data[i] = data[i * 3 + 2];
    }

    stbi_image_free(data);

    return (rgb_image_t) {
        .width = (size_t) width,
        .height = (size_t) height,
        .r_data = r_data,
        .g_data = g_data,
        .b_data = b_data
    };
}


void free_rgb_image(rgb_image_t* image) {
    if (image != NULL) {
        if (image->r_data != NULL) {
            free(image->r_data);
            image->r_data = NULL;
        }
        if (image->g_data != NULL) {
            free(image->g_data);
            image->g_data = NULL;
        }
        if (image->b_data != NULL) {
            free(image->b_data);
            image->b_data = NULL;
        }
        image->width = 0;
        image->height = 0;
    }
}


typedef struct {
    unsigned char r, g, b;
} rgb_pixel_t;

rgb_pixel_t get_average_rgb(rgb_image_t* image, size_t x1, size_t x2, size_t y1, size_t y2) {
    double r_total = 0, g_total = 0, b_total = 0;

    for (size_t x = x1; x < x2; x++) {
        for (size_t y = y1; y < y2; y++) {
            size_t idx = x + y * image->width;
            r_total += (double) image->r_data[idx];
            g_total += (double) image->g_data[idx];
            b_total += (double) image->b_data[idx];
        }
    }

    double n = (x2 - x1) * (y2 - y1);

    return (rgb_pixel_t) {
        .r = (unsigned char) (r_total / n),
        .g = (unsigned char) (g_total / n),
        .b = (unsigned char) (b_total / n)
    };
}


rgb_image_t make_resized_rgb(rgb_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method) {
    size_t width, height;

    size_t proposed_height = (original->height * max_width) / (2 * original->width);
    if (proposed_height <= max_height) {
        width = max_width, height = proposed_height;
    } else {
        width = (2 * original->width * max_height) / (original->height);
        height = max_height;
    }

    unsigned char* r_data = calloc(width * height, sizeof(unsigned char));
    unsigned char* g_data = calloc(width * height, sizeof(unsigned char));
    unsigned char* b_data = calloc(width * height, sizeof(unsigned char));

    if (r_data == NULL || g_data == NULL || b_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for resized RGB image\n");
        free(r_data);
        free(g_data);
        free(b_data);
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    if (method == INTERPOLATION_NEAREST) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                size_t x = (i * original->width) / width;
                size_t y = (j * original->height) / height;
                size_t original_idx = x + y * original->width;
                size_t new_idx = i + j * width;
                r_data[new_idx] = original->r_data[original_idx];
                g_data[new_idx] = original->g_data[original_idx];
                b_data[new_idx] = original->b_data[original_idx];
            }
        }
    } else { // INTERPOLATION_AVERAGE
        for (size_t i = 0; i < width; i++) {
            size_t x1 = (i * original->width) / (width);
            size_t x2 = ((i + 1) * original->width) / (width);
            for (size_t j = 0; j < height; j++) {
                size_t y1 = (j * original->height) / (height);
                size_t y2 = ((j + 1) * original->height) / (height);

                rgb_pixel_t avg = get_average_rgb(original, x1, x2, y1, y2);
                size_t idx = i + j * width;
                r_data[idx] = avg.r;
                g_data[idx] = avg.g;
                b_data[idx] = avg.b;
            }
        }
    }

    return (rgb_image_t) {
        .width = width,
        .height = height,
        .r_data = r_data,
        .g_data = g_data,
        .b_data = b_data
    };
}


grayscale_image_t rgb_to_grayscale(rgb_image_t* rgb) {
    size_t pixel_count = rgb->width * rgb->height;
    unsigned char* data = malloc(pixel_count);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for grayscale conversion\n");
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    // Use standard luminance weights: 0.299*R + 0.587*G + 0.114*B
    for (size_t i = 0; i < pixel_count; i++) {
        data[i] = (unsigned char)(
            0.299 * rgb->r_data[i] +
            0.587 * rgb->g_data[i] +
            0.114 * rgb->b_data[i]
        );
    }

    return (grayscale_image_t) {
        .width = rgb->width,
        .height = rgb->height,
        .data = data
    };
}
