#include "minunit.h"
#include "../include/filters.h"
#include "../include/image_processing.h"
#include <stdlib.h>

char *test_canny_edge_detector() {
    int width = 10;
    int height = 10;
    size_t num_pixels = width * height;
    unsigned char *image_data = calloc(num_pixels, sizeof(unsigned char));

    // Create a simple square in the middle of the image
    for (int y = 3; y < 7; y++) {
        for (int x = 3; x < 7; x++) {
            image_data[y * width + x] = 255;
        }
    }

    grayscale_image_t image = {
        .width = (size_t)width,
        .height = (size_t)height,
        .data = image_data
    };

    grayscale_image_t canny_image = apply_canny_edge_detection(&image, 1.4f, 0.1f, 0.3f);

    // Assert that the output image is not null
    mu_assert("Canny image data is null", canny_image.data != NULL);

    // Count the number of edge pixels
    int edge_pixels = 0;
    for (size_t i = 0; i < num_pixels; i++) {
        if (canny_image.data[i] > 0) {
            edge_pixels++;
        }
    }

    // Assert that there are some edge pixels
    mu_assert("No edge pixels detected", edge_pixels > 0);

    // A more specific assertion could be to check for edges around the square.
    // For example, check that pixels at (2,3), (3,2), (7,6), (6,7) are edges.
    // This requires a very predictable canny implementation. The current check is a good start.
    
    free(image_data);
    free(canny_image.data);
    return 0;
}

char *all_tests() {
    mu_run_test(test_canny_edge_detector);
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
