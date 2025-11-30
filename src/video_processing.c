#include "../include/video_processing.h"
#include "../include/image_processing.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize FFmpeg and open video file
VideoContext* open_video(const char* filename) {
    VideoContext* vid_ctx = (VideoContext*)calloc(1, sizeof(VideoContext));
    if (!vid_ctx) {
        fprintf(stderr, "Error: Could not allocate VideoContext\n");
        return NULL;
    }

    vid_ctx->fmt_ctx = NULL;
    if (avformat_open_input(&vid_ctx->fmt_ctx, filename, NULL, NULL) < 0) {
        fprintf(stderr, "Error: Could not open source file %s\n", filename);
        free(vid_ctx);
        return NULL;
    }

    if (avformat_find_stream_info(vid_ctx->fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Error: Could not find stream information\n");
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    vid_ctx->video_stream_idx = -1;
    AVCodecParameters *codec_params = NULL;
    const AVCodec *codec = NULL;

    for (unsigned int i = 0; i < vid_ctx->fmt_ctx->nb_streams; i++) {
        codec_params = vid_ctx->fmt_ctx->streams[i]->codecpar;
        if (codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            vid_ctx->video_stream_idx = i;
            break;
        }
    }

    if (vid_ctx->video_stream_idx == -1) {
        fprintf(stderr, "Error: Could not find any video stream in the file\n");
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        fprintf(stderr, "Error: Unsupported codec!\n");
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    vid_ctx->codec_ctx = avcodec_alloc_context3(codec);
    if (!vid_ctx->codec_ctx) {
        fprintf(stderr, "Error: Could not allocate video codec context\n");
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    if (avcodec_parameters_to_context(vid_ctx->codec_ctx, codec_params) < 0) {
        fprintf(stderr, "Error: Could not copy codec parameters to context\n");
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    if (avcodec_open2(vid_ctx->codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Error: Could not open codec\n");
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    vid_ctx->width = vid_ctx->codec_ctx->width;
    vid_ctx->height = vid_ctx->codec_ctx->height;
    vid_ctx->fps = 0; // Will calculate if needed

    vid_ctx->frame = av_frame_alloc();
    if (!vid_ctx->frame) {
        fprintf(stderr, "Error: Could not allocate AVFrame\n");
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    vid_ctx->packet = av_packet_alloc();
    if (!vid_ctx->packet) {
        fprintf(stderr, "Error: Could not allocate AVPacket\n");
        av_frame_free(&vid_ctx->frame);
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    vid_ctx->sws_ctx = sws_getContext(
        vid_ctx->width, vid_ctx->height, vid_ctx->codec_ctx->pix_fmt,
        vid_ctx->width, vid_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL
    );
    if (!vid_ctx->sws_ctx) {
        fprintf(stderr, "Error: Could not allocate SwsContext\n");
        av_packet_free(&vid_ctx->packet);
        av_frame_free(&vid_ctx->frame);
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
        return NULL;
    }

    return vid_ctx;
}

// Read a single frame from the video
bool read_video_frame(VideoContext* vid_ctx, rgb_image_t* out_rgb_frame) {
    int response = 0;
    AVFrame *pFrameRGB = NULL;
    uint8_t *buffer = NULL;
    int num_bytes;

    while (av_read_frame(vid_ctx->fmt_ctx, vid_ctx->packet) >= 0) {
        if (vid_ctx->packet->stream_index == vid_ctx->video_stream_idx) {
            response = avcodec_send_packet(vid_ctx->codec_ctx, vid_ctx->packet);
            if (response < 0) {
                fprintf(stderr, "Error while sending a packet to the decoder\n");
                av_packet_unref(vid_ctx->packet);
                return false;
            }

            while (response >= 0) {
                response = avcodec_receive_frame(vid_ctx->codec_ctx, vid_ctx->frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    fprintf(stderr, "Error while receiving a frame from the decoder\n");
                    av_packet_unref(vid_ctx->packet);
                    return false;
                }

                // Convert the image from its native format to RGB
                pFrameRGB = av_frame_alloc();
                if (!pFrameRGB) {
                    fprintf(stderr, "Error: Could not allocate RGB AVFrame\n");
                    av_packet_unref(vid_ctx->packet);
                    return false;
                }

                num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, vid_ctx->width, vid_ctx->height, 1);
                buffer = (uint8_t *)av_malloc(num_bytes);
                if (!buffer) {
                    fprintf(stderr, "Error: Could not allocate buffer for RGB frame\n");
                    av_frame_free(&pFrameRGB);
                    av_packet_unref(vid_ctx->packet);
                    return false;
                }

                av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, vid_ctx->width, vid_ctx->height, 1);

                sws_scale(
                    vid_ctx->sws_ctx,
                    (uint8_t const * const *)vid_ctx->frame->data,
                    vid_ctx->frame->linesize,
                    0,
                    vid_ctx->height,
                    pFrameRGB->data,
                    pFrameRGB->linesize
                );

                // Populate out_rgb_frame
                out_rgb_frame->width = vid_ctx->width;
                out_rgb_frame->height = vid_ctx->height;
                out_rgb_frame->r_data = (unsigned char*)malloc(num_bytes / 3);
                out_rgb_frame->g_data = (unsigned char*)malloc(num_bytes / 3);
                out_rgb_frame->b_data = (unsigned char*)malloc(num_bytes / 3);

                if (!out_rgb_frame->r_data || !out_rgb_frame->g_data || !out_rgb_frame->b_data) {
                    fprintf(stderr, "Error: Could not allocate RGB data for out_rgb_frame\n");
                    free(out_rgb_frame->r_data); free(out_rgb_frame->g_data); free(out_rgb_frame->b_data);
                    av_free(buffer);
                    av_frame_free(&pFrameRGB);
                    av_packet_unref(vid_ctx->packet);
                    return false;
                }

                for (int y = 0; y < vid_ctx->height; y++) {
                    for (int x = 0; x < vid_ctx->width; x++) {
                        int rgb_idx = y * pFrameRGB->linesize[0] + x * 3;
                        int flat_idx = y * vid_ctx->width + x;
                        out_rgb_frame->r_data[flat_idx] = pFrameRGB->data[0][rgb_idx];
                        out_rgb_frame->g_data[flat_idx] = pFrameRGB->data[0][rgb_idx + 1];
                        out_rgb_frame->b_data[flat_idx] = pFrameRGB->data[0][rgb_idx + 2];
                    }
                }

                av_free(buffer);
                av_frame_free(&pFrameRGB);
                av_packet_unref(vid_ctx->packet);
                return true; // Frame successfully decoded
            }
        }
        av_packet_unref(vid_ctx->packet);
    }
    return false; // EOF or error
}

// Close video context and free resources
void close_video(VideoContext* vid_ctx) {
    if (vid_ctx) {
        sws_freeContext(vid_ctx->sws_ctx);
        av_packet_free(&vid_ctx->packet);
        av_frame_free(&vid_ctx->frame);
        avcodec_free_context(&vid_ctx->codec_ctx);
        avformat_close_input(&vid_ctx->fmt_ctx);
        free(vid_ctx);
    }
}

// Function to apply temporal averaging on a buffer of frames
grayscale_image_t temporal_average(grayscale_image_t** frames, int num_frames) {
    grayscale_image_t result = {0};
    if (frames == NULL || num_frames <= 0) {
        fprintf(stderr, "Error: Invalid input to temporal_average\n");
        return result;
    }

    size_t width = frames[0]->width;
    size_t height = frames[0]->height;
    size_t num_pixels = width * height;

    result.width = width;
    result.height = height;
    result.data = (unsigned char*)calloc(num_pixels, sizeof(unsigned char));
    if (result.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for temporal average image\n");
        result.width = 0;
        result.height = 0;
        return result;
    }

    for (size_t i = 0; i < num_pixels; i++) {
        double sum = 0;
        for (int j = 0; j < num_frames; j++) {
            sum += frames[j]->data[i];
        }
        result.data[i] = (unsigned char)round(sum / num_frames);
    }

    return result;
}

// Helper function to calculate Mean Absolute Difference (MAD) between two blocks
static double calculate_mad(const unsigned char* block1, int block1_stride,
                           const unsigned char* block2, int block2_stride,
                           int block_size) {
    double mad = 0.0;
    for (int y = 0; y < block_size; y++) {
        for (int x = 0; x < block_size; x++) {
            mad += abs(block1[y * block1_stride + x] - block2[y * block2_stride + x]);
        }
    }
    return mad / (block_size * block_size);
}

// Function to estimate motion between two frames using Block Matching
MotionVectorField* estimate_motion(const grayscale_image_t* current_frame, 
                                   const grayscale_image_t* reference_frame, 
                                   int block_size, int search_window) {
    if (current_frame == NULL || reference_frame == NULL ||
        current_frame->width != reference_frame->width ||
        current_frame->height != reference_frame->height ||
        block_size <= 0 || search_window < 0) {
        fprintf(stderr, "Error: Invalid input to estimate_motion\n");
        return NULL;
    }

    int width = current_frame->width;
    int height = current_frame->height;

    int num_blocks_x = (width + block_size - 1) / block_size;
    int num_blocks_y = (height + block_size - 1) / block_size;
    int total_blocks = num_blocks_x * num_blocks_y;

    MotionVectorField* mv_field = (MotionVectorField*)malloc(sizeof(MotionVectorField));
    if (mv_field == NULL) {
        fprintf(stderr, "Error: Failed to allocate MotionVectorField\n");
        return NULL;
    }
    mv_field->vectors = (MotionVector*)malloc(sizeof(MotionVector) * total_blocks);
    if (mv_field->vectors == NULL) {
        fprintf(stderr, "Error: Failed to allocate MotionVector array\n");
        free(mv_field);
        return NULL;
    }
    mv_field->num_vectors = total_blocks;

    int vector_idx = 0;
    for (int by = 0; by < num_blocks_y; by++) {
        for (int bx = 0; bx < num_blocks_x; bx++) {
            int current_block_x = bx * block_size;
            int current_block_y = by * block_size;

            double min_mad = -1.0;
            int best_dx = 0;
            int best_dy = 0;

            // Search area in reference frame
            int search_start_x = current_block_x - search_window;
            int search_end_x = current_block_x + search_window;
            int search_start_y = current_block_y - search_window;
            int search_end_y = current_block_y + search_window;

            // Clamp search area to frame boundaries
            if (search_start_x < 0) search_start_x = 0;
            if (search_start_y < 0) search_start_y = 0;
            if (search_end_x + block_size > width) search_end_x = width - block_size;
            if (search_end_y + block_size > height) search_end_y = height - block_size;


            // Extract current block data
            unsigned char* current_block_data = (unsigned char*)malloc(block_size * block_size);
            if (current_block_data == NULL) {
                // Handle error
                fprintf(stderr, "Error: Failed to allocate current_block_data\n");
                free(mv_field->vectors);
                free(mv_field);
                return NULL;
            }
            for (int y = 0; y < block_size; y++) {
                if (current_block_y + y < height && current_block_x < width) { // Ensure within bounds
                    memcpy(current_block_data + y * block_size, 
                           current_frame->data + (current_block_y + y) * width + current_block_x, 
                           block_size);
                } else { // Fill with padding if outside image bounds
                    memset(current_block_data + y * block_size, 0, block_size);
                }
            }


            for (int ref_y = search_start_y; ref_y <= search_end_y; ref_y++) {
                for (int ref_x = search_start_x; ref_x <= search_end_x; ref_x++) {
                    if (ref_x + block_size > width || ref_y + block_size > height) {
                        continue; // Skip if candidate block goes out of bounds
                    }

                    unsigned char* ref_block_data = (unsigned char*)malloc(block_size * block_size);
                    if (ref_block_data == NULL) {
                        // Handle error
                        fprintf(stderr, "Error: Failed to allocate ref_block_data\n");
                        free(current_block_data);
                        free(mv_field->vectors);
                        free(mv_field);
                        return NULL;
                    }
                    for (int y = 0; y < block_size; y++) {
                        memcpy(ref_block_data + y * block_size, 
                               reference_frame->data + (ref_y + y) * width + ref_x, 
                               block_size);
                    }

                    double mad = calculate_mad(current_block_data, block_size, ref_block_data, block_size, block_size);
                    
                    if (min_mad == -1.0 || mad < min_mad) {
                        min_mad = mad;
                        best_dx = ref_x - current_block_x;
                        best_dy = ref_y - current_block_y;
                    }
                    free(ref_block_data);
                }
            }
            free(current_block_data);

            mv_field->vectors[vector_idx].block_x = current_block_x;
            mv_field->vectors[vector_idx].block_y = current_block_y;
            mv_field->vectors[vector_idx].dx = best_dx;
            mv_field->vectors[vector_idx].dy = best_dy;
            vector_idx++;
        }
    }

    return mv_field;
}

// Function to compensate motion in a frame using motion vectors
grayscale_image_t* compensate_motion(const grayscale_image_t* reference_frame, const MotionVectorField* mv_field, int block_size) {
    if (reference_frame == NULL || mv_field == NULL || block_size <= 0) {
        fprintf(stderr, "Error: Invalid input to compensate_motion\n");
        return NULL;
    }

    int width = reference_frame->width;
    int height = reference_frame->height;

    grayscale_image_t* compensated_frame = (grayscale_image_t*)malloc(sizeof(grayscale_image_t));
    if (compensated_frame == NULL) {
        fprintf(stderr, "Error: Failed to allocate compensated_frame\n");
        return NULL;
    }
    compensated_frame->width = width;
    compensated_frame->height = height;
    compensated_frame->data = (unsigned char*)calloc(width * height, sizeof(unsigned char));
    if (compensated_frame->data == NULL) {
        fprintf(stderr, "Error: Failed to allocate compensated_frame data\n");
        free(compensated_frame);
        return NULL;
    }

    for (int i = 0; i < mv_field->num_vectors; i++) {
        MotionVector mv = mv_field->vectors[i];

        int ref_block_x = mv.block_x + mv.dx;
        int ref_block_y = mv.block_y + mv.dy;

        // Ensure block is within reference frame boundaries
        if (ref_block_x < 0) ref_block_x = 0;
        if (ref_block_y < 0) ref_block_y = 0;
        if (ref_block_x + block_size > width) ref_block_x = width - block_size;
        if (ref_block_y + block_size > height) ref_block_y = height - block_size;

        for (int y = 0; y < block_size; y++) {
            for (int x = 0; x < block_size; x++) {
                int current_frame_pixel_x = mv.block_x + x;
                int current_frame_pixel_y = mv.block_y + y;
                int ref_frame_pixel_x = ref_block_x + x;
                int ref_frame_pixel_y = ref_block_y + y;

                if (current_frame_pixel_x < width && current_frame_pixel_y < height &&
                    ref_frame_pixel_x < width && ref_frame_pixel_y < height) {
                    
                    compensated_frame->data[current_frame_pixel_y * width + current_frame_pixel_x] =
                        reference_frame->data[ref_frame_pixel_y * width + ref_frame_pixel_x];
                }
            }
        }
    }

    return compensated_frame;
}

// Function to compute optical flow between two grayscale frames (Lucas-Kanade)
OpticalFlowField* compute_optical_flow(const grayscale_image_t* frame1, const grayscale_image_t* frame2, int window_size) {
    if (frame1 == NULL || frame2 == NULL || frame1->width != frame2->width || frame1->height != frame2->height || window_size <= 0) {
        fprintf(stderr, "Error: Invalid input to compute_optical_flow\n");
        return NULL;
    }

    int width = frame1->width;
    int height = frame1->height;
    int half_window = window_size / 2;

    OpticalFlowField* flow_field = (OpticalFlowField*)malloc(sizeof(OpticalFlowField));
    if (flow_field == NULL) {
        fprintf(stderr, "Error: Failed to allocate OpticalFlowField\n");
        return NULL;
    }
    flow_field->width = width;
    flow_field->height = height;
    flow_field->flow_vectors = (OpticalFlowVector*)calloc(width * height, sizeof(OpticalFlowVector));
    if (flow_field->flow_vectors == NULL) {
        fprintf(stderr, "Error: Failed to allocate flow_vectors\n");
        free(flow_field);
        return NULL;
    }

    // Allocate memory for gradients
    double* Ix = (double*)calloc(width * height, sizeof(double));
    double* Iy = (double*)calloc(width * height, sizeof(double));
    double* It = (double*)calloc(width * height, sizeof(double));
    if (Ix == NULL || Iy == NULL || It == NULL) {
        fprintf(stderr, "Error: Failed to allocate gradient arrays\n");
        free(Ix); free(Iy); free(It);
        free(flow_field->flow_vectors); free(flow_field);
        return NULL;
    }

    // Calculate gradients Ix, Iy, It
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            size_t idx = y * width + x;
            // Ix (Sobel filter in X direction)
            Ix[idx] = (double)(frame1->data[idx + 1] - frame1->data[idx - 1]) / 2.0;
            // Iy (Sobel filter in Y direction)
            Iy[idx] = (double)(frame1->data[idx + width] - frame1->data[idx - width]) / 2.0;
            // It (Temporal gradient)
            It[idx] = (double)(frame2->data[idx] - frame1->data[idx]);
        }
    }

    // Compute flow vectors for each pixel
    for (int y = half_window; y < height - half_window; y++) {
        for (int x = half_window; x < width - half_window; x++) {
            double sum_Ix2 = 0, sum_Iy2 = 0, sum_IxIy = 0, sum_IxIt = 0, sum_IyIt = 0;

            // Sum gradients over the window
            for (int wy = -half_window; wy <= half_window; wy++) {
                for (int wx = -half_window; wx <= half_window; wx++) {
                    size_t idx = (y + wy) * width + (x + wx);
                    sum_Ix2 += Ix[idx] * Ix[idx];
                    sum_Iy2 += Iy[idx] * Iy[idx];
                    sum_IxIy += Ix[idx] * Iy[idx];
                    sum_IxIt += Ix[idx] * It[idx];
                    sum_IyIt += Iy[idx] * It[idx];
                }
            }

            // Construct and solve the 2x2 system:
            // [ Gxx Gxy ] [ vx ] = [ -bxx ]
            // [ Gyx Gyy ] [ vy ] = [ -byy ]
            // Where Gxx = sum_Ix2, Gxy = sum_IxIy, Gyy = sum_Iy2
            // bxx = sum_IxIt, byy = sum_IyIt

            double det = sum_Ix2 * sum_Iy2 - sum_IxIy * sum_IxIy;

            if (fabs(det) < 1e-6) { // Avoid division by zero for ill-conditioned matrices
                flow_field->flow_vectors[y * width + x].vx = 0.0;
                flow_field->flow_vectors[y * width + x].vy = 0.0;
            } else {
                flow_field->flow_vectors[y * width + x].vx = (-sum_Ix2 * sum_IyIt + sum_IxIy * sum_IxIt) / det;
                flow_field->flow_vectors[y * width + x].vy = (sum_IxIy * sum_IyIt - sum_Iy2 * sum_IxIt) / det;
            }
        }
    }

    free(Ix); free(Iy); free(It);
    return flow_field;
}

// Function to free MotionVectorField
void free_motion_vector_field(MotionVectorField* mv_field) {
    if (mv_field) {
        free(mv_field->vectors);
        free(mv_field);
    }
}

// Function to free OpticalFlowField
void free_optical_flow_field(OpticalFlowField* flow_field) {
    if (flow_field) {
        free(flow_field->flow_vectors);
        free(flow_field);
    }
}
