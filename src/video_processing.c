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
