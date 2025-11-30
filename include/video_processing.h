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

#endif // VIDEO_PROCESSING_H
