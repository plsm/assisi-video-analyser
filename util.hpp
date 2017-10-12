#ifndef __UTIL__
#define __UTIL__

#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"

void print_histogram (const QVector<double> &histogram, const char *name);

void print_image (const cv::Mat &image, const char *name);

cv::Mat modulo_difference (const cv::Mat &a, const cv::Mat &b);

int number_different_pixels (const Parameters &parameters, const QVector<double> &histogram);

#endif
