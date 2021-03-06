#ifndef __PARAMETERS__
#define __PARAMETERS__

#include <stdio.h>
#include <string>
#include <opencv2/core/core.hpp>

/**
 * @brief The RunParameters class represents parameters used to perform an experimental run.
 */
class RunParameters
{
	unsigned int compute_number_frames () const;
	cv::Size compute_frame_size () const;
public:
	const std::string folder;
	const std::string frame_file_type;
	const unsigned int number_ROIs;
	const unsigned int delta_frame;
	const unsigned int number_frames;
	const cv::Size frame_size;
	RunParameters (const std::string &folder, const std::string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame);
	std::string background_filename () const
	{
		return folder + "background." + frame_file_type;
	}
	std::string frame_filename (int index_frame) const
	{
		std::string result = this->folder + "frames-";
		char number[5];
		sprintf (number, "%04d", index_frame);
		result += number;
		result += ".";
		result += this->frame_file_type;
		return result;
	}
	std::string mask_filename (int index_mask) const
	{
		return this->folder + "Mask-" + std::to_string (index_mask + 1) + ".png";
	}
	std::string histogram_background_filename () const
	{
		return this->folder + "histogram-background.csv";
	}
	std::string histogram_frames_all_filename () const
	{
		return this->folder + "histogram-frames-all.csv";
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
	template<typename A, typename B, typename C> void fold3_frames_I (void (*func) (unsigned int, A, B, C), A acc1, B acc2, C acc3) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			func (index_frame, acc1, acc2, acc3);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C> void fold3_frames_V (void (*func) (A *, B *, C), A *acc1, B *acc2, C acc3) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			func (acc1, acc2, acc3);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C> void fold3_frames_IF (void (*func) (unsigned int, const std::string &, A, B, C), A acc1, B acc2, C acc3) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (index_frame, frame_filename, acc1, acc2, acc3);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C, typename D> void fold4_frames_IF (void (*func) (unsigned int, const std::string &, A, B, C, D), A acc1, B acc2, C acc3, D acc4) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (index_frame, frame_filename, acc1, acc2, acc3, acc4);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C, typename D> void fold4_frames_F (void (*func) (const std::string &, A, B, C, D), A acc1, B acc2, C acc3, D acc4) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (frame_filename, acc1, acc2, acc3, acc4);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
	template<typename A, typename B, typename C, typename D, typename E> void fold5_frames_F (void (*func) (const std::string &, A, B, C, D, E), A acc1, B acc2, C acc3, D acc4, E acc5) const
	{
		for (unsigned int index_frame = 1; index_frame <= this->number_frames; index_frame++) {
			std::string frame_filename = this->frame_filename (index_frame);
			func (frame_filename, acc1, acc2, acc3, acc4, acc5);
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
	}
};

/**
 * Parameters that the user can set and that affect image processing functions.
 */
class UserParameters: public RunParameters
{
	std::string rectangle () const
	{
		return
		      "_RECT=" +
		      std::to_string (this->x1) + "x" + std::to_string (this->y1) + "-" +
		      std::to_string (this->x2) + "x" + std::to_string (this->y2);
	}
	UserParameters (const std::string &folder, const std::string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int same_colour_threshold);
	unsigned int same_colour_threshold;
	unsigned int same_colour_level;
public:
	/**
	 * @brief x1 lowest horizontal coordinate of the rectangular area used in light calibration.
	 */
	int x1;
	int y1;
	int x2;
	int y2;
	UserParameters ();
	UserParameters (const std::string &folder, const std::string &frame_file_type, unsigned int number_ROIs);
	static UserParameters parse (int argc, char *argv[]);
	std::string features_pixel_count_difference_raw_filename () const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (this->same_colour_threshold) +
		      "_DF=" + std::to_string (this->delta_frame) +
		      "_raw.csv";
	}
	std::string features_pixel_count_difference_histogram_equalization_filename () const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (this->same_colour_threshold) +
		      "_DF=" + std::to_string (this->delta_frame) +
		      "_histogram-equalization.csv";
	}
	std::string histogram_frames_rect () const
	{
		return this->folder +
		       "histogram-frames-rect-" +
		       this->rectangle () +
		       ".csv";
	}
	std::string histogram_frames_light_calibrated_most_common_colour_method_PLSM_filename () const
	{
		return
		      this->folder +
		      "histogram-frames"
		      "_light-calibrated-most-common-colour" +
		      this->rectangle () +
		      "_PLSM"
		      ".csv";
	}
	std::string histogram_frames_light_calibrated_most_common_colour_method_LC_filename () const
	{
		return
		      this->folder +
		      "histogram-frames"
		      "_light-calibrated-most-common-colour" +
		      this->rectangle () +
		      "_LC"
		      ".csv";
	}
	std::string features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_PLSM () const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (this->same_colour_threshold) +
		      "_DF=" + std::to_string (this->delta_frame) +
		      "_light-calibration-most-common-colour" +
		      rectangle () +
		      "_PLSM" +
		      ".csv";
	}
	std::string features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_LC () const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (this->same_colour_threshold) +
		      "_DF=" + std::to_string (this->delta_frame) +
		      "_light-calibration-most-common-colour" +
		      rectangle () +
		      "_LC" +
		      ".csv";
	}
	std::string highest_colour_level_frames_rect_filename () const
	{
		return
		      this->folder +
		      "most-common-colour" +
		      rectangle () +
		      ".csv";
	}
	/**
	 * @brief rectangle_user return a string representing the rectangle to be analysed in a human readable way.
	 * @return a string representing the rectangle to be analysed in a human readable way.
	 */
	std::string rectangle_user () const
	{
		return
		    "(" + std::to_string (this->x1) + "," + std::to_string (this->y1) + ")-(" +
		    std::to_string (this->x2) + "," + std::to_string (this->y2) + ")";
	}
	unsigned int get_same_colour_threshold () const;
	void set_same_colour_threshold (unsigned int value);
	unsigned int get_same_colour_level() const;
};

typedef UserParameters Parameters;

#endif
