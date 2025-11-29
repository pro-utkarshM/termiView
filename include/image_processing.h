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

typedef enum {
    INTERPOLATION_NEAREST,
    INTERPOLATION_AVERAGE
} interpolation_method_t;

grayscale_image_t load_image_as_grayscale(const char* file_path);

grayscale_image_t make_resized_grayscale(grayscale_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method);

void print_image(grayscale_image_t* image, bool dark_mode);

void free_grayscale_image(grayscale_image_t* image);

rgb_image_t load_image_as_rgb(const char* file_path);

rgb_image_t make_resized_rgb(rgb_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method);

void free_rgb_image(rgb_image_t* image);

grayscale_image_t rgb_to_grayscale(rgb_image_t* rgb);

grayscale_image_t apply_salt_pepper_noise(const grayscale_image_t* original, float density);

void quantize_grayscale(unsigned char* image, int width, int height, int levels);

void calculate_histogram(const grayscale_image_t* image, int* histogram);
void equalize_histogram(grayscale_image_t* image);

grayscale_image_t connected_components(grayscale_image_t* image, int connectivity);

grayscale_image_t apply_otsu_thresholding(const grayscale_image_t* image);

grayscale_image_t apply_adaptive_thresholding(const grayscale_image_t* image, int block_size, double c);

grayscale_image_t apply_region_growing(const grayscale_image_t* image, int seed_x, int seed_y, int threshold);

#endif