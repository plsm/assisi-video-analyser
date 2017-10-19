#include "experiment.hpp"

#include "image.hpp"

Experiment::Experiment (const Parameters &parameters):
	parameters (parameters),
	masks (parameters.number_ROIs)
{
	fprintf (stderr, "Initialising experiment data\n");
	fprintf (stderr, "  reading experiment background\n");
	this->background = read_background (parameters);
	fprintf (stderr, "  reading experiment masks\n");
	for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
		masks [index_mask] = read_image (parameters.mask_filename (index_mask));
	}
}
