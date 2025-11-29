#include "../include/filters.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Clamp a value between 0 and 255
 */
static unsigned char clamp_byte(float value) {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return (unsigned char)(value + 0.5f);
}

/**
 * Apply convolution to a single channel
 */
static unsigned char* convolve_channel(const unsigned char* input, size_t width, size_t height,
                                       const kernel_t* kernel) {
    unsigned char* output = (unsigned char*)malloc(width * height);
    if (output == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for convolution output\n");
        return NULL;
    }

    int k_half = (int)(kernel->size / 2);

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            float sum = 0.0f;

            // Apply kernel
            for (int ky = -(int)k_half; ky <= (int)k_half; ky++) {
                for (int kx = -(int)k_half; kx <= (int)k_half; kx++) {
                    // Handle boundary conditions with edge clamping
                    int px = (int)x + kx;
                    int py = (int)y + ky;

                    // Clamp to image boundaries
                    if (px < 0) px = 0;
                    if (px >= (int)width) px = (int)width - 1;
                    if (py < 0) py = 0;
                    if (py >= (int)height) py = (int)height - 1;

                    size_t pixel_idx = (size_t)py * width + (size_t)px;
                    size_t kernel_idx = (size_t)(ky + k_half) * kernel->size + (size_t)(kx + k_half);

                    sum += (float)input[pixel_idx] * kernel->data[kernel_idx];
                }
            }

            // Apply divisor and offset
            sum = sum / kernel->divisor + kernel->offset;
            output[y * width + x] = clamp_byte(sum);
        }
    }

    return output;
}

grayscale_image_t apply_convolution_grayscale(const grayscale_image_t* image, const kernel_t* kernel) {
    grayscale_image_t result = {0};

    if (image == NULL || image->data == NULL || kernel == NULL || kernel->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_convolution_grayscale\n");
        return result;
    }

    if (kernel->size % 2 == 0) {
        fprintf(stderr, "Error: Kernel size must be odd\n");
        return result;
    }

    result.width = image->width;
    result.height = image->height;
    result.data = convolve_channel(image->data, image->width, image->height, kernel);

    if (result.data == NULL) {
        result.width = 0;
        result.height = 0;
    }

    return result;
}

rgb_image_t apply_convolution_rgb(const rgb_image_t* image, const kernel_t* kernel) {
    rgb_image_t result = {0};

    if (image == NULL || image->r_data == NULL || kernel == NULL || kernel->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_convolution_rgb\n");
        return result;
    }

    if (kernel->size % 2 == 0) {
        fprintf(stderr, "Error: Kernel size must be odd\n");
        return result;
    }

    result.width = image->width;
    result.height = image->height;

    // Apply convolution to each channel
    result.r_data = convolve_channel(image->r_data, image->width, image->height, kernel);
    result.g_data = convolve_channel(image->g_data, image->width, image->height, kernel);
    result.b_data = convolve_channel(image->b_data, image->width, image->height, kernel);

    if (result.r_data == NULL || result.g_data == NULL || result.b_data == NULL) {
        // Cleanup on failure
        free(result.r_data);
        free(result.g_data);
        free(result.b_data);
        result.width = 0;
        result.height = 0;
        result.r_data = result.g_data = result.b_data = NULL;
    }

    return result;
}

kernel_t create_gaussian_blur_kernel(size_t size, float sigma) {
    kernel_t kernel = {0};

    if (size % 2 == 0) {
        fprintf(stderr, "Error: Kernel size must be odd\n");
        return kernel;
    }

    kernel.size = size;
    kernel.data = (float*)malloc(size * size * sizeof(float));
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for kernel\n");
        return kernel;
    }

    int half = (int)(size / 2);
    float sum = 0.0f;
    float sigma_sq_2 = 2.0f * sigma * sigma;

    // Generate Gaussian kernel
    for (int y = -half; y <= half; y++) {
        for (int x = -half; x <= half; x++) {
            float exponent = -(float)(x * x + y * y) / sigma_sq_2;
            float value = expf(exponent) / (M_PI * sigma_sq_2);
            
            size_t idx = (size_t)(y + half) * size + (size_t)(x + half);
            kernel.data[idx] = value;
            sum += value;
        }
    }

    // Normalize kernel (sum of all values should equal 1)
    kernel.divisor = sum;

    return kernel;
}

kernel_t create_sharpen_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for sharpen kernel\n");
        return kernel;
    }

    // Sharpen kernel:
    //  0  -1   0
    // -1   5  -1
    //  0  -1   0
    float values[] = {
         0.0f, -1.0f,  0.0f,
        -1.0f,  5.0f, -1.0f,
         0.0f, -1.0f,  0.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_sobel_x_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Sobel X kernel\n");
        return kernel;
    }

    // Sobel X (horizontal edges):
    // -1  0  1
    // -2  0  2
    // -1  0  1
    float values[] = {
        -1.0f,  0.0f,  1.0f,
        -2.0f,  0.0f,  2.0f,
        -1.0f,  0.0f,  1.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_sobel_y_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Sobel Y kernel\n");
        return kernel;
    }

    // Sobel Y (vertical edges):
    // -1 -2 -1
    //  0  0  0
    //  1  2  1
    float values[] = {
        -1.0f, -2.0f, -1.0f,
         0.0f,  0.0f,  0.0f,
         1.0f,  2.0f,  1.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_prewitt_x_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Prewitt X kernel\n");
        return kernel;
    }

    // Prewitt X (horizontal edges):
    // -1  0  1
    // -1  0  1
    // -1  0  1
    float values[] = {
        -1.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  1.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_prewitt_y_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Prewitt Y kernel\n");
        return kernel;
    }

    // Prewitt Y (vertical edges):
    // -1 -1 -1
    //  0  0  0
    //  1  1  1
    float values[] = {
        -1.0f, -1.0f, -1.0f,
         0.0f,  0.0f,  0.0f,
         1.0f,  1.0f,  1.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_roberts_x_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Roberts X kernel\n");
        return kernel;
    }

    // Roberts X (diagonal edges):
    // +1  0  0
    //  0 -1  0
    //  0  0  0
    float values[] = {
        1.0f,  0.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f,  0.0f,  0.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_roberts_y_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Roberts Y kernel\n");
        return kernel;
    }

    // Roberts Y (diagonal edges):
    //  0 +1  0
    // -1  0  0
    //  0  0  0
    float values[] = {
        0.0f,  1.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

kernel_t create_laplacian_kernel(void) {
    kernel_t kernel = {0};
    kernel.size = 3;
    kernel.data = (float*)malloc(9 * sizeof(float));
    kernel.divisor = 1.0f;
    kernel.offset = 0.0f;

    if (kernel.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Laplacian kernel\n");
        return kernel;
    }

    // Laplacian (edge detection):
    //  0  1  0
    //  1 -4  1
    //  0  1  0
    float values[] = {
         0.0f,  1.0f,  0.0f,
         1.0f, -4.0f,  1.0f,
         0.0f,  1.0f,  0.0f
    };

    memcpy(kernel.data, values, 9 * sizeof(float));
    return kernel;
}

void free_kernel(kernel_t* kernel) {
    if (kernel != NULL && kernel->data != NULL) {
        free(kernel->data);
        kernel->data = NULL;
        kernel->size = 0;
    }
}

grayscale_image_t apply_sobel_edge_detection(const grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_sobel_edge_detection\n");
        return result;
    }

    // 1. Create Sobel kernels
    kernel_t kernel_x = create_sobel_x_kernel();
    kernel_t kernel_y = create_sobel_y_kernel();

    // 2. Apply convolutions
    grayscale_image_t gx_image = apply_convolution_grayscale(image, &kernel_x);
    grayscale_image_t gy_image = apply_convolution_grayscale(image, &kernel_y);

    // 3. Combine results
    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)malloc(result.width * result.height);
    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Sobel result\n");
        result.width = 0;
        result.height = 0;
    } else {
        for (size_t i = 0; i < result.width * result.height; i++) {
            float gx = (float)gx_image.data[i];
            float gy = (float)gy_image.data[i];
            float magnitude = sqrtf(gx * gx + gy * gy);
            result.data[i] = clamp_byte(magnitude);
        }
    }

    // 4. Cleanup
    free(gx_image.data);
    free(gy_image.data);
    free_kernel(&kernel_x);
    free_kernel(&kernel_y);

    return result;
}

grayscale_image_t apply_prewitt_edge_detection(const grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_prewitt_edge_detection\n");
        return result;
    }

    // 1. Create Prewitt kernels
    kernel_t kernel_x = create_prewitt_x_kernel();
    kernel_t kernel_y = create_prewitt_y_kernel();

    // 2. Apply convolutions
    grayscale_image_t gx_image = apply_convolution_grayscale(image, &kernel_x);
    grayscale_image_t gy_image = apply_convolution_grayscale(image, &kernel_y);

    // 3. Combine results
    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)malloc(result.width * result.height);
    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Prewitt result\n");
        result.width = 0;
        result.height = 0;
    } else {
        for (size_t i = 0; i < result.width * result.height; i++) {
            float gx = (float)gx_image.data[i];
            float gy = (float)gy_image.data[i];
            float magnitude = sqrtf(gx * gx + gy * gy);
            result.data[i] = clamp_byte(magnitude);
        }
    }

    // 4. Cleanup
    free(gx_image.data);
    free(gy_image.data);
    free_kernel(&kernel_x);
    free_kernel(&kernel_y);

    return result;
}

grayscale_image_t apply_roberts_edge_detection(const grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_roberts_edge_detection\n");
        return result;
    }

    // 1. Create Roberts kernels
    kernel_t kernel_x = create_roberts_x_kernel();
    kernel_t kernel_y = create_roberts_y_kernel();

    // 2. Apply convolutions
    grayscale_image_t gx_image = apply_convolution_grayscale(image, &kernel_x);
    grayscale_image_t gy_image = apply_convolution_grayscale(image, &kernel_y);

    // 3. Combine results
    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)malloc(result.width * result.height);
    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Roberts result\n");
        result.width = 0;
        result.height = 0;
    } else {
        for (size_t i = 0; i < result.width * result.height; i++) {
            float gx = (float)gx_image.data[i];
            float gy = (float)gy_image.data[i];
            float magnitude = sqrtf(gx * gx + gy * gy);
            result.data[i] = clamp_byte(magnitude);
        }
    }

    // 4. Cleanup
    free(gx_image.data);
    free(gy_image.data);
    free_kernel(&kernel_x);
    free_kernel(&kernel_y);

    return result;
}


// Recursive helper for hysteresis
static void hysteresis_recursive(grayscale_image_t* image, int x, int y) {
    if (x < 0 || x >= (int)image->width || y < 0 || y >= (int)image->height || image->data[y * image->width + x] != 128) {
        return;
    }

    image->data[y * image->width + x] = 255; // Mark as strong edge

    // Check 8 neighbors
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            hysteresis_recursive(image, x + dx, y + dy);
        }
    }
}

static void hysteresis_edge_tracking(grayscale_image_t* image) {
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            if (image->data[y * image->width + x] == 255) {
                hysteresis_recursive(image, (int)x, (int)y);
            }
        }
    }

    // Clean up any remaining weak edges
    for (size_t i = 0; i < image->width * image->height; i++) {
        if (image->data[i] == 128) {
            image->data[i] = 0;
        }
    }
}

static grayscale_image_t double_thresholding(const grayscale_image_t* image, float low_ratio, float high_ratio) {
    grayscale_image_t result = {0};
    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)calloc(image->width * image->height, sizeof(unsigned char));

    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for thresholding result\n");
        result.width = 0;
        result.height = 0;
        return result;
    }

    // Find the maximum magnitude to scale thresholds
    float max_mag = 0;
    for (size_t i = 0; i < image->width * image->height; i++) {
        if (image->data[i] > max_mag) {
            max_mag = image->data[i];
        }
    }

    float high_threshold = max_mag * high_ratio;
    float low_threshold = high_threshold * low_ratio;

    const unsigned char WEAK = 128;
    const unsigned char STRONG = 255;

    for (size_t i = 0; i < image->width * image->height; i++) {
        if (image->data[i] >= high_threshold) {
            result.data[i] = STRONG;
        } else if (image->data[i] >= low_threshold) {
            result.data[i] = WEAK;
        }
    }

    return result;
}

static grayscale_image_t non_maximum_suppression(size_t width, size_t height, const float* magnitude, const float* orientation) {
    grayscale_image_t result = {0};
    result.width = width;
    result.height = height;
    result.data = (unsigned char*)calloc(width * height, sizeof(unsigned char));
    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for non-maximum suppression\n");
        result.width = 0;
        result.height = 0;
        return result;
    }

    for (size_t y = 1; y < height - 1; y++) {
        for (size_t x = 1; x < width - 1; x++) {
            size_t i = y * width + x;
            float angle = orientation[i] * 180.0f / M_PI;
            if (angle < 0) angle += 180;

            float mag = magnitude[i];
            float q = 255.0f;
            float r = 255.0f;

            // Find neighbors in the gradient direction
            if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
                q = magnitude[i + 1];
                r = magnitude[i - 1];
            } else if (22.5 <= angle && angle < 67.5) {
                q = magnitude[i - width + 1];
                r = magnitude[i + width - 1];
            } else if (67.5 <= angle && angle < 112.5) {
                q = magnitude[i - width];
                r = magnitude[i + width];
            } else if (112.5 <= angle && angle < 157.5) {
                q = magnitude[i - width - 1];
                r = magnitude[i + width + 1];
            }

            // Suppress if not a local maximum
            if (mag >= q && mag >= r) {
                result.data[i] = clamp_byte(mag);
            }
        }
    }
    return result;
}

grayscale_image_t apply_canny_edge_detection(const grayscale_image_t* image, float sigma, float low_threshold_ratio, float high_threshold_ratio) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Invalid input to apply_canny_edge_detection\n");
        return result;
    }

    // Step 1: Apply Gaussian Blur
    // For Canny, typically a 5x5 kernel with sigma=1.4 is a good starting point
    size_t kernel_size = 5; 
    if (sigma <= 0) sigma = 1.4f; // Default sigma if not provided or invalid
    if (kernel_size % 2 == 0) kernel_size++; // Ensure odd kernel size

    kernel_t gaussian_kernel = create_gaussian_blur_kernel(kernel_size, sigma);
    grayscale_image_t blurred_image = apply_convolution_grayscale(image, &gaussian_kernel);
    free_kernel(&gaussian_kernel);

    if (blurred_image.data == NULL) {
        return result;
    }


    // Step 2: Calculate Gradient Magnitude and Direction
    grayscale_image_t gx_image, gy_image;
    float* magnitude = (float*)malloc(image->width * image->height * sizeof(float));
    float* orientation = (float*)malloc(image->width * image->height * sizeof(float));

    if (magnitude == NULL || orientation == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for gradient calculation\n");
        free(blurred_image.data);
        free(magnitude);
        free(orientation);
        return result;
    }

    kernel_t sobel_x = create_sobel_x_kernel();
    kernel_t sobel_y = create_sobel_y_kernel();

    gx_image = apply_convolution_grayscale(&blurred_image, &sobel_x);
    gy_image = apply_convolution_grayscale(&blurred_image, &sobel_y);

    free_kernel(&sobel_x);
    free_kernel(&sobel_y);

    if (gx_image.data == NULL || gy_image.data == NULL) {
        free(blurred_image.data);
        free(gx_image.data);
        free(gy_image.data);
        free(magnitude);
        free(orientation);
        return result;
    }

    for (size_t i = 0; i < image->width * image->height; i++) {
        float gx = (float)gx_image.data[i];
        float gy = (float)gy_image.data[i];
        magnitude[i] = sqrtf(gx * gx + gy * gy);
        orientation[i] = atan2f(gy, gx);
    }
    
    free(gx_image.data);
    free(gy_image.data);

    // Step 3: Non-Maximum Suppression
    grayscale_image_t nms_image = non_maximum_suppression(image->width, image->height, magnitude, orientation);
    
    // Cleanup allocated memory for gradient
    free(magnitude);
    free(orientation);
    
    if (nms_image.data == NULL) {
        free(blurred_image.data);
        return result;
    }

    // Step 4: Double Thresholding
    grayscale_image_t thresholded_image = double_thresholding(&nms_image, low_threshold_ratio, high_threshold_ratio);
    free(nms_image.data);

    if(thresholded_image.data == NULL){
        free(blurred_image.data);
        return result;
    }

    // Step 5: Edge Tracking by Hysteresis
    hysteresis_edge_tracking(&thresholded_image);

    // Final result is the thresholded image after hysteresis
    result = thresholded_image; 
    
    // Cleanup allocated memory
    free(blurred_image.data);

    return result;
}

filter_type_t parse_filter_type(const char* filter_str) {
    if (filter_str == NULL) {
        return FILTER_NONE;
    }

    if (strcmp(filter_str, "blur") == 0) {
        return FILTER_BLUR;
    } else if (strcmp(filter_str, "sharpen") == 0) {
        return FILTER_SHARPEN;
    } else if (strcmp(filter_str, "sobel") == 0 || strcmp(filter_str, "edge-sobel") == 0) {
        return FILTER_EDGE_SOBEL;
    } else if (strcmp(filter_str, "prewitt") == 0 || strcmp(filter_str, "edge-prewitt") == 0) {
        return FILTER_EDGE_PREWITT;
    } else if (strcmp(filter_str, "roberts") == 0 || strcmp(filter_str, "edge-roberts") == 0) {
        return FILTER_EDGE_ROBERTS;
    } else if (strcmp(filter_str, "canny") == 0 || strcmp(filter_str, "edge-canny") == 0) {
        return FILTER_EDGE_CANNY;
    } else if (strcmp(filter_str, "laplacian") == 0 || strcmp(filter_str, "edge-laplacian") == 0) {
        return FILTER_EDGE_LAPLACIAN;
    } else if (strcmp(filter_str, "salt-pepper") == 0) {
        return FILTER_SALT_PEPPER;
    } else if (strcmp(filter_str, "ideal-lowpass") == 0) {
        return FILTER_IDEAL_LOWPASS;
    } else if (strcmp(filter_str, "ideal-highpass") == 0) {
        return FILTER_IDEAL_HIGHPASS;
    } else if (strcmp(filter_str, "gaussian-lowpass") == 0) {
        return FILTER_GAUSSIAN_LOWPASS;
    } else if (strcmp(filter_str, "gaussian-highpass") == 0) {
        return FILTER_GAUSSIAN_HIGHPASS;
    } else if (strcmp(filter_str, "none") == 0) {
        return FILTER_NONE;
    } else {
        fprintf(stderr, "Warning: Unknown filter type '%s', using none\n", filter_str);
        return FILTER_NONE;
    }
}
