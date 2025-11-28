#ifndef FREQUENCY_H
#define FREQUENCY_H

#include "image_processing.h"

/**
 * Compute the 2D Discrete Fourier Transform of a grayscale image
 * and return the magnitude spectrum as a new grayscale image.
 */
grayscale_image_t dft_grayscale(grayscale_image_t* image);

#endif // FREQUENCY_H
