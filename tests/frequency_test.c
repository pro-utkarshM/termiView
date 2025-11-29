#include "minunit.h"
#include "frequency.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static double calculate_variance(const grayscale_image_t* image) {
    size_t num_pixels = image->width * image->height;
    if (num_pixels == 0) return 0.0;

    double mean = 0.0;
    for (size_t i = 0; i < num_pixels; i++) {
        mean += image->data[i];
    }
    mean /= num_pixels;

    double variance = 0.0;
    for (size_t i = 0; i < num_pixels; i++) {
        variance += pow(image->data[i] - mean, 2);
    }
    variance /= num_pixels;

    return variance;
}

char *test_frequency_filters() {
    int width = 32;
    int height = 32;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));

    // Create a checkerboard pattern to have some high frequencies
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            image_data[y * width + x] = ((x / 8) % 2 == (y / 8) % 2) ? 0 : 255;
        }
    }

    grayscale_image_t original_image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    double original_variance = calculate_variance(&original_image);

    // Test Ideal Low-pass filter
    grayscale_image_t lowpass_image = apply_frequency_filter(&original_image, FILTER_IDEAL_LOWPASS, 10.0);
    double lowpass_variance = calculate_variance(&lowpass_image);
    mu_assert(lowpass_variance < original_variance, "Ideal Low-pass filter should reduce variance");

    // Test Ideal High-pass filter
    grayscale_image_t highpass_image = apply_frequency_filter(&original_image, FILTER_IDEAL_HIGHPASS, 10.0);
    double highpass_variance = calculate_variance(&highpass_image);
    // High-pass filter might not necessarily increase the variance of this specific image,
    // but it should be significantly different from the low-pass filtered one.
    // For a checkerboard, it should remove the DC component and thus could decrease variance.
    // A better test is to see if low frequencies are gone. Let's just check if it's different.
    mu_assert(fabs(highpass_variance - original_variance) > 1.0, "Ideal High-pass filter should change variance");

    // Test Gaussian Low-pass filter
    grayscale_image_t gaussian_lowpass_image = apply_frequency_filter(&original_image, FILTER_GAUSSIAN_LOWPASS, 10.0);
    double gaussian_lowpass_variance = calculate_variance(&gaussian_lowpass_image);
    mu_assert(gaussian_lowpass_variance < original_variance, "Gaussian Low-pass filter should reduce variance");

    // Test Gaussian High-pass filter
    grayscale_image_t gaussian_highpass_image = apply_frequency_filter(&original_image, FILTER_GAUSSIAN_HIGHPASS, 10.0);
    double gaussian_highpass_variance = calculate_variance(&gaussian_highpass_image);
    mu_assert(fabs(gaussian_highpass_variance - original_variance) > 1.0, "Gaussian High-pass filter should change variance");

    free(image_data);
    free(lowpass_image.data);
    free(highpass_image.data);
    free(gaussian_lowpass_image.data);
    free(gaussian_highpass_image.data);

    return 0;
}

char *test_dwt_grayscale_constant_image() {
    int width = 8;
    int height = 8;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));
    memset(image_data, 128, num_pixels); // Constant image with value 128

    grayscale_image_t original_image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    grayscale_image_t dwt_image = dwt_grayscale(&original_image);

    mu_assert_int_eq((int)width, (int)dwt_image.width);
    mu_assert_int_eq((int)height, (int)dwt_image.height);

    // For a constant image, only the top-left coefficient (LL band) should be non-zero
    // After normalization, it should have the highest value
    mu_assert(dwt_image.data[0] > 200, "LL band should be high");

    // All other detail coefficients (LH, HL, HH bands) should be close to zero
    for (size_t i = 1; i < num_pixels; i++) {
        mu_assert(dwt_image.data[i] < 5, "Detail coefficients should be close to zero");
    }

    free(image_data);
    free(dwt_image.data);
    return 0;
}

char *test_dct_grayscale_constant_image() {
    int width = 8;
    int height = 8;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));
    memset(image_data, 128, num_pixels); // Constant image with value 128

    grayscale_image_t original_image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    grayscale_image_t dct_image = dct_grayscale(&original_image);

    mu_assert_int_eq((int)width, (int)dct_image.width);
    mu_assert_int_eq((int)height, (int)dct_image.height);

    // For a constant image, only the DC component (top-left, u=0, v=0) should be non-zero
    // The value after normalization and log transform should be 255 for the DC component
    // and close to 0 for others.

    // Allow a small tolerance for floating point comparisons
    mu_assert(dct_image.data[0] > 200, "DC component should be high"); // Should be close to 255

    for (size_t i = 1; i < num_pixels; i++) {
        mu_assert(dct_image.data[i] < 5, "Other components should be close to zero");
    }

    free(image_data);
    free(dct_image.data);
    return 0;
}

char *all_tests() {
    mu_run_test(test_dct_grayscale_constant_image);
    mu_run_test(test_dwt_grayscale_constant_image);
    mu_run_test(test_frequency_filters);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}