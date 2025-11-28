#include "../include/frequency.h"
#include <fftw3.h>
#include <math.h>
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
