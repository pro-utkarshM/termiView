#include "minunit.h"
#include "../include/video_processing.h"
#include "../include/image_processing.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // For memcpy
#include <math.h>   // For pow, fabs

char *test_video_io();
char *test_motion_estimation();

char *test_video_io() {
    // Assuming a test video file exists in the assets directory
    const char* test_video_path = "assets/test_video.mp4";

    VideoContext* vid_ctx = open_video(test_video_path);
    mu_assert("Failed to open video context", vid_ctx != NULL);

    rgb_image_t frame;
    bool frame_read = read_video_frame(vid_ctx, &frame);
    mu_assert("Failed to read first video frame", frame_read == true);
    mu_assert("Frame width should be greater than 0", frame.width > 0);
    mu_assert("Frame height should be greater than 0", frame.height > 0);

    // Basic check: is data allocated?
    mu_assert("RGB frame red data should not be NULL", frame.r_data != NULL);
    mu_assert("RGB frame green data should not be NULL", frame.g_data != NULL);
    mu_assert("RGB frame blue data should not be NULL", frame.b_data != NULL);

    // Free the frame data
    free_rgb_image(&frame);

    close_video(vid_ctx);

    return 0;
}

char *test_motion_estimation() {
    // Test parameters
    int width = 32;
    int height = 32;
    int block_size = 8;
    int search_window = 4;
    int shift_x = 4;
    int shift_y = 2;

    // Create reference frame (simple gradient)
    grayscale_image_t reference_frame = { .width = width, .height = height };
    reference_frame.data = (unsigned char*)malloc(width * height);
    mu_assert("Reference frame allocation failed", reference_frame.data != NULL);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            reference_frame.data[y * width + x] = (unsigned char)(x + y);
        }
    }

    // Create current frame (shifted reference frame)
    grayscale_image_t current_frame = { .width = width, .height = height };
    current_frame.data = (unsigned char*)malloc(width * height);
    mu_assert("Current frame allocation failed", current_frame.data != NULL);
    memset(current_frame.data, 0, width * height); // Initialize with black
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int ref_x = x - shift_x;
            int ref_y = y - shift_y;
            if (ref_x >= 0 && ref_x < width && ref_y >= 0 && ref_y < height) {
                current_frame.data[y * width + x] = reference_frame.data[ref_y * width + ref_x];
            }
        }
    }

    // Estimate motion
    MotionVectorField* mv_field = estimate_motion(&current_frame, &reference_frame, block_size, search_window);
    mu_assert("MotionVectorField should not be NULL", mv_field != NULL);
    mu_assert("MotionVectorField should contain vectors", mv_field->num_vectors > 0);

    // Basic check on some motion vectors (assuming simple uniform motion)
    // This is a simplified check, a more robust test would iterate through all vectors
    // if (mv_field->num_vectors > 0) {
    //     mu_assert("First MV dx should be close to shift_x", abs(mv_field->vectors[0].dx - shift_x) < 2);
    //     mu_assert("First MV dy should be close to shift_y", abs(mv_field->vectors[0].dy - shift_y) < 2);
    // }

    // Compensate motion
    grayscale_image_t* compensated_frame = compensate_motion(&reference_frame, mv_field, block_size);
    mu_assert("Compensated frame should not be NULL", compensated_frame != NULL);
    mu_assert("Compensated frame width should match", compensated_frame->width == width);
    mu_assert("Compensated frame height should match", compensated_frame->height == height);

    // Compare compensated frame with current frame (expect low MSE)
    double mse = 0.0;
    for (int i = 0; i < width * height; i++) {
        mse += pow(current_frame.data[i] - compensated_frame->data[i], 2);
    }
    mse /= (width * height);
    printf("Motion Compensation MSE: %f\n", mse);
    mu_assert("Compensated frame should be close to current frame (MSE < 50)", mse < 50.0);


    // Cleanup
    free(reference_frame.data);
    free(current_frame.data);
    free_motion_vector_field(mv_field);
    free_grayscale_image(compensated_frame);
    free(compensated_frame);

    return 0;
}

char *all_tests() {
    mu_run_test(test_video_io);
    mu_run_test(test_motion_estimation);
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
