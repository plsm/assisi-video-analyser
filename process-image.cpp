#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "image.hpp"
#include "process-image.hpp"
#include "util.hpp"

using namespace std;

static map<int, Histogram> *read_histograms_frames (const Parameters &parameters, const string &filename);

Histogram *compute_histogram_background (const Parameters &parameters)
{
	fprintf (stderr, "Computing histogram of background image\n");
	Histogram *result = new Histogram ();
	string filename = parameters.histogram_background_filename ();
	if (access (filename.c_str (), R_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		FILE *file = fopen (filename.c_str (), "r");
		result->read (file);
		fclose (file);
	}
	else {
		fprintf (stderr, "  processing background image in folder %s\n", parameters.folder.c_str ());
		compute_histogram (read_background (parameters), *result);
		FILE *file = fopen (filename.c_str (), "w");
		result->write (file);
		fprintf (file, "\n");
		fclose (file);
	}
	return result;
}

map<int, Histogram> *compute_histogram_frames_all (const Parameters &parameters)
{
	fprintf (stderr, "Computing histogram of entire video frames\n");
	map<int, Histogram> *result;
	string filename = parameters.histogram_frames_all_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		result = read_histograms_frames(parameters, filename);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", parameters.folder.c_str ());
		FILE *file = fopen (filename.c_str (), "w");
		result = new map<int, Histogram> ();
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			string filename = parameters.frame_filename (index_frame);
			compute_histogram (read_frame (parameters, index_frame), (*result) [index_frame]);
			(*result) [index_frame].write (file);
			fprintf (file, "\n");
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
		fclose (file);
	}
	return result;
}

// map<int, QVector<double> *> *compute_histogram_frames_ROI (const string &folder, const string &frameFileType, int indexROI)
// {
// 	fprintf (stderr, "Computing histogram of a region of interest in all video frames\n");
// 	map<int, QVector<double> *> *result = new map<int, QVector<double> *> ();
// 	return result;
// }


map<int, Histogram> *compute_histogram_frames_rect (const Parameters &parameters, int x1, int y1, int x2, int y2)
{
	fprintf (stderr, "Computing histogram of rectangle (%d,%d)-(%d,%d) in all video frames\n", x1, y1, x2, y2);
	map<int, Histogram> *result;
	string filename = parameters.histogram_frames_rect (x1, y1, x2, y2);
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		result = read_histograms_frames (parameters, filename.c_str ());
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", parameters.folder.c_str ());
		FILE *file = fopen (filename.c_str (), "w");
		result = new map<int, Histogram> ();
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			Image frame = read_frame (parameters, index_frame);
			compute_histogram (frame, x1, y1, x2, y2, (*result) [index_frame]);
			(*result) [index_frame].write (file);
			fprintf (file, "\n");
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
		}
		fprintf (stderr, "\n");
		fclose (file);
	}
	return result;
}

void delete_histograms (map<int, QVector<double> *> *histograms)
{
	if (histograms == NULL) return;
	for (map<int, QVector<double> *>::iterator it = histograms->begin (); it != histograms->end (); it++) {
		delete it->second;
	}
	delete histograms;
}

// functions that operate on images

cv::Mat compute_difference_background_image (const Parameters &parameters, const UserParameters &user_parameters, int index_frame)
{
	cv::Mat background = cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat frame = cv::imread (parameters.frame_filename (index_frame), CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat diff;
	cv::absdiff (background, frame, diff);
	if (user_parameters.equalize_histograms) {
		cv::Mat result;
		cv::equalizeHist (diff, result);
		return result;
	}
	else
		return diff;
}

cv::Mat compute_difference_previous_image (const Parameters &parameters, const UserParameters &user_parameters, int index_frame)
{
	cv::Mat current_frame = read_frame (parameters, index_frame);
	cv::Mat previous_frame = read_frame (parameters, index_frame - parameters.delta_frame);
	cv::Mat diff;
	cv::absdiff (previous_frame, current_frame, diff);
	if (user_parameters.equalize_histograms) {
		cv::Mat result;
		cv::equalizeHist (diff, result);
		return result;
	}
	else
		return diff;
}

cv::Mat compute_threshold_mask_diff_background_diff_previous (const Parameters &parameters, int index_frame)
{
	cv::Mat background = read_background (parameters);
	cv::Mat current_frame = read_frame (parameters, index_frame);
	cv::Mat previous_frame = read_frame (parameters, index_frame - parameters.delta_frame);
	cv::Mat diff1, diff2, mask1, mask2, result;
	cv::absdiff (background, current_frame, diff1);
	cv::threshold (diff1, mask1, parameters.same_colour_level, 255, cv::THRESH_BINARY);
	cv::absdiff (previous_frame, current_frame, diff2);
	cv::threshold (diff2, mask2, parameters.same_colour_level, 255, cv::THRESH_BINARY);
	result = mask1 | mask2;
	return result;
}

cv::Mat light_calibration (const Experiment &experiment, unsigned int index_frame)
{
	cv::Mat background = read_background (experiment.parameters);
	cv::Mat frame = read_frame (experiment.parameters, index_frame);
	unsigned char pb = experiment.histogram_background_raw->most_common_colour ();
	unsigned char pf = (*experiment.highest_colour_level_frames_rect) [index_frame - 1];
	for(int i = 0; i < frame.rows; i++)
		for(int j = 0; j < frame.cols; j++) {
			unsigned char old_value = frame.at<unsigned char> (i, j);
			if (old_value < pf)
				frame.at<unsigned char> (i, j) = (old_value * pb) / pf;
			else
				frame.at<unsigned char> (i, j) = 255 - ((255 - old_value) * (255 - pb) / (255 - pf));
		}
	return frame;
}

vector<QVector<double> > *compute_pixel_count_difference_raw (const Parameters &parameters)
{
	fprintf (stderr, "Computing pixel count difference raw between background images and current frame and between current frame and %d frame afar\n", parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * parameters.number_ROIs);
	string data_filename = parameters.features_pixel_count_difference_raw_filename ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", data_filename.c_str ());
		FILE *file = fopen (data_filename.c_str (), "r");
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
				int value;
				if (index_mask > 0)
					fscanf (file, ",%d", &value);
				else
					fscanf (file, "%d", &value);
				(*result) [index_mask * 2].append (value);
				fscanf (file, ",%d", &value);
				(*result) [index_mask * 2 + 1].append (value);
			}
		}
		fclose (file);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", parameters.folder.c_str ());
		FILE *file = fopen (data_filename.c_str (), "w");
		cv::Mat background = cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
		Histogram histogram;

		vector<cv::Mat> frames (parameters.delta_frame);
		cv::Mat masks [parameters.number_ROIs];
		for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
			masks [index_mask] = cv::imread (parameters.mask_filename (index_mask), CV_LOAD_IMAGE_GRAYSCALE);
			print_image (masks [index_mask], "mask");
			cv::waitKey (0);
		}
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			string frame_filename = parameters.frame_filename (index_frame);
			frames [index_frame % parameters.delta_frame] = cv::imread (frame_filename, CV_LOAD_IMAGE_GRAYSCALE);
			cv::Mat &current_frame = frames [index_frame % parameters.delta_frame];
			// cerr << "aqui\n";
			// print_image (current_frame, "current frame");
			// cv::waitKey (0);
			cv::Mat number_bees;
			cv::absdiff (background, current_frame, number_bees);
			// print_image (number_bees, "number of bees");
			// cv::waitKey (0);
			int index_col = 0;
			int value;
			for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
				cv::Mat diff = number_bees & masks [index_mask];
				// if (first) {
				// 	print_image (diff, "number of bees in first mask");
				// 	first = false;
				// }
				compute_histogram (diff, histogram);
				(*result) [index_col++].append (value = number_different_pixels (parameters, histogram));
				if (index_mask > 0) fprintf (file, ",");
				fprintf (file, "%d", value);
				if (index_frame > parameters.delta_frame) {
					cv::absdiff (current_frame, frames [(index_frame + 1) % parameters.delta_frame], diff);
					diff = diff & masks [index_mask];
					compute_histogram (diff, histogram);
					(*result) [index_col++].append (value = number_different_pixels (parameters, histogram));
					fprintf (file, ",%d", value);
				}
				else {
					(*result) [index_col++].append (-1);
					fprintf (file, ",-1");
				}
			}
			fprintf (stderr, "\r    %d", index_frame);
			fflush (stderr);
			fprintf (file, "\n");
		}
		fprintf (stderr, "\n");
		fclose (file);
	}
	return result;
}

vector<QVector<double> > *compute_pixel_count_difference_histogram_equalization (const Experiment &experiment)
{
	fprintf (stderr, "Computing pixel count difference on images that have gone through histogram equalization between background images and current frame and between current frame and %d frame afar\n", experiment.parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * experiment.parameters.number_ROIs);
	string data_filename = experiment.parameters.features_pixel_count_difference_histogram_equalization_filename ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", data_filename.c_str ());
		typedef void (*fold_func) (vector<QVector<double> > *, FILE *, unsigned int );
		fold_func func = [] (vector<QVector<double> > *_result, FILE *_file, unsigned int number_ROIs) {
			for (unsigned int index_mask = 0; index_mask < number_ROIs; index_mask++) {
				int value;
				if (index_mask > 0)
					fscanf (_file, ",%d", &value);
				else
					fscanf (_file, "%d", &value);
				(*_result) [index_mask * 2].append (value);
				fscanf (_file, ",%d", &value);
				(*_result) [index_mask * 2 + 1].append (value);
			}
		};
		FILE *file = fopen (data_filename.c_str (), "r");
		experiment.parameters.fold3_frames_V (func, result, file, experiment.parameters.number_ROIs);
		fclose (file);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", experiment.parameters.folder.c_str ());
		struct State {
			const Experiment &_experiment;
			cv::Mat background_HE;
			std::vector<cv::Mat> frames;
			State (const Experiment &e):
				_experiment (e),
				frames (e.parameters.delta_frame + 1)
			{}
		} state (experiment);
		equalizeHist (experiment.background, state.background_HE);
		typedef void (*fold_func) (unsigned int, const std::string &, vector<QVector<double> > *, FILE *, State *);
		fold_func func = [] (unsigned int index_frame, const std::string &frame_filename, vector<QVector<double> > *_result, FILE *_file, State *_state) {
			cv::Mat current_frame_raw = read_image (frame_filename);
			equalizeHist (current_frame_raw, _state->frames [index_frame % (_state->_experiment.parameters.delta_frame + 1)]);
			cv::Mat &current_frame_HE = _state->frames [index_frame % (_state->_experiment.parameters.delta_frame + 1)];
			cv::Mat number_bees;
			Histogram histogram;
			cv::absdiff (_state->background_HE, current_frame_HE, number_bees);
			int index_col = 0;
			int value;
			for (unsigned int index_mask = 0; index_mask < _state->_experiment.parameters.number_ROIs; index_mask++) {
				cv::Mat diff = number_bees & _state->_experiment.masks [index_mask];
				compute_histogram (diff, histogram);
				(*_result) [index_col++].append (value = number_different_pixels (_state->_experiment.parameters, histogram));
				if (index_mask > 0) fprintf (_file, ",");
				fprintf (_file, "%d", value);
				if (index_frame > _state->_experiment.parameters.delta_frame) {
					cv::absdiff (current_frame_HE, _state->frames [(index_frame + 1) % _state->_experiment.parameters.delta_frame], diff);
					diff = diff & _state->_experiment.masks [index_mask];
					compute_histogram (diff, histogram);
					(*_result) [index_col++].append (value = number_different_pixels (_state->_experiment.parameters, histogram));
					fprintf (_file, ",%d", value);
				}
				else {
					(*_result) [index_col++].append (-1);
					fprintf (_file, ",-1");
				}
			}
			fprintf (_file, "\n");
		};
		FILE *file = fopen (data_filename.c_str (), "w");
		experiment.parameters.fold3_frames_IF (func, result, file, &state);
		fclose (file);
	}
	return result;
}

QVector<double> *compute_highest_colour_level_frames_rect (const Parameters &parameters, int x1, int y1, int x2, int y2)
{
	fprintf (stderr, "Computing the colour level with highest count in the histogram of a rectangular are in frames\n");
	QVector<double> *result = new QVector<double> ();
	string filename = parameters.highest_colour_level_frames_rect_filename (x1, y1, x2, y2);
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		FILE *file = fopen (filename.c_str (), "r");
		typedef void (*fold2_func) (QVector<double> *, FILE *);
		fold2_func func = [] (QVector<double> *_result, FILE *_file) {
			int value;
			fscanf (_file, "%d", &value);
			_result->append (value);
		};
		parameters.fold2_frames_V (func, result, file);
		fclose (file);
	}
	else {
		fprintf (stderr, "  computing from frames histograms\n");
		FILE *file = fopen (filename.c_str (), "w");
		map<int, Histogram> *map_histograms = compute_histogram_frames_rect (parameters, x1, y1, x2, y2);
		typedef void (*fold3_func) (unsigned int, map<int, Histogram> *, QVector<double> *, FILE *);
		fold3_func func3 = [] (unsigned int index_frame, map<int, Histogram> *_map_histograms, QVector<double> *_result, FILE *_file) {
			const Histogram &an_histogram = _map_histograms->at (index_frame);
			int value = 0;
			double best = an_histogram [0];
			for (unsigned int colour = 1; colour < NUMBER_COLOUR_LEVELS; colour++)
				if (an_histogram [colour] > best) {
					best = an_histogram [colour];
					value = colour;
				}
			_result->append (value);
			fprintf (_file, "%d\n", value);
		};
		parameters.fold3_frames_I (func3, map_histograms, result, file);
		delete map_histograms;
		fclose (file);
	}
	return result;
}

// private functions

map<int, Histogram> *read_histograms_frames (const Parameters &parameters, const string &filename)
{
	map<int, Histogram> *result = new map<int, Histogram> ();
	FILE *file = fopen (filename.c_str (), "r");
	for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
		(*result) [index_frame].read (file);
	};
	fclose (file);
	return result;
}
