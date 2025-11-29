#ifndef FILTERS_H
#define FILTERS_H

#include "image_processing.h"
#include <stddef.h>

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
 * Filter types supported by TermiView
 */
typedef enum {
    FILTER_NONE,
    FILTER_BLUR,
    FILTER_SHARPEN,
    FILTER_EDGE_SOBEL,
    FILTER_EDGE_PREWITT,
    FILTER_EDGE_ROBERTS,
    FILTER_EDGE_LAPLACIAN,
    FILTER_EDGE_CANNY,
    FILTER_SALT_PEPPER,
    FILTER_IDEAL_LOWPASS,
    FILTER_IDEAL_HIGHPASS,
    FILTER_GAUSSIAN_LOWPASS,
    FILTER_GAUSSIAN_HIGHPASS
} filter_type_t;

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
 * Apply Sobel edge detection to a grayscale image
 * Returns a new image with the filter applied
 */
grayscale_image_t apply_sobel_edge_detection(const grayscale_image_t* image);

/**
 * Apply Prewitt edge detection to a grayscale image
 * Returns a new image with the filter applied
 */
grayscale_image_t apply_prewitt_edge_detection(const grayscale_image_t* image);

/**
 * Apply Roberts Cross edge detection to a grayscale image
 * Returns a new image with the filter applied
 */
grayscale_image_t apply_roberts_edge_detection(const grayscale_image_t* image);

/**
 * Apply Canny edge detection to a grayscale image
 * Returns a new image with the filter applied
 */
grayscale_image_t apply_canny_edge_detection(const grayscale_image_t* image, float sigma, float low_threshold_ratio, float high_threshold_ratio);

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
 * Create a Prewitt edge detection kernel (horizontal)
 */
kernel_t create_prewitt_x_kernel(void);

/**
 * Create a Prewitt edge detection kernel (vertical)
 */
kernel_t create_prewitt_y_kernel(void);

/**
 * Create a Roberts Cross edge detection kernel (horizontal)
 */
kernel_t create_roberts_x_kernel(void);

/**
 * Create a Roberts Cross edge detection kernel (vertical)
 */
kernel_t create_roberts_y_kernel(void);

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
