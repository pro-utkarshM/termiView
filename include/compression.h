#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stddef.h>

// Huffman Tree Node structure
typedef struct HuffmanNode {
    unsigned char data;          // The actual byte value (only for leaf nodes)
    unsigned int frequency;      // Frequency of the byte
    struct HuffmanNode *left, *right; // Left and right children
} HuffmanNode;

// Structure to hold Huffman codes
typedef struct {
    unsigned char** codes; // Array of Huffman codes
    size_t* lengths;      // Lengths of corresponding codes
    size_t num_symbols;   // Number of unique symbols
} huffman_codes_t;


// Function to build Huffman tree and generate codes
huffman_codes_t* build_huffman_codes(const unsigned int* frequencies);

// Function to calculate frequencies
void calculate_frequencies(const unsigned char* data, size_t data_len, unsigned int* frequencies);

// Function to build Huffman tree
HuffmanNode* build_huffman_tree(const unsigned int* frequencies);

// Function to encode data using Huffman codes
unsigned char* huffman_encode(const unsigned char* data, size_t data_len, 
                              const huffman_codes_t* codes, size_t* encoded_len_bits);

// Function to decode data using Huffman codes
unsigned char* huffman_decode(const unsigned char* encoded_data, size_t encoded_len_bits, 
                              HuffmanNode* huffman_tree_root, size_t* decoded_len);

// Function to free Huffman codes
void free_huffman_codes(huffman_codes_t* codes);

// Function to free Huffman tree
void free_huffman_tree(HuffmanNode* node);

// Structures for Arithmetic Coding state
typedef struct {
    unsigned int low;
    unsigned int high;
    unsigned int range;
} ArithmeticEncoderState;

typedef struct {
    unsigned int value;
    unsigned int low;
    unsigned int high;
    unsigned int range;
} ArithmeticDecoderState;

// Function to encode data using Arithmetic coding
unsigned char* arithmetic_encode(const unsigned char* data, size_t data_len, 
                                 const unsigned int* frequencies, size_t* encoded_len_bytes);

// Function to decode data using Arithmetic coding
unsigned char* arithmetic_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, 
                                 const unsigned int* frequencies, size_t data_len, size_t* decoded_len);

// Function to encode data using LZW compression
unsigned char* lzw_encode(const unsigned char* data, size_t data_len, size_t* encoded_len_bytes);

// Function to decode data using LZW compression
unsigned char* lzw_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, size_t* decoded_len);

// Function to encode data using Run-Length Encoding
unsigned char* rle_encode(const unsigned char* data, size_t data_len, size_t* encoded_len_bytes);

// Function to decode data using Run-Length Encoding
unsigned char* rle_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, size_t* decoded_len);

#include <fftw3.h>
#include "image_processing.h"
#include "video_processing.h" // For MotionVectorField

// Functions for DCT based compression
void compute_dct_2d(const grayscale_image_t* image, double* out_coeffs);
void compute_idct_2d(double* in_coeffs, grayscale_image_t* out_image);

// Function to encode data using DCT-based compression
unsigned char* dct_based_encode(const grayscale_image_t* image, size_t* encoded_len_bytes);

// Function to decode data using DCT-based compression
grayscale_image_t* dct_based_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, size_t width, size_t height);

// Function to encode grayscale image using JPEG (simplified)
unsigned char* jpeg_encode(const grayscale_image_t* image, int quality, size_t* encoded_len_bytes);

// Functions for DWT based compression
void compute_dwt_2d(const grayscale_image_t* image, double* out_coeffs, int levels);
void compute_idwt_2d(double* in_coeffs, grayscale_image_t* out_image, int levels);

// Function to encode data using Wavelet-based compression
unsigned char* wavelet_encode(const grayscale_image_t* image, int levels, size_t* encoded_len_bytes);

// Function to decode data using Wavelet-based compression
grayscale_image_t* wavelet_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, size_t width, size_t height, int levels);

// Frame types for video compression
typedef enum {
    FRAME_TYPE_I, // Intra-coded frame
    FRAME_TYPE_P  // Predictive-coded frame
} FrameType;

// Structure to hold a coded frame
typedef struct {
    FrameType type;
    int original_width;
    int original_height;
    unsigned char* encoded_data;
    size_t encoded_data_len;
    // For P-frames, motion vectors would also be stored here
    // MotionVectorField* mv_field; // This would require video_processing.h
} CodedFrame;

// Functions for video predictive coding
// (These will be implemented in compression.c and rely on motion estimation from video_processing)

// Encodes a single frame (I-frame or P-frame)
CodedFrame* predictive_encode_frame(const grayscale_image_t* current_frame, const grayscale_image_t* reference_frame, FrameType type, int block_size, int search_window, int quality);

// Decodes a single frame (I-frame or P-frame)
grayscale_image_t* predictive_decode_frame(const CodedFrame* coded_frame, const grayscale_image_t* reference_frame, int block_size);

// Encodes a sequence of frames
unsigned char* video_predictive_encode(grayscale_image_t** frames, int num_frames, int block_size, int search_window, int quality, size_t* encoded_stream_len);

// Decodes a sequence of frames
grayscale_image_t** video_predictive_decode(const unsigned char* encoded_stream, size_t encoded_stream_len, int num_frames, int block_size, size_t* decoded_frames_count);

// Function to free a CodedFrame
void free_coded_frame(CodedFrame* frame);


#endif // COMPRESSION_H
