#ifndef __IMAGE__
#define __IMAGE__

#include <queue>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <qvector.h>

#include "experiment.hpp"
#include "parameters.hpp"
#include "histogram.hpp"

extern const unsigned int NUMBER_COLOUR_LEVELS;

typedef cv::Mat Image;

class Experiment;

inline cv::Mat read_image (const std::string &filename)
{
	return cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
}

/**
 * Read the background image located in the folder parameter.
 */
inline cv::Mat read_background (const Parameters &parameters)
{
	return cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
}

/**
 * Read the nth frame located in the folder parameter.
 */
inline cv::Mat read_frame (const Parameters &parameters, unsigned int index_frame)
{
	return cv::imread (parameters.frame_filename (index_frame), CV_LOAD_IMAGE_GRAYSCALE);
}

/**
 * Compute the histogram of the given image.
 */
void compute_histogram (const cv::Mat &image, Histogram &histogram);

/**
 * Compute the histogram of the given image.
 */
void compute_histogram (const cv::Mat &image, int x1, int y1, int x2, int y2, Histogram &histogram);

/**
 * Compute pixel count difference between the given frame and the background
 * image and between the given frame and a frame x seconds afar.
 *
 * The pixel count difference data is stored in parameter result.
 */
void compute_pixel_count_difference (const Experiment &experiment, const cv::Mat &background, const cv::Mat &frame, FILE *file, std::queue<cv::Mat> *cache, std::vector<QVector<double> > *result);

cv::Mat light_calibrate (const Experiment &experiment, unsigned int frame);

void light_calibrate (cv::Mat &frame, unsigned int pb, unsigned int pf);

#endif
