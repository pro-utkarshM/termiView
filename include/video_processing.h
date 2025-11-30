#ifndef VIDEO_PROCESSING_H
#define VIDEO_PROCESSING_H

#include <stddef.h>
#include "image_processing.h" // For rgb_image_t and grayscale_image_t

// Forward declarations for FFmpeg structs
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

// Structure to hold video stream information and context
typedef struct {
    struct AVFormatContext *fmt_ctx;
    struct AVCodecContext *codec_ctx;
    int video_stream_idx;
    int width;
    int height;
    int fps;
    struct SwsContext *sws_ctx; // For pixel format conversion
    struct AVFrame *frame;      // Reusable frame for decoding
    struct AVPacket *packet;    // Reusable packet for reading
} VideoContext;

// Function to initialize video context and open video file
VideoContext* open_video(const char* filename);

// Function to read a single frame from the video
// Returns true on success, false on EOF or error
bool read_video_frame(VideoContext* vid_ctx, rgb_image_t* out_rgb_frame);

// Function to close video context and free resources
void close_video(VideoContext* vid_ctx);

// Function to apply temporal averaging on a buffer of frames
grayscale_image_t temporal_average(grayscale_image_t** frames, int num_frames);

// Structure to represent a single motion vector
typedef struct {
    int block_x;
    int block_y;
    int dx;
    int dy;
} MotionVector;

// Structure to represent a field of motion vectors for a frame
typedef struct {
    MotionVector* vectors;
    int num_vectors;
} MotionVectorField;

// Function to estimate motion between two frames
MotionVectorField* estimate_motion(const grayscale_image_t* current_frame, const grayscale_image_t* reference_frame, int block_size, int search_window);

// Function to compensate motion in a frame using motion vectors
grayscale_image_t* compensate_motion(const grayscale_image_t* reference_frame, const MotionVectorField* mv_field, int block_size);

// Function to free MotionVectorField
void free_motion_vector_field(MotionVectorField* mv_field);

// Structure to represent optical flow for each pixel (vx, vy)
typedef struct {
    double vx;
    double vy;
} OpticalFlowVector;

// Structure to represent an entire optical flow field
typedef struct {
    OpticalFlowVector* flow_vectors;
    int width;
    int height;
} OpticalFlowField;

// Function to compute optical flow between two grayscale frames
OpticalFlowField* compute_optical_flow(const grayscale_image_t* frame1, const grayscale_image_t* frame2, int window_size);

// Function to free OpticalFlowField
void free_optical_flow_field(OpticalFlowField* flow_field);

#endif // VIDEO_PROCESSING_H
