#include "minunit.h"
#include "image_processing.h"
#include <stdlib.h>

char *test_otsu_thresholding();
char *test_adaptive_thresholding();

char *test_equalize_histogram() {
    int width = 2;
    int height = 2;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));
    image_data[0] = 0;
    image_data[1] = 64;
    image_data[2] = 128;
    image_data[3] = 192;

    grayscale_image_t image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    equalize_histogram(&image);

    mu_assert_int_eq(0, image.data[0]);
    mu_assert_int_eq(85, image.data[1]);
    mu_assert_int_eq(170, image.data[2]);
    mu_assert_int_eq(255, image.data[3]);

    free(image_data);
    return 0;
}

char *test_calculate_histogram() {
    int width = 2;
    int height = 2;
    unsigned char image_data[] = {10, 20, 10, 30};
    grayscale_image_t image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    int histogram[256] = {0};
    calculate_histogram(&image, histogram);

    mu_assert_int_eq(2, histogram[10]);
    mu_assert_int_eq(1, histogram[20]);
    mu_assert_int_eq(1, histogram[30]);
    mu_assert_int_eq(0, histogram[0]); // Check some other values
    mu_assert_int_eq(0, histogram[255]);

    return 0;
}

char *test_apply_salt_pepper_noise() {
    srand(0); // Seed for deterministic testing

    int width = 10;
    int height = 10;
    size_t num_pixels = width * height;
    unsigned char *original_data = malloc(num_pixels * sizeof(unsigned char));
    for (size_t i = 0; i < num_pixels; i++) {
        original_data[i] = 128; // Initialize with a mid-gray value
    }

    grayscale_image_t original_image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = original_data
    };

    float density = 0.2f; // 20% noise
    grayscale_image_t noisy_image = apply_salt_pepper_noise(&original_image, density);

    mu_assert_int_eq((int)width, (int)noisy_image.width);
    mu_assert_int_eq((int)height, (int)noisy_image.height);

    int salt_count = 0;
    int pepper_count = 0;
    int unchanged_count = 0;

    for (size_t i = 0; i < num_pixels; i++) {
        if (noisy_image.data[i] == 0) {
            pepper_count++;
        } else if (noisy_image.data[i] == 255) {
            salt_count++;
        } else {
            unchanged_count++;
            mu_assert_int_eq(original_image.data[i], noisy_image.data[i]);
        }
    }

    // Expected values are approximate due to randomness, even with fixed seed
    // For density 0.2, expect around 10% salt and 10% pepper
    // (density / 2) * num_pixels = 0.1 * 100 = 10
    // We'll allow a range for the counts
    mu_assert(salt_count > 5 && salt_count < 15, "Salt count out of expected range");
    mu_assert(pepper_count > 5 && pepper_count < 15, "Pepper count out of expected range");
    mu_assert(unchanged_count > 70 && unchanged_count < 90, "Unchanged count out of expected range");

    free(original_data);
    free(noisy_image.data);
    return 0;
}

char *test_quantize_grayscale() {
    int width = 2;
    int height = 2;
    unsigned char *image = malloc(width * height * sizeof(unsigned char));
    image[0] = 50;
    image[1] = 100;
    image[2] = 150;
    image[3] = 200;

    int levels = 4;
    quantize_grayscale(image, width, height, levels);

    // Expected values after quantization
    // With 4 levels, the step is 256 / 4 = 64
    // The new values are floor(old_value / 64) * 64 + 32 (for midpoint)
    // val = floor(val / step) * step + step / 2
    // 50 -> floor(50/64)*64 + 32 = 32
    // 100 -> floor(100/64)*64 + 32 = 96
    // 150 -> floor(150/64)*64 + 32 = 160
    // 200 -> floor(200/64)*64 + 32 = 224

    mu_assert_int_eq(32, image[0]);
    mu_assert_int_eq(96, image[1]);
    mu_assert_int_eq(160, image[2]);
    mu_assert_int_eq(224, image[3]);

    free(image);
    return 0;
}

char *test_adaptive_thresholding() {
    int width = 5;
    int height = 5;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));

    // Create a gradient image
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            image_data[y * width + x] = (unsigned char)((y * width + x) * 10);
        }
    }

    grayscale_image_t image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    grayscale_image_t thresholded_image = apply_adaptive_thresholding(&image, 3, 5);

    // Assert that the output image is not null
    mu_assert("Adaptive thresholding image data is null", thresholded_image.data != NULL);

    // Check a few pixels
    // The top-left corner should be black, and the bottom-right should be white
    mu_assert("Adaptive: pixel (0,0) should be black", thresholded_image.data[0] == 0);
    mu_assert("Adaptive: pixel (4,4) should be white", thresholded_image.data[num_pixels-1] == 255);


    free(image_data);
    free(thresholded_image.data);
    return 0;
}

char *all_tests() {
    mu_run_test(test_quantize_grayscale);
    mu_run_test(test_apply_salt_pepper_noise);
    mu_run_test(test_calculate_histogram);
    mu_run_test(test_equalize_histogram);
    mu_run_test(test_otsu_thresholding);
    mu_run_test(test_adaptive_thresholding);
    return 0;
}

char *test_otsu_thresholding() {
    int width = 4;
    int height = 1;
    size_t num_pixels = width * height;
    unsigned char *image_data = malloc(num_pixels * sizeof(unsigned char));
    // Bimodal image
    image_data[0] = 10;
    image_data[1] = 20;
    image_data[2] = 200;
    image_data[3] = 210;

    grayscale_image_t image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    grayscale_image_t thresholded_image = apply_otsu_thresholding(&image);

    // The threshold should be between 20 and 200.
    // Let's check the result.
    mu_assert("Otsu: pixel 0 should be black", thresholded_image.data[0] == 0);
    mu_assert("Otsu: pixel 1 should be black", thresholded_image.data[1] == 0);
    mu_assert("Otsu: pixel 2 should be white", thresholded_image.data[2] == 255);
    mu_assert("Otsu: pixel 3 should be white", thresholded_image.data[3] == 255);

    free(image_data);
    free(thresholded_image.data);
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
