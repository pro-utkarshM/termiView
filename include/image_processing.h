#ifndef IMAGE_PROCESSING
#define IMAGE_PROCESSING
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t width;
    size_t height;
    unsigned char* data;
} grayscale_image_t;

typedef struct {
    size_t width;
    size_t height;
    unsigned char* r_data;
    unsigned char* g_data;
    unsigned char* b_data;
} rgb_image_t;

grayscale_image_t load_image_as_grayscale(const char* file_path);

grayscale_image_t make_resized_grayscale(grayscale_image_t* original, size_t max_width, size_t max_height);

void print_image(grayscale_image_t* image, bool dark_mode);

void free_grayscale_image(grayscale_image_t* image);

rgb_image_t load_image_as_rgb(const char* file_path);

rgb_image_t make_resized_rgb(rgb_image_t* original, size_t max_width, size_t max_height);

void free_rgb_image(rgb_image_t* image);

grayscale_image_t rgb_to_grayscale(rgb_image_t* rgb);
#endif