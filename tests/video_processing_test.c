#include "minunit.h"
#include "../include/video_processing.h"
#include "../include/image_processing.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // For memcpy
#include <math.h>   // For pow, fabs

char *test_video_io();
char *test_motion_estimation();
char *test_optical_flow();

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

char *test_optical_flow() {
    int width = 32;
    int height = 32;
    int window_size = 5;
    double shift_x = 2.0; // Sub-pixel shift to test accuracy
    double shift_y = 1.0;

    grayscale_image_t frame1 = { .width = width, .height = height };
    frame1.data = (unsigned char*)malloc(width * height);
    mu_assert("Frame1 allocation failed", frame1.data != NULL);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            frame1.data[y * width + x] = (unsigned char)(x + y);
        }
    }

    grayscale_image_t frame2 = { .width = width, .height = height };
    frame2.data = (unsigned char*)malloc(width * height);
    mu_assert("Frame2 allocation failed", frame2.data != NULL);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Apply a simple shift
            int orig_x = (int)(x - shift_x);
            int orig_y = (int)(y - shift_y);
            if (orig_x >= 0 && orig_x < width && orig_y >= 0 && orig_y < height) {
                frame2.data[y * width + x] = frame1.data[orig_y * width + orig_x];
            } else {
                frame2.data[y * width + x] = 0; // Fill with black
            }
        }
    }

    OpticalFlowField* flow_field = compute_optical_flow(&frame1, &frame2, window_size);
    mu_assert("OpticalFlowField should not be NULL", flow_field != NULL);
    mu_assert("Flow field width should match", flow_field->width == width);
    mu_assert("Flow field height should match", flow_field->height == height);

    // Verify flow vectors in a central region (away from boundaries)
    int test_region_x = width / 2;
    int test_region_y = height / 2;
    size_t idx = test_region_y * width + test_region_x;

    printf("Optical Flow (center pixel): vx=%.2f, vy=%.2f\n", flow_field->flow_vectors[idx].vx, flow_field->flow_vectors[idx].vy);

    // Allow for some error due to approximation and integer pixel shifts
    mu_assert("Center vx should be close to shift_x", fabs(flow_field->flow_vectors[idx].vx - shift_x) < 2.0);
    mu_assert("Center vy should be close to shift_y", fabs(flow_field->flow_vectors[idx].vy - shift_y) < 2.0);

    free(frame1.data);
    free(frame2.data);
    free_optical_flow_field(flow_field);

    return 0;
}

char *all_tests() {
    mu_run_test(test_video_io);
    mu_run_test(test_motion_estimation);
    mu_run_test(test_optical_flow);
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
