#include <getopt.h>
#include <unistd.h>

#include "parameters.hpp"
#include "process-image.hpp"
#include "image.hpp"

using namespace std;


Parameters::Parameters ():
	// folder ("/media/Adamastor/ASSISIbf/results/demo/pha-review/TOP-Freq_770-amp_40-pause_420/run-files_frequency=770Hz_amplitude=40_vibration-period=580ms_pause-period=420ms_R#2/"),
	folder ("/media/Adamastor/ASSISIbf/results/demo/pha-review/TOP-Freq_770-amp_40-pause_420/run-files_frequency=770Hz_amplitude=40_vibration-period=580ms_pause-period=420ms_R#12/"),
	frame_file_type ("jpg"),
	number_ROIs (3),
	delta_frame (2),
	same_colour_threshold (15),
	number_frames (compute_number_frames ()),
	same_colour_level ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100)
{
}

Parameters::Parameters (const string &folder, const string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int same_colour_threshold):
	folder (folder),
	frame_file_type (frame_file_type),
	number_ROIs (number_ROIs),
	delta_frame (delta_frame),
	same_colour_threshold (same_colour_threshold),
	number_frames (compute_number_frames ()),
	same_colour_level ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100),
	frame_size (compute_frame_size ())
{
}

Parameters Parameters::parse (int argc, char *argv[])
{
	bool ok = true;
	const char *folder = "./";
	const char *frame_file_type = "jpg";
	unsigned int same_colour_threshold = 15;
	unsigned int delta_frame = 2;
	do {
		static struct option long_options[] = {
			{"folder"                , required_argument, 0, 'p' },
			{"frame-file-type"       , required_argument, 0, 'f' },
			{"same-colour-threshold" , required_argument, 0, 'c' },
			{"delta-frame"           , required_argument, 0, 'd'},
			{0,         0,                 0,  0 }
		};
		int c = getopt_long (argc, argv, "p:f:c:r:d:", long_options, 0);
		switch (c) {
		case '?':
			break;
		case -1:
			ok = false;
			break;
		case ':':
			fprintf (stderr, "Missing argument\n");
			exit (EXIT_FAILURE);
			break;
		case 'p':
			folder = optarg;
			break;
		case 'f':
			frame_file_type = optarg;
			break;
		case 'c':
			same_colour_threshold = (unsigned int) atoi (optarg);
			break;
		case 'd':
			delta_frame = (unsigned int) atoi (optarg);
			break;
		}
	} while (ok);
	return Parameters (folder, frame_file_type, 3, delta_frame, same_colour_threshold);
}

unsigned int Parameters::compute_number_frames () const
{
	unsigned int low, high;
	high = 1;
	while (access (this->frame_filename (high).c_str (), F_OK) == 0)
		high = 2 * high;
	low = high / 2;
	if (low > 0)
		while (low < high) {
			unsigned int middle = (low + high) / 2;
			if (access (this->frame_filename (middle).c_str (), F_OK) == 0)
				low = middle + 1;
			else
				high = middle - 1;
		}
	return low;
}

cv::Size Parameters::compute_frame_size () const
{
	return read_background (*this).size ();
}
