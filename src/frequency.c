#define _USE_MATH_DEFINES // For M_PI on some systems
#include "../include/frequency.h"
#include <fftw3.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdlib.h>
#include <string.h>

// Helper to shift the zero-frequency component to the center
static void fft_shift(double* data, size_t width, size_t height) {
    size_t half_w = width / 2;
    size_t half_h = height / 2;

    for (size_t y = 0; y < half_h; y++) {
        for (size_t x = 0; x < half_w; x++) {
            // Swap quadrants 1 and 3
            double tmp = data[y * width + x];
            data[y * width + x] = data[(y + half_h) * width + (x + half_w)];
            data[(y + half_h) * width + (x + half_w)] = tmp;

            // Swap quadrants 2 and 4
            tmp = data[y * width + (x + half_w)];
            data[y * width + (x + half_w)] = data[(y + half_h) * width + x];
            data[(y + half_h) * width + x] = tmp;
        }
    }
}

grayscale_image_t dft_grayscale(grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    size_t width = image->width;
    size_t height = image->height;
    size_t num_pixels = width * height;

    // Allocate FFTW arrays
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_pixels);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_pixels);

    if (in == NULL || out == NULL) {
        fftw_free(in);
        fftw_free(out);
        return result;
    }

    // Prepare input data
    for (size_t i = 0; i < num_pixels; i++) {
        in[i][0] = (double)image->data[i];
        in[i][1] = 0.0;
    }

    // Create and execute FFTW plan
    fftw_plan plan = fftw_plan_dft_2d(height, width, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    // Calculate magnitude spectrum
    double* magnitude = (double*)malloc(sizeof(double) * num_pixels);
    if (magnitude == NULL) {
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
        return result;
    }

    for (size_t i = 0; i < num_pixels; i++) {
        double real = out[i][0];
        double imag = out[i][1];
        // Log scale: log(1 + |F(u,v)|)
        magnitude[i] = log(1 + sqrt(real * real + imag * imag));
    }

    // Shift zero frequency to center
    fft_shift(magnitude, width, height);

    // Normalize to 0-255 for display
    double max_mag = 0.0;
    for (size_t i = 0; i < num_pixels; i++) {
        if (magnitude[i] > max_mag) {
            max_mag = magnitude[i];
        }
    }

    result.width = width;
    result.height = height;
    result.data = (unsigned char*)malloc(num_pixels);
    if (result.data != NULL) {
        if (max_mag > 0) {
            for (size_t i = 0; i < num_pixels; i++) {
                result.data[i] = (unsigned char)((magnitude[i] / max_mag) * 255.0);
            }
        } else {
            memset(result.data, 0, num_pixels);
        }
    }

    // Cleanup
    free(magnitude);
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return result;
}

// Helper function to get alpha value for DCT
static double get_alpha(int i, int N) {
    if (i == 0) {
        return sqrt(1.0 / N);
    } else {
        return sqrt(2.0 / N);
    }
}

grayscale_image_t dct_grayscale(grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    size_t width = image->width;
    size_t height = image->height;
    size_t num_pixels = width * height;

    // DCT coefficients will be double
    double* dct_coeffs = (double*)malloc(sizeof(double) * num_pixels);
    if (dct_coeffs == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for DCT coefficients\n");
        return result;
    }

    for (size_t u = 0; u < height; u++) { // u corresponds to rows
        for (size_t v = 0; v < width; v++) { // v corresponds to columns
            double sum = 0.0;
            for (size_t x = 0; x < height; x++) { // x corresponds to rows
                for (size_t y = 0; y < width; y++) { // y corresponds to columns
                    sum += (double)image->data[x * width + y] *
                           cos(((2.0 * x + 1.0) * u * M_PI) / (2.0 * height)) *
                           cos(((2.0 * y + 1.0) * v * M_PI) / (2.0 * width));
                }
            }
            dct_coeffs[u * width + v] = get_alpha(u, height) * get_alpha(v, width) * sum;
        }
    }

    // Normalize DCT coefficients for visualization (e.g., log scale and 0-255)
    double max_val = 0.0;
    for (size_t i = 0; i < num_pixels; i++) {
        // Use absolute value for visualization, and log scale to make low frequencies visible
        double val = fabs(dct_coeffs[i]);
        if (val > max_val) {
            max_val = val;
        }
    }

    result.width = width;
    result.height = height;
    result.data = (unsigned char*)malloc(num_pixels);
    if (result.data != NULL) {
        if (max_val > 0) {
            for (size_t i = 0; i < num_pixels; i++) {
                // Apply log transformation for better visualization of dynamic range
                double normalized_val = log(1 + fabs(dct_coeffs[i])) / log(1 + max_val);
                result.data[i] = (unsigned char)(normalized_val * 255.0);
            }
        } else {
            memset(result.data, 0, num_pixels);
        }
    }

    free(dct_coeffs);
    return result;
}

// Helper function for 1D Haar DWT (in-place)
static void dwt_1d(double* data, int n) {
    if (n < 2) return;

    int half = n / 2;
    double* temp = (double*)malloc(n * sizeof(double));
    if (temp == NULL) return;

    for (int i = 0; i < half; i++) {
        temp[i] = (data[2 * i] + data[2 * i + 1]) / sqrt(2.0); // Average (low-pass)
        temp[i + half] = (data[2 * i] - data[2 * i + 1]) / sqrt(2.0); // Difference (high-pass)
    }

    memcpy(data, temp, n * sizeof(double));
    free(temp);

    // Recursively apply to the low-pass part
    dwt_1d(data, half);
}

grayscale_image_t dwt_grayscale(grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    size_t width = image->width;
    size_t height = image->height;
    size_t num_pixels = width * height;

    double* temp_data = (double*)malloc(num_pixels * sizeof(double));
    if (temp_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for DWT\n");
        return result;
    }
    for(size_t i = 0; i < num_pixels; i++) {
        temp_data[i] = (double)image->data[i];
    }

    // Apply 1D DWT to each row
    for (size_t y = 0; y < height; y++) {
        dwt_1d(&temp_data[y * width], width);
    }

    // Apply 1D DWT to each column
    double* col = (double*)malloc(height * sizeof(double));
    if(col == NULL) {
        free(temp_data);
        return result;
    }
    for (size_t x = 0; x < width; x++) {
        // Extract column
        for(size_t y = 0; y < height; y++) {
            col[y] = temp_data[y * width + x];
        }
        // Apply DWT
        dwt_1d(col, height);
        // Put column back
        for(size_t y = 0; y < height; y++) {
            temp_data[y * width + x] = col[y];
        }
    }
    free(col);

    // Normalize for visualization
    double max_val = 0.0;
    for (size_t i = 0; i < num_pixels; i++) {
        if (fabs(temp_data[i]) > max_val) {
            max_val = fabs(temp_data[i]);
        }
    }

    result.width = width;
    result.height = height;
    result.data = (unsigned char*)malloc(num_pixels);
    if (result.data != NULL) {
        if (max_val > 0) {
            for (size_t i = 0; i < num_pixels; i++) {
                result.data[i] = (unsigned char)((fabs(temp_data[i]) / max_val) * 255.0);
            }
        } else {
            memset(result.data, 0, num_pixels);
        }
    }

    free(temp_data);
    return result;
}

grayscale_image_t apply_frequency_filter(const grayscale_image_t* image, filter_type_t filter_type, double cutoff) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    size_t width = image->width;
    size_t height = image->height;
    size_t num_pixels = width * height;

    // 1. Forward DFT
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_pixels);
    fftw_complex* out_dft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_pixels);

    if (in == NULL || out_dft == NULL) {
        fftw_free(in);
        fftw_free(out_dft);
        return result;
    }

    for (size_t i = 0; i < num_pixels; i++) {
        in[i][0] = (double)image->data[i];
        in[i][1] = 0.0;
    }

    fftw_plan plan_forward = fftw_plan_dft_2d(height, width, in, out_dft, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan_forward);
    fftw_destroy_plan(plan_forward);

    // 2. Create filter mask
    double* filter_mask = (double*)malloc(sizeof(double) * num_pixels);
    if (filter_mask == NULL) {
        fftw_free(in);
        fftw_free(out_dft);
        return result;
    }

    double cutoff_sq = cutoff * cutoff;
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            double dist_sq = pow(y - height / 2.0, 2) + pow(x - width / 2.0, 2);
            double filter_value = 0.0;

            switch (filter_type) {
                case FILTER_IDEAL_LOWPASS:
                    filter_value = (dist_sq <= cutoff_sq) ? 1.0 : 0.0;
                    break;
                case FILTER_IDEAL_HIGHPASS:
                    filter_value = (dist_sq <= cutoff_sq) ? 0.0 : 1.0;
                    break;
                case FILTER_GAUSSIAN_LOWPASS:
                    filter_value = exp(-dist_sq / (2.0 * cutoff_sq));
                    break;
                case FILTER_GAUSSIAN_HIGHPASS:
                    filter_value = 1.0 - exp(-dist_sq / (2.0 * cutoff_sq));
                    break;
                default:
                    filter_value = 1.0; // Should not happen
                    break;
            }
            filter_mask[y * width + x] = filter_value;
        }
    }
    
    // Shift the mask so that the DC component is at (0,0)
    fft_shift(filter_mask, width, height);
    
    // 3. Apply filter
    for (size_t i = 0; i < num_pixels; i++) {
        out_dft[i][0] *= filter_mask[i];
        out_dft[i][1] *= filter_mask[i];
    }
    free(filter_mask);

    // 4. Inverse DFT
    fftw_complex* out_idft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_pixels);
    if(out_idft == NULL) {
        fftw_free(in);
        fftw_free(out_dft);
        return result;
    }
    fftw_plan plan_backward = fftw_plan_dft_2d(height, width, out_dft, out_idft, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan_backward);
    fftw_destroy_plan(plan_backward);

    // 5. Normalize
    result.width = width;
    result.height = height;
    result.data = (unsigned char*)malloc(num_pixels);
    if (result.data != NULL) {
        for (size_t i = 0; i < num_pixels; i++) {
            // Take the real part and normalize by the number of pixels
            double val = out_idft[i][0] / (double)num_pixels;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            result.data[i] = (unsigned char)val;
        }
    }

    // Cleanup
    fftw_free(in);
    fftw_free(out_dft);
    fftw_free(out_idft);

    return result;
}
