#ifndef FREQUENCY_H
#define FREQUENCY_H

#include "image_processing.h"
#include "filters.h"


/**
 * Compute the 2D Discrete Fourier Transform of a grayscale image
 * and return the magnitude spectrum as a new grayscale image.
 */
grayscale_image_t dft_grayscale(grayscale_image_t* image);

grayscale_image_t dct_grayscale(grayscale_image_t* image);

grayscale_image_t dwt_grayscale(grayscale_image_t* image);

grayscale_image_t apply_frequency_filter(const grayscale_image_t* image, filter_type_t filter_type, double cutoff);

#endif // FREQUENCY_H
