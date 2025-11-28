#include "minunit.h"
#include "frequency.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

    mu_assert_int_eq(width, dwt_image.width);
    mu_assert_int_eq(height, dwt_image.height);

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

    mu_assert_int_eq(width, dct_image.width);
    mu_assert_int_eq(height, dct_image.height);

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