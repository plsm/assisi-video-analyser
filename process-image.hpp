#ifndef __PROCESS_IMAGE__
#define __PROCESS_IMAGE__

#include <map>
#include <vector>
#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"
#include "experiment.hpp"
#include "image.hpp"
#include "histogram.hpp"

class Experiment;

/**
 * Compute the histogram for the backround image located in the given folder.
 *
 * The filename of the background image should be background.EXT, where EXT
 * corresponds to parameter frameFileType.
 */
Histogram *compute_histogram_background (const RunParameters &parameters);

/**
 * Compute the histogram for all the video frames located in the given folder.
 */
std::map<int, Histogram> *compute_histogram_frames_all (const RunParameters &parameters);

std::map<int, Histogram> *compute_histogram_frames_ROI (const RunParameters &parameters, int indexROI);

std::map<int, Histogram> *compute_histogram_frames_rect (const UserParameters &parameters);

/**
 * Compute an image that corresponds to the absolute difference between the
 * background image and a video frame.  This image can be used to get a
 * proxy for where are the bees.
 */
cv::Mat compute_difference_background_image (const UserParameters &parameters, int index_frame);

cv::Mat compute_difference_previous_image (const UserParameters &parameters, int index_frame);

/**
 * Compute an image that is the result of bit wise and operation between
 * the difference the background image and the given video frame, and the
 * difference between the given video frame and a frame afar.
 */
cv::Mat compute_threshold_mask_diff_background_diff_previous (const RunParameters &parameters, int index_frame);

/**
 * Perform light calibration on the given frame using the most common colour
 * intensity in the background image and in the given frame.  The background
 * image is assumed as a reference point.  The current frame is adjusted so that
 * its most common intensity matches the most common colour intensity of the
 * background frame.
 *
 * Let cb be the most common intensity in the background image.
 * Let cf be the most common intensity in the given frame.
 *
 * The pixels in the resulting image are computed as folows:
 *
 * dst(x,y) = src(x,y) * cb / cf                          if src(x,y) <= cf
 * dst(x,y) = M - (M - src(x,y)) * (M - cb) / (M - cf)    if src(x,y) > cf
 */
cv::Mat light_calibration (const Experiment &experiment, unsigned int index_frame);

std::vector<QVector<double> > *compute_pixel_count_difference_raw (const RunParameters &parameters);

std::vector<QVector<double> > *compute_pixel_count_difference_histogram_equalization (const Experiment &experiment);

/**
 * Compute the pixel count difference between background image and frame, and
 * between frames x seconds apart using light calibrated frames using the most
 * common colour intensity in a rectangular area of each frame.
 *
 * This function assumes that the most common intensity in a rectangular area
 * has already been calculated.
 */
std::vector<QVector<double> > *compute_pixel_count_difference_light_calibrated_most_common_colour (const Experiment &experiment);

/**
 * For each frame compute the colour level with the highest count in the
 * histogram of a rectangular area (in the video frame).
 */
QVector<double> *compute_highest_colour_level_frames_rect (const UserParameters &parameters);

#endif
