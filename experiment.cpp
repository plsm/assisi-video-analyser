#include "experiment.hpp"

#include "image.hpp"
#include "process-image.hpp"

using namespace std;

static vector<cv::Mat> read_masks (const RunParameters &parameters);

Experiment::Experiment (UserParameters &parameters):
	parameters (parameters),
   background (read_background (parameters)),
   masks (read_masks (parameters)),
	histogram_background_raw (compute_histogram_background (parameters)),
	histogram_frames_all_raw (compute_histogram_frames_all (parameters)),
	histogram_frames_rect_raw (NULL),
   pixel_count_difference_raw (compute_pixel_count_difference_raw (*this)),
   pixel_count_difference_light_calibrated_most_common_colour_method_PLSM (NULL),
   pixel_count_difference_light_calibrated_most_common_colour_method_LC (NULL),
   highest_colour_level_frames_rect (NULL),
   X_FRAMES (parameters.number_frames),
   X_FIRST_LAST_FRAMES (2)
{
	for (unsigned int i = 1; i <= parameters.number_frames; i++) {
		X_FRAMES [i - 1] = i;
	}
	X_FIRST_LAST_FRAMES [0] = 1;
	X_FIRST_LAST_FRAMES [1] = parameters.number_frames;
}

Experiment::~Experiment ()
{
	delete histogram_background_raw;
	delete histogram_frames_all_raw;
	delete histogram_frames_rect_raw;
	delete pixel_count_difference_raw;
	delete pixel_count_difference_light_calibrated_most_common_colour_method_PLSM;
	delete pixel_count_difference_light_calibrated_most_common_colour_method_LC;
	delete highest_colour_level_frames_rect;
}

vector<cv::Mat> read_masks (const RunParameters &parameters)
{
	vector<cv::Mat> result (parameters.number_ROIs);
	for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
		result [index_mask] = read_image (parameters.mask_filename (index_mask));
	}
	return result;
}
