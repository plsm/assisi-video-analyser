#ifndef __PROCESS_IMAGE__
#define __PROCESS_IMAGE__

#include <map>
#include <vector>
#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"
#include "experiment.hpp"

/**
 * Compute the histogram for the backround image located in the given folder.
 *
 * The filename of the background image should be background.EXT, where EXT
 * corresponds to parameter frameFileType.
 */
QVector<double> *compute_histogram_background (const Parameters &parameters);

/**
 * Compute the histogram for all the video frames located in the given folder.
 */
std::map<int, QVector<double> *> *compute_histogram_frames_all (const Parameters &parameters);

std::map<int, QVector<double> *> *compute_histogram_frames_ROI (const Parameters &parameters, int indexROI);

std::map<int, QVector<double> *> *compute_histogram_frames_rect (const Parameters &parameters, int x1, int y1, int x2, int y2);

QVector<double> *compute_histogram_image (const cv::Mat &image);

void delete_histograms (std::map<int, QVector<double> *> *histograms);

/**
 * Compute an image that corresponds to the absolute difference between the
 * background image and a video frame.  This image can be used to get a
 * proxy for where are the bees.
 */
cv::Mat compute_difference_background_image (const Parameters &parameters, const UserParameters &user_parameters, int index_frame);

cv::Mat compute_difference_previous_image (const Parameters &parameters, const UserParameters &user_parameters, int index_frame);

/**
 * Compute an image that is the result of bit wise and operation between
 * the difference the background image and the given video frame, and the
 * difference between the given video frame and a frame afar.
 */
cv::Mat compute_threshold_mask_diff_background_diff_previous (const Parameters &parameters, int index_frame);

std::vector<QVector<double> > *compute_pixel_count_difference_raw (const Parameters &parameters);

std::vector<QVector<double> > *compute_pixel_count_difference_histogram_equalization (const Experiment &experiment);

/**
 * For each frame compute the colour level with the highest count in the
 * histogram of a rectangular area (in the video frame).
 */
QVector<double> *compute_highest_colour_level_frames_rect (const Parameters &parameters, int x1, int y1, int x2, int y2);

#endif
