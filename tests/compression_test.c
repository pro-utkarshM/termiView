#include "minunit.h"
#include "../include/compression.h"
#include <stdlib.h>
#include <string.h>

// Forward declarations
char *test_huffman_compression_decompression();
char *test_arithmetic_compression_decompression();
char *test_lzw_compression_decompression();
char *test_rle_compression_decompression();
char *test_dct_based_compression_decompression();

char *test_huffman_compression_decompression() {
    // Sample data
    unsigned char original_data[] = "this is an example for huffman encoding";
    size_t original_len = strlen((char*)original_data);

    // 1. Calculate frequencies
    unsigned int frequencies[256];
    calculate_frequencies(original_data, original_len, frequencies);

    // 2. Build Huffman codes
    huffman_codes_t* h_codes = build_huffman_codes(frequencies);
    mu_assert("Huffman codes should not be NULL", h_codes != NULL);

    // Build Huffman tree for decoding (needed because h_codes doesn't directly contain the tree structure)
    // This is a bit redundant but necessary for the current huffman_decode signature
    HuffmanNode* h_tree_root = build_huffman_tree(frequencies);
    mu_assert("Huffman tree root should not be NULL", h_tree_root != NULL);


    // 3. Encode data
    size_t encoded_len_bits;
    unsigned char* encoded_data = huffman_encode(original_data, original_len, h_codes, &encoded_len_bits);
    mu_assert("Encoded data should not be NULL", encoded_data != NULL);
    mu_assert("Encoded length in bits should be greater than 0", encoded_len_bits > 0);

    // 4. Decode data
    size_t decoded_len;
    unsigned char* decoded_data = huffman_decode(encoded_data, encoded_len_bits, h_tree_root, &decoded_len);
    mu_assert("Decoded data should not be NULL", decoded_data != NULL);
    mu_assert("Decoded length should match original length", decoded_len == original_len);
    mu_assert("Decoded data should match original data", memcmp(original_data, decoded_data, original_len) == 0);

    // 5. Free allocated memory
    free(encoded_data);
    free(decoded_data);
    free_huffman_codes(h_codes);
    free_huffman_tree(h_tree_root);

    return 0;
}

char *test_lzw_compression_decompression() {
    unsigned char original_data[] = "TOBEORNOTTOBEORTOBEORNOT";
    size_t original_len = strlen((char*)original_data);

    // Encode
    size_t encoded_len_bytes;
    unsigned char* encoded_data = lzw_encode(original_data, original_len, &encoded_len_bytes);
    mu_assert("LZW encoded data should not be NULL", encoded_data != NULL);
    mu_assert("LZW encoded length should be greater than 0", encoded_len_bytes > 0);

    // Decode
    size_t decoded_len;
    unsigned char* decoded_data = lzw_decode(encoded_data, encoded_len_bytes, &decoded_len);
    mu_assert("LZW decoded data should not be NULL", decoded_data != NULL);
    mu_assert("LZW decoded length should match original length", decoded_len == original_len);
    mu_assert("LZW decoded data should match original data", memcmp(original_data, decoded_data, original_len) == 0);

    free(encoded_data);
    free(decoded_data);

    return 0;
}

char *test_rle_compression_decompression() {
    unsigned char original_data[] = "AAAABBBCCDAA";
    size_t original_len = strlen((char*)original_data);

    // Encode
    size_t encoded_len_bytes;
    unsigned char* encoded_data = rle_encode(original_data, original_len, &encoded_len_bytes);
    mu_assert("RLE encoded data should not be NULL", encoded_data != NULL);
    mu_assert("RLE encoded length should be greater than 0", encoded_len_bytes > 0);

    // Decode
    size_t decoded_len;
    unsigned char* decoded_data = rle_decode(encoded_data, encoded_len_bytes, &decoded_len);
    mu_assert("RLE decoded data should not be NULL", decoded_data != NULL);
    mu_assert("RLE decoded length should match original length", decoded_len == original_len);
    mu_assert("RLE decoded data should match original data", memcmp(original_data, decoded_data, original_len) == 0);

    free(encoded_data);
    free(decoded_data);

    return 0;
}

char *test_dct_based_compression_decompression() {
    int width = 8;
    int height = 8;
    grayscale_image_t original_image = { .width = width, .height = height };
    original_image.data = (unsigned char*)malloc(width * height);
    mu_assert("Original image data allocation failed", original_image.data != NULL);

    // Create a simple gradient image for testing
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            original_image.data[i * width + j] = (unsigned char)(i * 32 + j * 4);
        }
    }

    // Encode
    size_t encoded_len_bytes;
    unsigned char* encoded_data = dct_based_encode(&original_image, &encoded_len_bytes);
    mu_assert("DCT-based encoded data should not be NULL", encoded_data != NULL);
    mu_assert("DCT-based encoded length should be greater than 0", encoded_len_bytes > 0);

    // Decode
    grayscale_image_t* decoded_image = dct_based_decode(encoded_data, encoded_len_bytes, width, height);
    mu_assert("DCT-based decoded image should not be NULL", decoded_image != NULL);
    mu_assert("Decoded image width should match original", decoded_image->width == original_image.width);
    mu_assert("Decoded image height should match original", decoded_image->height == original_image.height);

    // Compare original and decoded images (allowing for some loss due to quantization)
    double mse = 0.0;
    for (int i = 0; i < width * height; i++) {
        mse += pow(original_image.data[i] - decoded_image->data[i], 2);
    }
    mse /= (width * height);
    printf("DCT MSE: %f\n", mse);
    // A small MSE indicates good reconstruction. The threshold can be adjusted.
    mu_assert("DCT-based decoded image should be close to original (MSE < 6000)", mse < 6000.0);

    // Cleanup
    free(original_image.data);
    free(encoded_data);
    free_grayscale_image(decoded_image);
    free(decoded_image);

    return 0;
}

char *all_tests() {
    mu_run_test(test_huffman_compression_decompression);
    mu_run_test(test_arithmetic_compression_decompression);
    mu_run_test(test_lzw_compression_decompression);
    mu_run_test(test_rle_compression_decompression);
    mu_run_test(test_dct_based_compression_decompression);
    return 0;
}

char *test_arithmetic_compression_decompression() {
    unsigned char original_data[] = "ABAABCAABCACBA";
    size_t original_len = strlen((char*)original_data);

    unsigned int frequencies[256] = {0};
    calculate_frequencies(original_data, original_len, frequencies);

    // Encode
    size_t encoded_len_bytes;
    unsigned char* encoded_data = arithmetic_encode(original_data, original_len, frequencies, &encoded_len_bytes);
    mu_assert("Arithmetic encoded data should not be NULL", encoded_data != NULL);
    mu_assert("Arithmetic encoded length should be greater than 0", encoded_len_bytes > 0);

    // Decode
    size_t decoded_len;
    unsigned char* decoded_data = arithmetic_decode(encoded_data, encoded_len_bytes, frequencies, original_len, &decoded_len);
    mu_assert("Arithmetic decoded data should not be NULL", decoded_data != NULL);
    mu_assert("Arithmetic decoded length should match original length", decoded_len == original_len);
    mu_assert("Arithmetic decoded data should match original data", memcmp(original_data, decoded_data, original_len) == 0);

    free(encoded_data);
    free(decoded_data);

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
