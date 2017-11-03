#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "image.hpp"
#include "process-image.hpp"
#include "util.hpp"

using namespace std;

static map<int, Histogram> *read_histograms_frames (const RunParameters &parameters, const string &filename);
static void read_pixel_count_difference (const RunParameters &parameters, const string &filename, vector<QVector<double> > *data);

Histogram *compute_histogram_background (const RunParameters &parameters)
{
	fprintf (stderr, "Computing histogram of background image...\n");
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

map<int, Histogram> *compute_histogram_frames_all (const RunParameters &parameters)
{
	fprintf (stderr, "Computing histogram of entire video frames...\n");
	map<int, Histogram> *result;
	string filename = parameters.histogram_frames_all_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		result = read_histograms_frames (parameters, filename);
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


map<int, Histogram> *compute_histogram_frames_rect (const UserParameters &parameters)
{
	fprintf (stderr, "Computing histogram in rectangle %s of all video frames...\n", parameters.rectangle_user ().c_str ());
	map<int, Histogram> *result;
	string filename = parameters.histogram_frames_rect ();
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
			compute_histogram (frame, parameters.x1, parameters.y1, parameters.x2, parameters.y2, (*result) [index_frame]);
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

map<int, Histogram> *compute_histogram_frames_light_calibrated_most_common_colour_method_PLSM (const Experiment &experiment)
{
	fprintf (stderr,
	         "Computing histogram of frames that were light calibrated using the PLSM method."
	         "  Light calibration uses the most common colour in rectangle %s for each frame.\n",
	         experiment.parameters.rectangle_user ().c_str ());
	map<int, Histogram> *result;
	string filename = experiment.parameters.histogram_frames_light_calibrated_most_common_colour_method_PLSM_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		result = read_histograms_frames (experiment.parameters, filename);
	}
	else {
		Histogram histogram;
		compute_histogram (experiment.background, histogram);
		unsigned int pb = histogram.most_common_colour ();
		result = new map<int, Histogram> ();
		typedef void (*fold3_func) (unsigned int, const string &, unsigned int, FILE *, map<int, Histogram> *);
		fold3_func func = [] (unsigned int index_frame, const string &filename, unsigned int pb, FILE *_file, map<int, Histogram> *_result) {
			cv::Mat frame = read_image (filename);
			compute_histogram (frame, (*_result) [index_frame]);
			unsigned int pf = (*_result) [index_frame].most_common_colour ();
			light_calibrate_method_PLSM (frame, pb, pf);
			compute_histogram (frame, (*_result) [index_frame]);
			(*_result) [index_frame].write (_file);
			fprintf (_file, "\n");
		};
		FILE *file = fopen (filename.c_str (), "w");
		experiment.parameters.fold3_frames_IF (func, pb, file, result);
		fclose (file);
	}
	return result;
}

map<int, Histogram> *compute_histogram_frames_light_calibrated_most_common_colour_method_LC (const Experiment &experiment)
{
	fprintf (stderr,
	         "Computing histogram of frames that were light calibrated using the LC method."
	         "  Light calibration uses the most common colour in rectangle %s for each frame.\n",
	         experiment.parameters.rectangle_user ().c_str ());
	map<int, Histogram> *result;
	string filename = experiment.parameters.histogram_frames_light_calibrated_most_common_colour_method_LC_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		result = read_histograms_frames (experiment.parameters, filename);
	}
	else {
		Histogram histogram;
		compute_histogram (experiment.background, histogram);
		unsigned int pb = histogram.most_common_colour ();
		result = new map<int, Histogram> ();
		typedef void (*fold3_func) (unsigned int, const string &, unsigned int, FILE *, map<int, Histogram> *);
		fold3_func func = [] (unsigned int index_frame, const string &filename, unsigned int pb, FILE *_file, map<int, Histogram> *_result) {
			cv::Mat frame = read_image (filename);
			compute_histogram (frame, (*_result) [index_frame]);
			unsigned int pf = (*_result) [index_frame].most_common_colour ();
			light_calibrate_method_LC (frame, pb, pf);
			compute_histogram (frame, (*_result) [index_frame]);
			(*_result) [index_frame].write (_file);
			fprintf (_file, "\n");
		};
		FILE *file = fopen (filename.c_str (), "w");
		experiment.parameters.fold3_frames_IF (func, pb, file, result);
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

cv::Mat compute_difference_background_image (const UserParameters &parameters, int index_frame)
{
	cv::Mat background = cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat frame = cv::imread (parameters.frame_filename (index_frame), CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat diff;
	cv::absdiff (background, frame, diff);
	return diff;
}

cv::Mat compute_difference_previous_image (const UserParameters &parameters, int index_frame)
{
	cv::Mat current_frame = read_frame (parameters, index_frame);
	cv::Mat previous_frame = read_frame (parameters, index_frame - parameters.delta_frame - 1);
	cv::Mat diff;
	cv::absdiff (previous_frame, current_frame, diff);
	return diff;
}

cv::Mat compute_threshold_mask_diff_background_diff_previous (const UserParameters &parameters, int index_frame)
{
	cv::Mat background = read_background (parameters);
	cv::Mat current_frame = read_frame (parameters, index_frame);
	cv::Mat previous_frame = read_frame (parameters, index_frame - parameters.delta_frame);
	cv::Mat diff1, diff2, mask1, mask2, result;
	cv::absdiff (background, current_frame, diff1);
	cv::threshold (diff1, mask1, parameters.get_same_colour_level (), 255, cv::THRESH_BINARY);
	cv::absdiff (previous_frame, current_frame, diff2);
	cv::threshold (diff2, mask2, parameters.get_same_colour_level (), 255, cv::THRESH_BINARY);
	result = mask1 | mask2;
	return result;
}

cv::Mat light_calibration (const Experiment &experiment, unsigned int index_frame)
{
	cv::Mat background_rect = cv::Mat (
	         experiment.background,
	         cv::Range (experiment.parameters.y1, experiment.parameters.y2),
	         cv::Range (experiment.parameters.x1, experiment.parameters.x2));
	Histogram histogram;
	compute_histogram (background_rect, histogram);
	cv::Mat frame = read_frame (experiment.parameters, index_frame);
	unsigned char pb = histogram.most_common_colour ();
	unsigned char pf = (*experiment.highest_colour_level_frames_rect) [index_frame - 1];
	fprintf (stderr, "Light calibration frame #%d with pb=%d and pf=%d\n", index_frame, pb, pf);
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

vector<QVector<double> > *compute_pixel_count_difference_raw (const Experiment &experiment)
{
	fprintf (stderr, "Computing pixel count difference on raw frames. The difference is between background image and current frame and between %d frames afar.\n", experiment.parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * experiment.parameters.number_ROIs);
	string data_filename = experiment.parameters.features_pixel_count_difference_raw_filename ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		read_pixel_count_difference (experiment.parameters, data_filename, result);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", experiment.parameters.folder.c_str ());
		queue<cv::Mat> cache;
		typedef void (*fold4_func) (const string &, const Experiment *, FILE *, queue<cv::Mat> *, vector<QVector<double> > *);
		fold4_func func = [] (const string &filename, const Experiment *_experiment,
		      FILE *_file, queue<cv::Mat> *_cache, vector<QVector<double> > *_result) {
			cv::Mat frame = read_image (filename);
			compute_pixel_count_difference (*_experiment, _experiment->background, frame, _file, _cache, _result);
			fprintf (_file, "\n");
		};
		FILE *file = fopen (data_filename.c_str (), "w");
		experiment.parameters.fold4_frames_F (func, &experiment, file, &cache, result);
		fclose (file);
	}
	return result;
}

vector<QVector<double> > *compute_pixel_count_difference_histogram_equalization (const Experiment &experiment)
{
	fprintf (stderr, "Computing pixel count difference on frames that have gone through histogram equalization between background images and current frame and between %d frames afar.\n", experiment.parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * experiment.parameters.number_ROIs);
	string data_filename = experiment.parameters.features_pixel_count_difference_histogram_equalization_filename ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		read_pixel_count_difference (experiment.parameters, data_filename, result);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", experiment.parameters.folder.c_str ());
		queue<cv::Mat> cache;
		typedef void (*fold5_func) (const string &, const Experiment *, FILE *, cv::Mat *, queue<cv::Mat> *, vector<QVector<double> > *);
		fold5_func func = [] (
		      const string &filename, const Experiment *_experiment, FILE *_file,
		      cv::Mat *background_HE, queue<cv::Mat> *_cache,
		      vector<QVector<double> > *_result) {
			cv::Mat frame_raw = read_image (filename);
			cv::Mat frame_HE;
			equalizeHist (frame_raw, frame_HE);
			compute_pixel_count_difference (*_experiment, *background_HE, frame_HE, _file, _cache, _result);
			fprintf (_file, "\n");
		};
		cv::Mat background_HE;
		cv::equalizeHist (experiment.background, background_HE);
		FILE *file = fopen (data_filename.c_str (), "w");
		experiment.parameters.fold5_frames_F (func, &experiment, file, &background_HE, &cache, result);
		fclose (file);
	}
	return result;
}

vector<QVector<double> > *compute_pixel_count_difference_light_calibrated_most_common_colour_method_PLSM (const Experiment &experiment)
{
	fprintf (stderr,
	         "Computing pixel count difference on frames that have been light calibrated using the most common colour in rectangle %s."
	         "  The difference is between background image and current frame and between %d frames afar."
	         "  Using PLSM method.\n",
	         experiment.parameters.rectangle_user ().c_str (), experiment.parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * experiment.parameters.number_ROIs);
	string data_filename = experiment.parameters.features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_PLSM ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		read_pixel_count_difference (experiment.parameters, data_filename, result);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", experiment.parameters.folder.c_str ());
		queue<cv::Mat> cache;
		typedef void (*fold4_func) (unsigned int, const string &, const Experiment *, FILE *, queue<cv::Mat> *, vector<QVector<double> > *);
		fold4_func func = [] (unsigned int index_frame, const string &filename, const Experiment *_experiment, FILE *_file, queue<cv::Mat> *_cache, vector<QVector<double> > *_result) {
			cv::Mat frame = read_image (filename);
			unsigned char pb = _experiment->histogram_background_raw->most_common_colour ();
			unsigned char pf = (*_experiment->highest_colour_level_frames_rect) [index_frame - 1];
			light_calibrate_method_PLSM (frame, pb, pf);
			compute_pixel_count_difference (*_experiment, _experiment->background, frame, _file, _cache, _result);
			fprintf (_file, "\n");
		};
		FILE *file = fopen (data_filename.c_str (), "w");
		experiment.parameters.fold4_frames_IF (func, &experiment, file, &cache, result);
		fclose (file);
	}
	return result;
}

vector<QVector<double> > *compute_pixel_count_difference_light_calibrated_most_common_colour_method_LC (const Experiment &experiment)
{
	fprintf (stderr,
	         "Computing pixel count difference on frames that have been light calibrated using the most common colour in rectangle %s."
	         "  The difference is between background image and current frame and between %d frames afar."
	         "  Using LC method.\n",
	         experiment.parameters.rectangle_user ().c_str (), experiment.parameters.delta_frame);
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * experiment.parameters.number_ROIs);
	string data_filename = experiment.parameters.features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_LC ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		read_pixel_count_difference (experiment.parameters, data_filename, result);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", experiment.parameters.folder.c_str ());
		queue<cv::Mat> cache;
		typedef void (*fold4_func) (unsigned int, const string &, const Experiment *, FILE *, queue<cv::Mat> *, vector<QVector<double> > *);
		fold4_func func = [] (unsigned int index_frame, const string &filename, const Experiment *_experiment, FILE *_file, queue<cv::Mat> *_cache, vector<QVector<double> > *_result) {
			cv::Mat frame = read_image (filename);
			unsigned char pb = _experiment->histogram_background_raw->most_common_colour ();
			unsigned char pf = (*_experiment->highest_colour_level_frames_rect) [index_frame - 1];
			light_calibrate_method_LC (frame, pb, pf);
			compute_pixel_count_difference (*_experiment, _experiment->background, frame, _file, _cache, _result);
			fprintf (_file, "\n");
		};
		FILE *file = fopen (data_filename.c_str (), "w");
		experiment.parameters.fold4_frames_IF (func, &experiment, file, &cache, result);
		fclose (file);
	}
	return result;
}

QVector<double> *compute_highest_colour_level_frames_rect (const UserParameters &parameters)
{
	fprintf (stderr, "Computing the most common colour in rectangle %s of raw frames...\n", parameters.rectangle_user ().c_str ());
	QVector<double> *result = new QVector<double> ();
	string filename = parameters.highest_colour_level_frames_rect_filename ();
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
		map<int, Histogram> *map_histograms = compute_histogram_frames_rect (parameters);
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

map<int, Histogram> *read_histograms_frames (const RunParameters &parameters, const string &filename)
{
	map<int, Histogram> *result = new map<int, Histogram> ();
	FILE *file = fopen (filename.c_str (), "r");
	for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
		(*result) [index_frame].read (file);
	};
	fclose (file);
	return result;
}

void read_pixel_count_difference (const RunParameters &parameters, const string &filename, vector<QVector<double> > *data)
{
	fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
	typedef void (*fold_func) (unsigned int, vector<QVector<double> > *, FILE *, unsigned int );
	fold_func func = [] (unsigned int index_frame, vector<QVector<double> > *_result, FILE *_file, unsigned int number_ROIs) {
		for (unsigned int index_mask = 0; index_mask < number_ROIs; index_mask++) {
			int value;
			int ret;
			ret = fscanf (_file, (index_mask > 0 ? ",%d" : "%d"), &value);
			if (ret == 0) {
				fprintf (stderr, "Error reading the %d-th pixel count value of the %d-th frame!\n", 2 * index_mask + 1, index_frame);
				exit (1);
			}
			(*_result) [index_mask * 2].append (value);
			ret = fscanf (_file, ",%d", &value);
			if (ret == 0) {
				fprintf (stderr, "Error reading the %d-th pixel count value of the %d-th frame!\n", 2 * (index_mask + 1), index_frame);
				exit (1);
			}
			(*_result) [index_mask * 2 + 1].append (value);
		}
	};
	FILE *file = fopen (filename.c_str (), "r");
	parameters.fold3_frames_I (func, data, file, parameters.number_ROIs);
	fclose (file);
}
