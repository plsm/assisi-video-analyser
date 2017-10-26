#include <opencv2/imgproc/imgproc.hpp>

#include "image.hpp"
#include "util.hpp"

const unsigned int NUMBER_COLOUR_LEVELS = 256;

void compute_histogram (const cv::Mat &image, Histogram &histogram)
{
	// Quantize the saturation to 32 levels
	int sbins = NUMBER_COLOUR_LEVELS;
	int histSize[] = {sbins};
	// saturation varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, NUMBER_COLOUR_LEVELS };
	const float* ranges[] = { sranges };
	cv::MatND hist;
	// we compute the histogram from the 0-th
	int channels[] = {0};
	cv::calcHist (&image, 1, channels, cv::Mat (), // do not use mask
             hist, 1, histSize, ranges,
             true, // the histogram is uniform
             false);
	for (unsigned int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		histogram [i] = hist.at<float> (i);
	}
}

/**
 * Compute the histogram of the given image.
 */
void compute_histogram (const cv::Mat &image, int x1, int y1, int x2, int y2, Histogram &histogram)
{
	cv::Mat cropped (image, cv::Range (y1, y2), cv::Range (x1, x2));
	compute_histogram (cropped, histogram);
}

void compute_pixel_count_difference (const Experiment &experiment, const cv::Mat &background, const cv::Mat &current_frame, FILE *file, std::queue<cv::Mat> *cache, std::vector<QVector<double> > *result)
{
	static Histogram histogram;
	static cv::Mat number_bees, bee_speed, diff;
	cv::absdiff (background, current_frame, number_bees);
	bool enough_frames = cache->size () == experiment.parameters.delta_frame;
	if (enough_frames) {
		cv::Mat previous_frame = cache->front ();
		cache->pop ();
		cv::absdiff (previous_frame, current_frame, bee_speed);
	}
	int index_col = 0;
	int value;
	for (unsigned int index_mask = 0; index_mask < experiment.parameters.number_ROIs; index_mask++) {
		if (index_mask > 0) fprintf (file, ",");
		diff = number_bees & experiment.masks [index_mask];
		compute_histogram (diff, histogram);
		value = number_different_pixels (experiment.parameters, histogram);
		(*result) [index_col++].append (value);
		fprintf (file, "%d", value);
		if (enough_frames) {
			diff = bee_speed & experiment.masks [index_mask];
			compute_histogram (diff, histogram);
			value = number_different_pixels (experiment.parameters, histogram);
			(*result) [index_col++].append (value);
			fprintf (file, ",%d", value);
		}
		else {
			(*result) [index_col++].append (-1);
			fprintf (file, ",-1");
		}
	}
	cache->push (current_frame);
}

cv::Mat light_calibrate (const Experiment &experiment, unsigned int index_frame)
{
	cv::Mat frame = read_frame (experiment.parameters, index_frame);
	unsigned char pb = experiment.histogram_background_raw->most_common_colour ();
	unsigned char pf = (*experiment.highest_colour_level_frames_rect) [index_frame - 1];
	light_calibrate (frame, pb, pf);
	return frame;
}

void light_calibrate (cv::Mat &frame, unsigned int pb, unsigned int pf)
{
	for(int i = 0; i < frame.rows; i++)
		for(int j = 0; j < frame.cols; j++) {
			unsigned char old_value = frame.at<unsigned char> (i, j);
			if (old_value < pf)
				frame.at<unsigned char> (i, j) = (old_value * pb) / pf;
			else
				frame.at<unsigned char> (i, j) = 255 - ((255 - old_value) * (255 - pb) / (255 - pf));
		}
}
