#include "minunit.h"
#include "image_processing.h"
#include <stdlib.h>

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

    mu_assert_int_eq(width, noisy_image.width);
    mu_assert_int_eq(height, noisy_image.height);

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

char *all_tests() {
    mu_run_test(test_quantize_grayscale);
    mu_run_test(test_apply_salt_pepper_noise);
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
