#ifndef __PARAMETERS__
#define __PARAMETERS__

#include <string>

class Parameters
{
public:
	const std::string folder;
	const std::string frame_file_type;
	const unsigned int number_ROIs;
	const unsigned int delta_frame;
	const unsigned int same_colour_threshold;
	const unsigned int number_frames;
	const unsigned int same_colour_level;
	Parameters (int argc, char *argv[]);
	std::string background_filename () const
	{
		return folder + "/background." + frame_file_type;
	}
	std::string frame_filename (int index_frame) const
	{
		std::string result = this->folder + "/frames-";
		char number[5];
		sprintf (number, "%04d", index_frame);
		result += number;
		result += ".";
		result += this->frame_file_type;
		return result;
	}
	std::string mask_filename (int index_mask) const
	{
		return this->folder + "/Mask-" + std::to_string (index_mask) + ".jpg";
	}
	std::string histogram_background_filename () const
	{
		return this->folder + "/histogram-background.csv";
	}
	std::string pixel_count_difference_raw_filename () const
	{
		return this->folder + "/pixel-count-difference-" + std::to_string (this->same_colour_threshold) + "-raw.csv";
	}
};

#endif
