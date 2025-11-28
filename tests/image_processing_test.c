#include "minunit.h"
#include "image_processing.h"
#include <stdlib.h>

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
