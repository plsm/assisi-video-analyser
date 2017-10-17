#ifndef __PROCESS_IMAGE__
#define __PROCESS_IMAGE__

#include <map>
#include <vector>
#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"

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

cv::Mat compute_difference_background_image (const Parameters &parameters, int index_frame);

std::vector<QVector<double> > *compute_pixel_count_difference_raw (const Parameters &parameters);

/**
 * For each frame compute the colour level with the highest count in the
 * histogram of a rectangular area (in the video frame).
 */
QVector<double> *compute_highest_colour_level_frames_rect (const Parameters &parameters, int x1, int y1, int x2, int y2);

#endif
