#include "parameters.hpp"

#include "process-image.hpp"

Parameters::Parameters (int argc, char *argv[]):
	folder ("/media/Adamastor/ASSISIbf/results/demo/pha-review/TOP-Freq_770-amp_40-pause_420/run-files_frequency=770Hz_amplitude=40_vibration-period=580ms_pause-period=420ms_R#2/"),
	frame_file_type ("jpg"),
	number_ROIs (3),
	delta_frame (2),
	same_colour_threshold (15),
	number_frames (730),
	same_colour_level ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100)
{
}

