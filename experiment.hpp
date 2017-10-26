#ifndef __EXPERIMENT__
#define __EXPERIMENT__

#include <opencv2/core/core.hpp>
#include <QtCore/QVector>
#include <vector>
#include <map>

#include "parameters.hpp"
#include "process-image.hpp"
#include "image.hpp"

class Experiment {
public:
	const Parameters &parameters;
	cv::Mat background;
	std::vector<cv::Mat> masks;
	/**
	 * Cache with the histogram of the background image.
	 */
	Histogram *histogram_background_raw;
	/**
	 * Cache with the histogram of all frames.
	 */
	std::map<int, Histogram> *histogram_frames_all_raw;
	/**
	 * Caches the histogram of a rectangular area for all frames.
	 */
	std::map<int, Histogram> *histogram_frames_rect_raw;
	/**
	 * Cache with the pixel count difference raw between background image and
	 * raw frame and between raw frames x seconds apart, for all regions of interest.
	 */
	std::vector<QVector<double> > *pixel_count_difference_raw;
	/**
	 * @brief Cache with the pixel count difference between background image and
	 * light calibrated frame and between light calibrated frames x seconds
	 * apart, for all regions of interest, using the most common colour to calibrate.
	 *
	 * A rectangular area in the background image is used to compute the most
	 * common colour that is used as reference.  The same rectangular area is
	 * used for all frames to compute the most common colour that is used as the
	 * value to calibrate.  It is assumed that attribute
	 * highest_colour_level_frames_rect contains the most common colour for each
	 * frame.
	 *
	 * @see #highest_colour_level_frames_rect
	 */
	std::vector<QVector<double> > *pixel_count_difference_light_calibrated_most_common_colour;
	/**
	 * Cached highest colour level in frame histogram.
	 */
	QVector<double> *highest_colour_level_frames_rect;

	QVector<double> X_FRAMES;

	Experiment (const Parameters &parameters);
	virtual ~Experiment ();
};

#endif
