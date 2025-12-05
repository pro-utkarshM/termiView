#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "../include/image_processing.h"
#include "../include/stb_image_write.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LEVEL_CHARS " .-=+*x#$&X@"
#define N_LEVELS 12


grayscale_image_t load_image_as_grayscale(const char* file_path) {
    int width, height, _;
    unsigned char* data = stbi_load(file_path, &width, &height, &_, 1);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to load image '%s': %s\n", file_path, stbi_failure_reason());
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    return (grayscale_image_t) {
        .width = (size_t) width,
        .height = (size_t) height,
        .data = data
    };
}


unsigned char get_average(grayscale_image_t* image, size_t x1, size_t x2, size_t y1, size_t y2) {
    double total = 0;

    for (size_t x = x1; x < x2; x++) {
        for (size_t y = y1; y < y2; y++) {
            total += (double) image->data[x + y * image->width];
        }
    }

    double n = (x2 - x1) * (y2 - y1);

    return (char) (total / n);
}


grayscale_image_t make_resized_grayscale(grayscale_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method) {
    size_t width, height;

    size_t proposed_height = (original->height * max_width) / (2 * original->width);
    if (proposed_height <= max_height) {
        width = max_width, height = proposed_height;
    } else {
        width = (2 * original->width * max_height) / (original->height);
        height = max_height;
    }

    unsigned char* data = calloc(width * height, sizeof(unsigned char));
    if (data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for resized image\n");
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    if (method == INTERPOLATION_NEAREST) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                size_t x = (i * original->width) / width;
                size_t y = (j * original->height) / height;
                data[i + j * width] = original->data[x + y * original->width];
            }
        }
    } else { // INTERPOLATION_AVERAGE
        for (size_t i = 0; i < width; i++) {
            size_t x1 = (i * original->width) / (width);
            size_t x2 = ((i + 1) * original->width) / (width);
            for (size_t j = 0; j < height; j++) {
                size_t y1 = (j * original->height) / (height);
                size_t y2 = ((j + 1) * original->height) / (height);

                data[i + j * width] = get_average(original, x1, x2, y1, y2);
            }
        }
    }

    return (grayscale_image_t) {
        .width = width,
        .height = height,
        .data = data
    };
}


void print_image(grayscale_image_t* image, bool dark_mode) {
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t level = (image->data[y * image->width + x] * N_LEVELS) / 256;
            if (!dark_mode) level = N_LEVELS - level - 1;
            putchar(LEVEL_CHARS[level]);
        }
        printf("\n");
    }
}

void free_grayscale_image(grayscale_image_t* image) {
    if (image != NULL && image->data != NULL) {
        stbi_image_free(image->data);
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}

bool save_grayscale_image_to_png(const grayscale_image_t* image, const char* filename) {
    if (image == NULL || image->data == NULL || filename == NULL) {
        fprintf(stderr, "Error: Invalid arguments to save_grayscale_image_to_png\n");
        return false;
    }

    int width  = (int)image->width;
    int height = (int)image->height;

    // 1 channel (grayscale): stride = width bytes per row
    int stride_in_bytes = width;

    int ok = stbi_write_png(filename, width, height, 1, image->data, stride_in_bytes);
    if (!ok) {
        fprintf(stderr, "Error: Failed to write PNG file '%s'\n", filename);
        return false;
    }

    return true;
}


rgb_image_t load_image_as_rgb(const char* file_path) {
    int width, height, channels;
    unsigned char* data = stbi_load(file_path, &width, &height, &channels, 3);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to load image '%s': %s\n", file_path, stbi_failure_reason());
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    // Separate RGB channels
    size_t pixel_count = width * height;
    unsigned char* r_data = malloc(pixel_count);
    unsigned char* g_data = malloc(pixel_count);
    unsigned char* b_data = malloc(pixel_count);

    if (r_data == NULL || g_data == NULL || b_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for RGB channels\n");
        stbi_image_free(data);
        free(r_data);
        free(g_data);
        free(b_data);
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    for (size_t i = 0; i < pixel_count; i++) {
        r_data[i] = data[i * 3 + 0];
        g_data[i] = data[i * 3 + 1];
        b_data[i] = data[i * 3 + 2];
    }

    stbi_image_free(data);

    return (rgb_image_t) {
        .width = (size_t) width,
        .height = (size_t) height,
        .r_data = r_data,
        .g_data = g_data,
        .b_data = b_data
    };
}


void free_rgb_image(rgb_image_t* image) {
    if (image != NULL) {
        if (image->r_data != NULL) {
            free(image->r_data);
            image->r_data = NULL;
        }
        if (image->g_data != NULL) {
            free(image->g_data);
            image->g_data = NULL;
        }
        if (image->b_data != NULL) {
            free(image->b_data);
            image->b_data = NULL;
        }
        image->width = 0;
        image->height = 0;
    }
}


typedef struct {
    unsigned char r, g, b;
} rgb_pixel_t;

rgb_pixel_t get_average_rgb(rgb_image_t* image, size_t x1, size_t x2, size_t y1, size_t y2) {
    double r_total = 0, g_total = 0, b_total = 0;

    for (size_t x = x1; x < x2; x++) {
        for (size_t y = y1; y < y2; y++) {
            size_t idx = x + y * image->width;
            r_total += (double) image->r_data[idx];
            g_total += (double) image->g_data[idx];
            b_total += (double) image->b_data[idx];
        }
    }

    double n = (x2 - x1) * (y2 - y1);

    return (rgb_pixel_t) {
        .r = (unsigned char) (r_total / n),
        .g = (unsigned char) (g_total / n),
        .b = (unsigned char) (b_total / n)
    };
}


rgb_image_t make_resized_rgb(rgb_image_t* original, size_t max_width, size_t max_height, interpolation_method_t method) {
    size_t width, height;

    size_t proposed_height = (original->height * max_width) / (2 * original->width);
    if (proposed_height <= max_height) {
        width = max_width, height = proposed_height;
    } else {
        width = (2 * original->width * max_height) / (original->height);
        height = max_height;
    }

    unsigned char* r_data = calloc(width * height, sizeof(unsigned char));
    unsigned char* g_data = calloc(width * height, sizeof(unsigned char));
    unsigned char* b_data = calloc(width * height, sizeof(unsigned char));

    if (r_data == NULL || g_data == NULL || b_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for resized RGB image\n");
        free(r_data);
        free(g_data);
        free(b_data);
        return (rgb_image_t) { .width = 0, .height = 0, .r_data = NULL, .g_data = NULL, .b_data = NULL };
    }

    if (method == INTERPOLATION_NEAREST) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                size_t x = (i * original->width) / width;
                size_t y = (j * original->height) / height;
                size_t original_idx = x + y * original->width;
                size_t new_idx = i + j * width;
                r_data[new_idx] = original->r_data[original_idx];
                g_data[new_idx] = original->g_data[original_idx];
                b_data[new_idx] = original->b_data[original_idx];
            }
        }
    } else { // INTERPOLATION_AVERAGE
        for (size_t j = 0; j < height; j++) {
            size_t y1 = (j * original->height) / height;
            size_t y2 = ((j + 1) * original->height) / height;

            for (size_t i = 0; i < width; i++) {
                size_t x1 = (i * original->width) / width;
                size_t x2 = ((i + 1) * original->width) / width;

                rgb_pixel_t avg = get_average_rgb(original, x1, x2, y1, y2);
                size_t idx = i + j * width;

                r_data[idx] = avg.r;
                g_data[idx] = avg.g;
                b_data[idx] = avg.b;
            }
        }
    }

    return (rgb_image_t) {
        .width = width,
        .height = height,
        .r_data = r_data,
        .g_data = g_data,
        .b_data = b_data
    };
}


grayscale_image_t rgb_to_grayscale(rgb_image_t* rgb) {
    size_t pixel_count = rgb->width * rgb->height;
    unsigned char* data = malloc(pixel_count);

    if (data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for grayscale conversion\n");
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    // Use standard luminance weights: 0.299*R + 0.587*G + 0.114*B
    for (size_t i = 0; i < pixel_count; i++) {
        data[i] = (unsigned char)(
            0.299 * rgb->r_data[i] +
            0.587 * rgb->g_data[i] +
            0.114 * rgb->b_data[i]
        );
    }

    return (grayscale_image_t) {
        .width = rgb->width,
        .height = rgb->height,
        .data = data
    };
}

void calculate_histogram(const grayscale_image_t* image, int* histogram) {
    // Initialize histogram to zeros
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    // Populate histogram
    size_t num_pixels = image->width * image->height;
    for (size_t i = 0; i < num_pixels; i++) {
        histogram[image->data[i]]++;
    }
}

void equalize_histogram(grayscale_image_t* image) {
    int histogram[256];
    calculate_histogram(image, histogram);

    size_t num_pixels = image->width * image->height;
    int cdf[256];
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + histogram[i];
    }

    // Find min non-zero CDF value
    int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    if (cdf_min == 0) { // All pixels are the same or image is empty
        return;
    }

    // Apply equalization formula: h(v) = round(((cdf(v) - cdf_min) / (M*N - cdf_min)) * (L-1))
    // where L-1 = 255
    for (size_t i = 0; i < num_pixels; i++) {
        image->data[i] = (unsigned char)round(((double)cdf[image->data[i]] - cdf_min) / (num_pixels - cdf_min) * 255.0);
    }
}

// Disjoint Set Union (DSU) implementation
typedef struct {
    int* parent;
    int count;
} DSU;

static DSU* dsu_create(int n) {
    DSU* dsu = (DSU*)malloc(sizeof(DSU));
    dsu->parent = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        dsu->parent[i] = i;
    }
    dsu->count = n;
    return dsu;
}

static int dsu_find(DSU* dsu, int i) {
    if (dsu->parent[i] == i) {
        return i;
    }
    return dsu->parent[i] = dsu_find(dsu, dsu->parent[i]);
}

static void dsu_union(DSU* dsu, int i, int j) {
    int root_i = dsu_find(dsu, i);
    int root_j = dsu_find(dsu, j);
    if (root_i != root_j) {
        dsu->parent[root_i] = root_j;
        dsu->count--;
    }
}

static void dsu_free(DSU* dsu) {
    free(dsu->parent);
    free(dsu);
}

grayscale_image_t connected_components(grayscale_image_t* image, int connectivity) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    size_t width = image->width;
    size_t height = image->height;
    size_t num_pixels = width * height;

    int* labels = (int*)calloc(num_pixels, sizeof(int));
    if (labels == NULL) {
        return result;
    }

    DSU* dsu = dsu_create(num_pixels / 2); // Initial guess for number of labels
    int next_label = 1;

    // 1. First pass: label image and record equivalences
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t idx = y * width + x;
            if (image->data[idx] > 128) { // Binarize
                int smallest_neighbor_label = 0;
                
                // Check neighbors
                // Left
                if (x > 0 && labels[idx - 1] > 0) {
                    smallest_neighbor_label = labels[idx - 1];
                }
                // Top
                if (y > 0 && labels[idx - width] > 0) {
                    int top_label = labels[idx - width];
                    if (smallest_neighbor_label == 0) {
                        smallest_neighbor_label = top_label;
                    } else if (smallest_neighbor_label != top_label) {
                        dsu_union(dsu, smallest_neighbor_label, top_label);
                    }
                }

                if (connectivity == 8) {
                    // Top-left
                    if (y > 0 && x > 0 && labels[idx - width - 1] > 0) {
                        int tl_label = labels[idx - width - 1];
                        if (smallest_neighbor_label == 0) {
                            smallest_neighbor_label = tl_label;
                        } else if (smallest_neighbor_label != tl_label) {
                            dsu_union(dsu, smallest_neighbor_label, tl_label);
                        }
                    }
                    // Top-right
                    if (y > 0 && x < width - 1 && labels[idx - width + 1] > 0) {
                        int tr_label = labels[idx - width + 1];
                        if (smallest_neighbor_label == 0) {
                            smallest_neighbor_label = tr_label;
                        } else if (smallest_neighbor_label != tr_label) {
                            dsu_union(dsu, smallest_neighbor_label, tr_label);
                        }
                    }
                }

                if (smallest_neighbor_label == 0) {
                    labels[idx] = next_label++;
                    if (next_label > dsu->count) {
                        // Resize DSU if needed
                        dsu->parent = (int*)realloc(dsu->parent, next_label * 2 * sizeof(int));
                        for (int i = dsu->count; i < next_label * 2; i++) dsu->parent[i] = i;
                        dsu->count = next_label * 2;
                    }
                } else {
                    labels[idx] = smallest_neighbor_label;
                }
            }
        }
    }

    // 2. Second pass: resolve labels
    for (size_t i = 0; i < num_pixels; i++) {
        if (labels[i] > 0) {
            labels[i] = dsu_find(dsu, labels[i]);
        }
    }

    // Create a color map for labels
    int* colors = (int*)calloc(next_label, sizeof(int));
    int color_count = 1;
    for (size_t i = 0; i < num_pixels; i++) {
        if (labels[i] > 0 && colors[labels[i]] == 0) {
            colors[labels[i]] = (color_count++ * 50) % 256;
        }
    }

    // Create output image
    result.width = width;
    result.height = height;
    result.data = (unsigned char*)malloc(num_pixels);
    if (result.data != NULL) {
        for (size_t i = 0; i < num_pixels; i++) {
            result.data[i] = (unsigned char)colors[labels[i]];
        }
    }

    free(labels);
    free(colors);
    dsu_free(dsu);

    return result;
}

grayscale_image_t apply_salt_pepper_noise(const grayscale_image_t* original, float density) {
    grayscale_image_t noisy_image;
    noisy_image.width = original->width;
    noisy_image.height = original->height;
    size_t num_pixels = original->width * original->height;
    noisy_image.data = (unsigned char*)malloc(num_pixels);

    if (noisy_image.data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for noisy image\n");
        return (grayscale_image_t) { .width = 0, .height = 0, .data = NULL };
    }

    // Initialize random seed (should be done once per program execution)
    // srand(time(NULL));

    for (size_t i = 0; i < num_pixels; i++) {
        float r = (float)rand() / (float)RAND_MAX; // Random float between 0.0 and 1.0

        if (r < density / 2.0f) {
            noisy_image.data[i] = 0;   // Salt (black)
        } else if (r > 1.0f - (density / 2.0f)) {
            noisy_image.data[i] = 255; // Pepper (white)
        } else {
            noisy_image.data[i] = original->data[i]; // Keep original
        }
    }

    return noisy_image;
}

void quantize_grayscale(unsigned char* image, int width, int height, int levels) {
    if (levels <= 1) {
        return; // Nothing to do
    }
    if (levels > 256) {
        levels = 256;
    }

    int step = 256 / levels;
    for (int i = 0; i < width * height; i++) {
        int original_value = image[i];
        int quantized_value = (original_value / step) * step + (step / 2);
        if (quantized_value > 255) {
            quantized_value = 255;
        }
        image[i] = (unsigned char)quantized_value;
    }
}

grayscale_image_t apply_otsu_thresholding(const grayscale_image_t* image) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    int histogram[256];
    calculate_histogram(image, histogram);

    size_t num_pixels = image->width * image->height;
    double total_intensity = 0;
    for (int i = 0; i < 256; i++) {
        total_intensity += i * histogram[i];
    }

    double best_variance = 0;
    int best_threshold = 0;

    double w0 = 0; // sum of probabilities for class 0
    double sum0 = 0; // sum of intensities for class 0

    for (int t = 0; t < 256; t++) {
        w0 += histogram[t];
        if (w0 == 0) continue;

        double w1 = num_pixels - w0;
        if (w1 == 0) break;

        sum0 += t * histogram[t];

        double m0 = sum0 / w0;
        double m1 = (total_intensity - sum0) / w1;

        double between_class_variance = w0 * w1 * (m0 - m1) * (m0 - m1);

        if (between_class_variance > best_variance) {
            best_variance = between_class_variance;
            best_threshold = t;
        }
    }

    int threshold = best_threshold; 

    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)malloc(image->width * image->height);
    if (result.data == NULL) {
        result.width = 0;
        result.height = 0;
        return result;
    }

    for (size_t i = 0; i < image->width * image->height; i++) {
        result.data[i] = image->data[i] > threshold ? 255 : 0;
    }

    return result;
}

// Queue structure for region growing
typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point* data;
    int head;
    int tail;
    int size;
    int capacity;
} Queue;

Queue* create_queue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->data = (Point*)malloc(sizeof(Point) * capacity);
    q->head = 0;
    q->tail = -1;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue* q, Point p) {
    if (q->size == q->capacity) {
        q->capacity *= 2;
        q->data = (Point*)realloc(q->data, sizeof(Point) * q->capacity);
    }
    q->tail = (q->tail + 1) % q->capacity;
    q->data[q->tail] = p;
    q->size++;
}

Point dequeue(Queue* q) {
    Point p = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return p;
}

bool is_empty(Queue* q) {
    return q->size == 0;
}

void free_queue(Queue* q) {
    free(q->data);
    free(q);
}


grayscale_image_t apply_region_growing(const grayscale_image_t* image, int seed_x, int seed_y, int threshold) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)calloc(image->width * image->height, sizeof(unsigned char));
    if (result.data == NULL) {
        result.width = 0;
        result.height = 0;
        return result;
    }

    // Check if seed is valid
    if (seed_x < 0 || seed_x >= (int)image->width || seed_y < 0 || seed_y >= (int)image->height) {
        fprintf(stderr, "Error: Invalid seed point for region growing\n");
        return result;
    }

    // Create a visited array
    bool* visited = (bool*)calloc(image->width * image->height, sizeof(bool));
    if (visited == NULL) {
        free(result.data);
        return result;
    }

    Queue* q = create_queue(100); // Initial queue capacity
    if (q == NULL) {
        free(result.data);
        free(visited);
        return result;
    }

    // Add seed to queue and mark as visited
    Point seed = {seed_x, seed_y};
    enqueue(q, seed);
    visited[seed_y * image->width + seed_x] = true;
    result.data[seed_y * image->width + seed_x] = 255; // Mark seed as part of the region

    unsigned char seed_value = image->data[seed_y * image->width + seed_x];

    // Define 8-connectivity neighbors
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    while (!is_empty(q)) {
        Point current = dequeue(q);

        for (int i = 0; i < 8; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (nx >= 0 && nx < (int)image->width && ny >= 0 && ny < (int)image->height &&
                !visited[ny * image->width + nx]) {
                
                unsigned char neighbor_value = image->data[ny * image->width + nx];
                if (abs(neighbor_value - seed_value) <= threshold) {
                    visited[ny * image->width + nx] = true;
                    result.data[ny * image->width + nx] = 255; // Mark as part of the region
                    enqueue(q, (Point){nx, ny});
                }
            }
        }
    }

    free_queue(q);
    free(visited);

    return result;
}

grayscale_image_t apply_adaptive_thresholding(const grayscale_image_t* image, int block_size, double c) {
    grayscale_image_t result = {0};
    if (image == NULL || image->data == NULL) {
        return result;
    }

    result.width = image->width;
    result.height = image->height;
    result.data = (unsigned char*)malloc(image->width * image->height);
    if (result.data == NULL) {
        result.width = 0;
        result.height = 0;
        return result;
    }

    int half_block = block_size / 2;

    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            double sum = 0;
            int count = 0;

            for (int j = -half_block; j <= half_block; j++) {
                for (int i = -half_block; i <= half_block; i++) {
                    int cur_x = (int)x + i;
                    int cur_y = (int)y + j;

                    if (cur_x >= 0 && cur_x < (int)image->width && cur_y >= 0 && cur_y < (int)image->height) {
                        sum += image->data[cur_y * image->width + cur_x];
                        count++;
                    }
                }
            }

            double mean = sum / count;
            double threshold = mean - c;

            result.data[y * image->width + x] = image->data[y * image->width + x] > threshold ? 255 : 0;
        }
    }

    return result;
}