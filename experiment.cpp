#include "experiment.hpp"

#include "image.hpp"
#include "process-image.hpp"

Experiment::Experiment (UserParameters &parameters):
	parameters (parameters),
	masks (parameters.number_ROIs),
	histogram_background_raw (compute_histogram_background (parameters)),
	histogram_frames_all_raw (compute_histogram_frames_all (parameters)),
	histogram_frames_rect_raw (NULL),
	pixel_count_difference_raw (compute_pixel_count_difference_raw (parameters)),
   pixel_count_difference_light_calibrated_most_common_colour (NULL),
	highest_colour_level_frames_rect (NULL),
   X_FRAMES (parameters.number_frames),
   X_FIRST_LAST_FRAMES (2)
{
	fprintf (stderr, "Initialising experiment data\n");
	fprintf (stderr, "  reading experiment background\n");
	this->background = read_background (parameters);
	fprintf (stderr, "  reading experiment masks\n");
	for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
		masks [index_mask] = read_image (parameters.mask_filename (index_mask));
	}
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
	delete pixel_count_difference_light_calibrated_most_common_colour;
	delete highest_colour_level_frames_rect;
}
