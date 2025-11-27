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
    } else if (strcmp(filter_str, "laplacian") == 0 || strcmp(filter_str, "edge-laplacian") == 0) {
        return FILTER_EDGE_LAPLACIAN;
    } else if (strcmp(filter_str, "none") == 0) {
        return FILTER_NONE;
    } else {
        fprintf(stderr, "Warning: Unknown filter type '%s', using none\n", filter_str);
        return FILTER_NONE;
    }
}
