#ifndef __PROCESS_IMAGE__
#define __PROCESS_IMAGE__

#include <map>
#include <vector>
#include <qvector.h>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"

extern const int NUMBER_COLOUR_LEVELS;

/**
 * Compute the histogram for the backround image located in the given folder.
 *
 * The filename of the background image should be background.EXT, where EXT
 * corresponds to parameter frameFileType.
 */
QVector<double> *compute_histogram_background (const std::string &folder, const std::string &frameFileType);

/**
 * Compute the histogram for all the video frames located in the given folder.
 */
std::map<int, QVector<double> *> *compute_histogram_frames_all (const std::string &folder, const std::string &frameFileType);

std::map<int, QVector<double> *> *compute_histogram_frames_ROI (const std::string &folder, const std::string &frameFileType, int indexROI);

std::map<int, QVector<double> *> *compute_histogram_frames_rect (const std::string &folder, const std::string &frameFileType, int x1, int y1, int x2, int y2);

QVector<double> *compute_histogram_image (const cv::Mat &image);

void delete_histograms (std::map<int, QVector<double> *> *histograms);

cv::Mat compute_difference_background_image (const std::string &folder, const std::string &frame_file_type, int index_frame);

std::vector<QVector<double> > *compute_pixel_count_difference_raw (const Parameters &parameters);

#endif
