#ifndef FILTERS_H
#define FILTERS_H

#include "image_processing.h"
#include <stddef.h>

/**
 * Filter types supported by TermiView
 */
typedef enum {
    FILTER_NONE,
    FILTER_BLUR,
    FILTER_SHARPEN,
    FILTER_EDGE_SOBEL,
    FILTER_EDGE_LAPLACIAN
} filter_type_t;

/**
 * Represents a convolution kernel
 */
typedef struct {
    size_t size;        // Kernel dimension (e.g., 3 for 3x3, 5 for 5x5)
    float* data;        // Kernel values (size x size array)
    float divisor;      // Normalization divisor
    float offset;       // Offset to add after convolution
} kernel_t;

/**
 * Apply a convolution kernel to a grayscale image
 * Returns a new image with the filter applied
 */
grayscale_image_t apply_convolution_grayscale(const grayscale_image_t* image, const kernel_t* kernel);

/**
 * Apply a convolution kernel to an RGB image (applies to each channel)
 * Returns a new image with the filter applied
 */
rgb_image_t apply_convolution_rgb(const rgb_image_t* image, const kernel_t* kernel);

/**
 * Create a Gaussian blur kernel
 * size: kernel dimension (must be odd, e.g., 3, 5, 7)
 * sigma: standard deviation for Gaussian distribution
 */
kernel_t create_gaussian_blur_kernel(size_t size, float sigma);

/**
 * Create a sharpen kernel
 */
kernel_t create_sharpen_kernel(void);

/**
 * Create a Sobel edge detection kernel (horizontal)
 */
kernel_t create_sobel_x_kernel(void);

/**
 * Create a Sobel edge detection kernel (vertical)
 */
kernel_t create_sobel_y_kernel(void);

/**
 * Create a Laplacian edge detection kernel
 */
kernel_t create_laplacian_kernel(void);

/**
 * Free kernel memory
 */
void free_kernel(kernel_t* kernel);

/**
 * Parse filter type from string
 */
filter_type_t parse_filter_type(const char* filter_str);

#endif // FILTERS_H
