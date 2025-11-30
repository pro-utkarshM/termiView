#include "../include/compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For bool type

// Helper function to calculate frequencies of bytes in data
void calculate_frequencies(const unsigned char* data, size_t data_len, unsigned int* frequencies) {
    // Initialize frequencies to 0
    for (int i = 0; i < 256; i++) {
        frequencies[i] = 0;
    }

    // Calculate frequencies
    for (size_t i = 0; i < data_len; i++) {
        frequencies[data[i]]++;
    }
}

// Function to create a new Huffman node
static HuffmanNode* create_huffman_node(unsigned char data, unsigned int frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    if (node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for HuffmanNode\n");
        exit(EXIT_FAILURE);
    }
    node->data = data;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

// Min-Priority Queue for Huffman Nodes (implemented as a min-heap)
typedef struct {
    HuffmanNode** array;
    unsigned int size;
    unsigned int capacity;
} MinPriorityQueue;

// Function to create a min-priority queue
static MinPriorityQueue* create_min_priority_queue(unsigned int capacity) {
    MinPriorityQueue* pq = (MinPriorityQueue*)malloc(sizeof(MinPriorityQueue));
    if (pq == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for MinPriorityQueue\n");
        exit(EXIT_FAILURE);
    }
    pq->size = 0;
    pq->capacity = capacity;
    pq->array = (HuffmanNode**)malloc(pq->capacity * sizeof(HuffmanNode*));
    if (pq->array == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for MinPriorityQueue array\n");
        free(pq);
        exit(EXIT_FAILURE);
    }
    return pq;
}

// Function to swap two Huffman nodes
static void swap_huffman_nodes(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* t = *a;
    *a = *b;
    *b = t;
}

// Heapify function for min-heap
static void min_heapify(MinPriorityQueue* pq, unsigned int idx) {
    unsigned int smallest = idx;
    unsigned int left = 2 * idx + 1;
    unsigned int right = 2 * idx + 2;

    if (left < pq->size && pq->array[left]->frequency < pq->array[smallest]->frequency) {
        smallest = left;
    }

    if (right < pq->size && pq->array[right]->frequency < pq->array[smallest]->frequency) {
        smallest = right;
    }

    if (smallest != idx) {
        swap_huffman_nodes(&pq->array[smallest], &pq->array[idx]);
        min_heapify(pq, smallest);
    }
}

// Function to check if size of heap is 1
static int is_size_one(MinPriorityQueue* pq) {
    return (pq->size == 1);
}

// Function to extract the minimum value node from heap
static HuffmanNode* extract_min(MinPriorityQueue* pq) {
    HuffmanNode* temp = pq->array[0];
    pq->array[0] = pq->array[pq->size - 1];
    pq->size--;
    min_heapify(pq, 0);
    return temp;
}

// Function to insert a new node to Min Heap
static void insert_min_heap(MinPriorityQueue* pq, HuffmanNode* huffman_node) {
    pq->size++;
    int i = pq->size - 1;

    // Check if the heap needs to be reallocated
    if (pq->size > pq->capacity) {
        pq->capacity *= 2;
        pq->array = (HuffmanNode**)realloc(pq->array, pq->capacity * sizeof(HuffmanNode*));
        if (pq->array == NULL) {
            fprintf(stderr, "Error: Failed to reallocate memory for priority queue\n");
            exit(EXIT_FAILURE);
        }
    }

    while (i && huffman_node->frequency < pq->array[(i - 1) / 2]->frequency) {
        pq->array[i] = pq->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    pq->array[i] = huffman_node;
}

// Function to build a min-heap from an array of nodes
static void build_min_heap(MinPriorityQueue* pq) {
    int n = pq->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i) {
        min_heapify(pq, i);
    }
}

// Function to build the Huffman tree
HuffmanNode* build_huffman_tree(const unsigned int* frequencies) {
    HuffmanNode *left, *right, *top;

    MinPriorityQueue* pq = create_min_priority_queue(256); // Max 256 unique characters
    if (pq == NULL) return NULL;

    // Create a leaf node for each character and add to the priority queue.
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            insert_min_heap(pq, create_huffman_node((unsigned char)i, frequencies[i]));
        }
    }

    // Iterate while size of heap doesn't become 1
    while (pq->size > 1) {
        // Extract the two minimum frequency nodes from priority queue
        left = extract_min(pq);
        right = extract_min(pq);

        // Create a new internal node with frequency equal to the sum of the two nodes' frequencies.
        // Make the two extracted nodes as left and right children of this new node.
        // Add this new node to the min heap.
        // '$' is a special character for internal nodes (not used as actual data)
        top = create_huffman_node('$', left->frequency + right->frequency);
        top->left = left;
        top->right = right;
        insert_min_heap(pq, top);
    }

    // The remaining node is the root of the Huffman tree
    HuffmanNode* root = extract_min(pq);
    free(pq->array);
    free(pq);
    return root;
}

// Helper function to free Huffman tree nodes
static void free_huffman_tree_nodes(HuffmanNode* node) {
    if (node == NULL) {
        return;
    }
    free_huffman_tree_nodes(node->left);
    free_huffman_tree_nodes(node->right);
    free(node);
}

// Stores Huffman codes in huffman_codes_t structure
static void generate_codes_recursive(HuffmanNode* root, char* current_code, int top, unsigned char** codes_array, size_t* lengths_array) {
    // Assign 0 to left edge and recurse
    if (root->left) {
        current_code[top] = '0';
        generate_codes_recursive(root->left, current_code, top + 1, codes_array, lengths_array);
    }

    // Assign 1 to right edge and recurse
    if (root->right) {
        current_code[top] = '1';
        generate_codes_recursive(root->right, current_code, top + 1, codes_array, lengths_array);
    }

    // If this is a leaf node, then it contains one of the input
    // characters, store its code
    if (!root->left && !root->right) {
        current_code[top] = '\0'; // Null-terminate the string
        codes_array[root->data] = (unsigned char*)malloc(sizeof(unsigned char) * (top + 1));
        if (codes_array[root->data] == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for Huffman code\n");
            exit(EXIT_FAILURE);
        }
        strcpy((char*)codes_array[root->data], current_code);
        lengths_array[root->data] = top;
    }
}

huffman_codes_t* build_huffman_codes(const unsigned int* frequencies) {
    HuffmanNode* root = build_huffman_tree(frequencies);
    if (root == NULL) {
        return NULL;
    }

    huffman_codes_t* h_codes = (huffman_codes_t*)malloc(sizeof(huffman_codes_t));
    if (h_codes == NULL) {
        free_huffman_tree_nodes(root);
        return NULL;
    }
    h_codes->num_symbols = 256;
    h_codes->codes = (unsigned char**)calloc(256, sizeof(unsigned char*));
    h_codes->lengths = (size_t*)calloc(256, sizeof(size_t));

    if (h_codes->codes == NULL || h_codes->lengths == NULL) {
        // Free partially allocated memory before returning
        if (h_codes->codes != NULL) free(h_codes->codes);
        if (h_codes->lengths != NULL) free(h_codes->lengths);
        free(h_codes);
        free_huffman_tree_nodes(root);
        return NULL;
    }

    char current_code[256]; // Max code length for 256 symbols is 255 bits, plus null terminator
    generate_codes_recursive(root, current_code, 0, h_codes->codes, h_codes->lengths);
    
    free_huffman_tree_nodes(root); // Free the tree nodes after codes are generated

    return h_codes;
}

// Function to encode data using Huffman codes
unsigned char* huffman_encode(const unsigned char* data, size_t data_len, 
                              const huffman_codes_t* h_codes, size_t* encoded_len_bits) {
    if (data == NULL || h_codes == NULL || encoded_len_bits == NULL) {
        return NULL;
    }

    // Calculate total encoded bits
    *encoded_len_bits = 0;
    for (size_t i = 0; i < data_len; i++) {
        // Ensure there's a code for the symbol
        if ((char*)h_codes->codes[data[i]] == NULL) { // Cast to char* for comparison
            fprintf(stderr, "Error: No Huffman code found for symbol %d\n", data[i]);
            return NULL;
        }
        *encoded_len_bits += h_codes->lengths[data[i]];
    }

    // Allocate memory for encoded data (in bytes)
    size_t encoded_bytes_len = (*encoded_len_bits + 7) / 8; // Round up to nearest byte
    unsigned char* encoded_data = (unsigned char*)calloc(encoded_bytes_len, sizeof(unsigned char));
    if (encoded_data == NULL) {
        return NULL;
    }

    // Fill encoded data byte by byte
    int bit_offset = 0;
    for (size_t i = 0; i < data_len; i++) {
        unsigned char symbol = data[i];
        char* code = (char*)h_codes->codes[symbol]; // Cast to char*
        size_t code_len = h_codes->lengths[symbol];

        for (size_t j = 0; j < code_len; j++) {
            if (code[j] == '1') {
                // Set the bit at current position
                encoded_data[bit_offset / 8] |= (1 << (7 - (bit_offset % 8)));
            }
            bit_offset++;
        }
    }

    return encoded_data;
}

// Function to decode data using Huffman codes
unsigned char* huffman_decode(const unsigned char* encoded_data, size_t encoded_len_bits, 
                              HuffmanNode* huffman_tree_root, size_t* decoded_len) {
    if (encoded_data == NULL || huffman_tree_root == NULL || decoded_len == NULL) {
        return NULL;
    }

    size_t current_decoded_capacity = 1024; // Initial capacity
    unsigned char* decoded_data = (unsigned char*)malloc(current_decoded_capacity * sizeof(unsigned char));
    if (decoded_data == NULL) {
        return NULL;
    }
    *decoded_len = 0;

    HuffmanNode* current_node = huffman_tree_root;
    for (size_t i = 0; i < encoded_len_bits; i++) {
        // Traverse the Huffman tree based on bits
        unsigned char bit = (encoded_data[i / 8] >> (7 - (i % 8))) & 1;

        if (bit == 0) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }

        // If a leaf node is reached, append the character to decoded_data
        if (current_node->left == NULL && current_node->right == NULL) {
            if (*decoded_len >= current_decoded_capacity) {
                current_decoded_capacity *= 2;
                decoded_data = (unsigned char*)realloc(decoded_data, current_decoded_capacity * sizeof(unsigned char));
                if (decoded_data == NULL) {
                    return NULL;
                }
            }
            decoded_data[*decoded_len] = current_node->data;
            (*decoded_len)++;
            current_node = huffman_tree_root; // Reset to root for next character
        }
    }

    // Shrink to fit
    decoded_data = (unsigned char*)realloc(decoded_data, (*decoded_len) * sizeof(unsigned char));
    return decoded_data;
}

// Function to free Huffman codes
void free_huffman_codes(huffman_codes_t* codes) {
    if (codes != NULL) {
        if (codes->codes != NULL) {
            for (size_t i = 0; i < codes->num_symbols; i++) { // Changed int to size_t
                free((void*)codes->codes[i]); // Cast to void*
            }
            free(codes->codes);
        }
        free(codes->lengths);
        free(codes);
    }
}

// Function to free Huffman tree
void free_huffman_tree(HuffmanNode* node) {
    free_huffman_tree_nodes(node);
}

// Helper function to build cumulative frequencies
static void build_cumulative_frequencies(const unsigned int* frequencies, unsigned int* cumulative_frequencies, size_t num_symbols) {
    cumulative_frequencies[0] = 0;
    for (size_t i = 0; i < num_symbols; i++) {
        cumulative_frequencies[i + 1] = cumulative_frequencies[i] + frequencies[i];
    }
}

// Helper function to emit a bit for arithmetic encoding/decoding
static void emit_bit_helper(unsigned char** encoded_buffer, size_t* encoded_buffer_idx, size_t* buffer_capacity, int bit) {
    // Check if enough space is allocated for the current byte
    if ((*encoded_buffer_idx / 8) >= *buffer_capacity) {
        *buffer_capacity *= 2;
        // Reallocate with a larger size
        unsigned char* new_buffer = (unsigned char*)realloc(*encoded_buffer, *buffer_capacity * sizeof(unsigned char));
        if (new_buffer == NULL) {
            fprintf(stderr, "Error: Failed to reallocate memory for encoded buffer\n");
            exit(EXIT_FAILURE);
        }
        *encoded_buffer = new_buffer;
        // Initialize new memory to 0 to avoid garbage bits
        memset(*encoded_buffer + (*buffer_capacity / 2), 0, *buffer_capacity / 2);
    }
    
    if (bit == 1) {
        (*encoded_buffer)[*encoded_buffer_idx / 8] |= (1 << (7 - (*encoded_buffer_idx % 8)));
    } else {
        // Ensure the bit is 0 (it should already be if using calloc and not setting it)
        (*encoded_buffer)[*encoded_buffer_idx / 8] &= ~(1 << (7 - (*encoded_buffer_idx % 8)));
    }
    (*encoded_buffer_idx)++;
}

// Function to encode data using Arithmetic coding
unsigned char* arithmetic_encode(const unsigned char* data, size_t data_len, 
                                 const unsigned int* frequencies, size_t* encoded_len_bytes) {
    if (data == NULL || frequencies == NULL || encoded_len_bytes == NULL) {
        return NULL;
    }

    size_t num_symbols = 256; // Assuming 8-bit symbols
    unsigned int cumulative_frequencies[num_symbols + 1];
    build_cumulative_frequencies(frequencies, cumulative_frequencies, num_symbols);

    unsigned int total_freq = cumulative_frequencies[num_symbols];
    if (total_freq == 0) {
        *encoded_len_bytes = 0;
        return NULL;
    }

    // Initialize range and other variables
    unsigned int low = 0;
    unsigned int high = 0xFFFFFFFF;
    int bits_to_follow = 0;

    // Output buffer
    size_t buffer_capacity = 16;
    unsigned char* encoded_buffer = (unsigned char*)calloc(buffer_capacity, sizeof(unsigned char));
    if (encoded_buffer == NULL) return NULL;
    size_t encoded_buffer_idx = 0;

    // Main encoding loop
    for (size_t i = 0; i < data_len; i++) {
        unsigned char symbol = data[i];
        unsigned long long range = (unsigned long long)high - low + 1;

        // Update range
        high = low + (unsigned int)((range * cumulative_frequencies[symbol + 1]) / total_freq) - 1;
        low = low + (unsigned int)((range * cumulative_frequencies[symbol]) / total_freq);

        // Renormalization
        for (;;) {
            if (high < 0x80000000U) { // If high is in the lower half
                emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 0);
                while (bits_to_follow > 0) {
                    emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 1);
                    bits_to_follow--;
                }
            } else if (low >= 0x80000000U) { // If low is in the upper half
                emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 1);
                while (bits_to_follow > 0) {
                    emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 0);
                    bits_to_follow--;
                }
                low -= 0x80000000U;
                high -= 0x80000000U;
            } else if (low >= 0x40000000U && high < 0xC0000000U) { // Underflow
                bits_to_follow++;
                low -= 0x40000000U;
                high -= 0x40000000U;
            } else {
                break;
            }
            low <<= 1;
            high = (high << 1) | 1;
        }
    }

    // Flush remaining bits
    bits_to_follow++;
    if (low < 0x40000000U) {
        emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 0);
        for (int i = 0; i < bits_to_follow; i++) emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 1);
    } else {
        emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 1);
        for (int i = 0; i < bits_to_follow; i++) emit_bit_helper(&encoded_buffer, &encoded_buffer_idx, &buffer_capacity, 0);
    }
    
    *encoded_len_bytes = (encoded_buffer_idx + 7) / 8;
    // Shrink to fit
    encoded_buffer = (unsigned char*)realloc(encoded_buffer, *encoded_len_bytes);

    return encoded_buffer;
}

unsigned char* arithmetic_decode(const unsigned char* encoded_data, size_t encoded_len_bytes, 
                                 const unsigned int* frequencies, size_t data_len, size_t* decoded_len) {
    if (encoded_data == NULL || frequencies == NULL || decoded_len == NULL || data_len == 0) {
        return NULL;
    }

    size_t num_symbols = 256;
    unsigned int cumulative_frequencies[num_symbols + 1];
    build_cumulative_frequencies(frequencies, cumulative_frequencies, num_symbols);
    unsigned int total_freq = cumulative_frequencies[num_symbols];

    if (total_freq == 0) {
        *decoded_len = 0;
        return NULL;
    }

    // Allocate memory for decoded data
    unsigned char* decoded_data = (unsigned char*)malloc(data_len * sizeof(unsigned char));
    if (decoded_data == NULL) return NULL;
    *decoded_len = 0;

    // Initialize range, code value, and other variables
    unsigned int low = 0;
    unsigned int high = 0xFFFFFFFF;
    unsigned int code = 0;

    // Read initial bits to fill the code register
    size_t bit_idx = 0;
    for (int i = 0; i < 32; i++) {
        if (bit_idx < encoded_len_bytes * 8) {
            code = (code << 1) | ((encoded_data[bit_idx / 8] >> (7 - (bit_idx % 8))) & 1);
            bit_idx++;
        } else {
            code <<= 1;
        }
    }

    // Main decoding loop
    for (size_t i = 0; i < data_len; i++) {
        unsigned long long range = (unsigned long long)high - low + 1;
        unsigned int scaled_value = (unsigned int)((((unsigned long long)code - low + 1) * total_freq - 1) / range);

        // Find symbol based on scaled value
        int symbol = 0;
        for (; symbol < (int)num_symbols; symbol++) {
            if (cumulative_frequencies[symbol + 1] > scaled_value) {
                break;
            }
        }
        decoded_data[i] = (unsigned char)symbol;

        // Update range
        high = low + (unsigned int)((range * cumulative_frequencies[symbol + 1]) / total_freq) - 1;
        low = low + (unsigned int)((range * cumulative_frequencies[symbol]) / total_freq);

        // Renormalization
        for (;;) {
            if (high < 0x80000000U) {
                // Do nothing to low and high
            } else if (low >= 0x80000000U) {
                code -= 0x80000000U;
                low -= 0x80000000U;
                high -= 0x80000000U;
            } else if (low >= 0x40000000U && high < 0xC0000000U) {
                code -= 0x40000000U;
                low -= 0x40000000U;
                high -= 0x40000000U;
            } else {
                break;
            }
            low <<= 1;
            high = (high << 1) | 1;

            if (bit_idx < encoded_len_bytes * 8) {
                code = (code << 1) | ((encoded_data[bit_idx / 8] >> (7 - (bit_idx % 8))) & 1);
                bit_idx++;
            } else {
                code <<= 1;
            }
        }
    }
    *decoded_len = data_len;
    return decoded_data;
}
