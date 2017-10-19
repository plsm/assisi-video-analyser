#ifndef __UTIL__
#define __UTIL__

#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"

extern QVector<double> X_COLOURS;

void init ();

void print_histogram (const QVector<double> &histogram, const char *name);

void print_image (const cv::Mat &image, const char *name);

cv::Mat modulo_difference (const cv::Mat &a, const cv::Mat &b);

/**
 * Computes the number of pixels that are different according to the same colour
 * threshold parameter.  The histogram is assumed to be the result of the
 * absolute difference between two images. This means that black colour
 * represents pixels with the same colour, while white represents pixels with
 * the maximum colour difference.
 */
int number_different_pixels (const Parameters &parameters, const QVector<double> &histogram);

#endif
