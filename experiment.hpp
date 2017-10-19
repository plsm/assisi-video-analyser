#ifndef __EXPERIMENT__
#define __EXPERIMENT__

#include <opencv2/core/core.hpp>
#include <QtCore/QVector>
#include <vector>
#include <map>

#include "parameters.hpp"

class Experiment {
public:
	const Parameters &parameters;
	cv::Mat background;
	std::vector<cv::Mat> masks;
	/**
	 * Cache with the histogram of the background image.
	 */
	QVector<double> *histogram_background_raw;
	/**
	 * Cache with the histogram of all frames.
	 */
	std::map<int, QVector<double> *> *histogram_frames_all_raw;
	/**
	 * Caches the histogram of a rectangular area for all frames.
	 */
	std::map<int, QVector<double> *> *histogram_frames_rect_raw;
	/**
	 * Cache with the pixel count difference raw between background image and
	 * current frame and between frames x s apart, for all regions of interest.
	 */
	std::vector<QVector<double> > *pixel_count_difference_raw;
	/**
	 * Cached highest colour level in frame histogram.
	 */
	QVector<double> *highest_colour_level_frames_rect;

	QVector<double> X_FRAMES;

	Experiment (const Parameters &parameters);
	virtual ~Experiment ();
};

#endif
