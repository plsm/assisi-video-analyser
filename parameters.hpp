#ifndef __PARAMETERS__
#define __PARAMETERS__

#include <stdio.h>
#include <string>
#include <opencv2/core/core.hpp>

class Parameters
{
	unsigned int compute_number_frames () const;
	cv::Size compute_frame_size () const;
public:
	const std::string folder;
	const std::string frame_file_type;
	const unsigned int number_ROIs;
	const unsigned int delta_frame;
	const unsigned int same_colour_threshold;
	const unsigned int number_frames;
	const unsigned int same_colour_level;
	const cv::Size frame_size;
	Parameters ();
	Parameters (const std::string &folder, const std::string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int same_colour_threshold);
	static Parameters parse (int argc, char *argv[]);
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
	std::string histogram_frames_all_filename () const
	{
		return this->folder + "/histogram-frames-all.csv";
	}
	std::string histogram_frames_rect (int x1, int y1, int x2, int y2) const
	{
		return this->folder + "/histogram-frames-rect-" +
		std::to_string (x1) + "-" + std::to_string (y1) + "-" +
		std::to_string (x2) + "-" + std::to_string (y2) + ".csv";
	}
	std::string features_pixel_count_difference_raw_filename () const
	{
		return this->folder + "/features_pixel-count-difference-" + std::to_string (this->same_colour_threshold) + "-raw.csv";
	}
	std::string highest_colour_level_frames_rect_filename (int x1, int y1, int x2, int y2) const
	{
		return this->folder + "/highest-colour-level-frames-rect-" +
		std::to_string (x1) + "-" + std::to_string (y1) + "-" +
		std::to_string (x2) + "-" + std::to_string (y2) + ".csv";
	}
	void fold0_frames_IF (void (*func) (unsigned int, const std::string &)) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (index_frame, frame_filename);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A> void fold1_frames_IF (void (*func) (unsigned int, const std::string &, A *), A *acc1) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (index_frame, frame_filename, acc1);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B> void fold2_frames_V (void (*func) (A *, B *), A *acc1, B *acc2) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			func (acc1, acc2);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B> void fold2_frames_IF (void (*func) (unsigned int, const std::string &, A *, B *), A *acc1, B *acc2) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (index_frame, frame_filename, acc1, acc2);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B> void fold2_frames_I (void (*func) (unsigned int, A *, B *), A *acc1, B *acc2) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			func (index_frame, acc1, acc2);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C> void fold3_frames_I (void (*func) (unsigned int, A *, B *, C *), A *acc1, B *acc2, C *acc3) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			func (index_frame, acc1, acc2, acc3);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
};

#endif
