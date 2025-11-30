#include "minunit.h"
#include "../include/video_processing.h"
#include "../include/image_processing.h"
#include <stdbool.h>
#include <stdio.h>

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

char *all_tests() {
    mu_run_test(test_video_io);
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
