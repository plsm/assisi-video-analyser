#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "process-image.hpp"
#include "util.hpp"

using namespace std;

const int NUMBER_COLOUR_LEVELS = 256;

void compute_histogram_image (const cv::Mat &image, QVector<double> &histogram);
QVector<double> *compute_histogram_image (const cv::Mat &image);
QVector<double> *compute_histogram_file (const string &filename);
QVector<double> *compute_histogram_file (const string &filename, int x1, int y1, int x2, int y2);
QVector<double> *read_histogram_file (FILE *file);
map<int, QVector<double> *> *read_histograms_file (const string &filename);
void write_histogram_file (FILE *file, int indexFrame, const QVector<double> *histogram);
void write_histogram_file (FILE *file, const QVector<double> *histogram);
string get_frame_filename (int index_frame, const string &frameFileType);

QVector<double> *compute_histogram_background (const string &folder, const string &frame_file_type)
{
	fprintf (stderr, "Computing histogram of background image\n");
	QVector<double> *result;
	string filename = folder + "/histogram-background.csv";
	if (access (filename.c_str (), R_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		FILE *file = fopen (filename.c_str (), "r");
		result = read_histogram_file (file);
		fclose (file);
	}
	else {
		fprintf (stderr, "  processing background image in folder %s\n", folder.c_str ());
		FILE *file = fopen (filename.c_str (), "w");
		string filename = folder + "/background." + frame_file_type;
		result = compute_histogram_file (filename);
		write_histogram_file (file, result);
		fclose (file);
	}
	return result;
}

map<int, QVector<double> *> *compute_histogram_frames_all (const string &folder, const string &frame_file_type)
{
	fprintf (stderr, "Computing histogram of entire video frames\n");
	map<int, QVector<double> *> *result = new map<int, QVector<double> *> ();
	bool good = true;
	int indexFrame = 1;
	string filename = folder + "/histogram-frames-all.csv";
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		FILE *file = fopen (filename.c_str (), "r");
		do {
			good = fscanf (file, "%d,", &indexFrame) == 1;
			if (good) {
				(*result) [indexFrame] = read_histogram_file (file);
			}
		} while (good);
		fclose (file);
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", folder.c_str ());
		FILE *file = fopen (filename.c_str (), "w");
		do {
			string filename = folder + get_frame_filename (indexFrame, frame_file_type);
			good = access (filename.c_str (), R_OK) == 0;
			if (good) {
				(*result) [indexFrame] = compute_histogram_file (filename);
				write_histogram_file (file, indexFrame, (*result) [indexFrame]);
				fprintf (stderr, "\r    %d", indexFrame);
				fflush (stderr);
				indexFrame++;
			}
		} while (good);
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


map<int, QVector<double> *> *compute_histogram_frames_rect (const std::string &folder, const std::string &frame_file_type, int x1, int y1, int x2, int y2)
{
	fprintf (stderr, "Computing histogram of rectangle (%d,%d)-(%d,%d) in all video frames\n", x1, y1, x2, y2);
	map<int, QVector<double> *> *result = new map<int, QVector<double> *> ();
	string filename = folder + "/histogram-frames-rect-" +
		to_string (x1) + "-" + to_string (y1) + "-" +
		to_string (x2) + "-" + to_string (y2) + ".csv";
	if (access (filename.c_str (), F_OK) == 0) {
		fprintf (stderr, "  reading data from file %s\n", filename.c_str ());
		result = read_histograms_file (filename.c_str ());
	}
	else {
		fprintf (stderr, "  processing video frames in folder %s\n", folder.c_str ());
		FILE *file = fopen (filename.c_str (), "w");
		bool good = true;
		for (int indexFrame = 1; good; indexFrame++) {
			string filename = folder + get_frame_filename (indexFrame, frame_file_type);
			good = access (filename.c_str (), R_OK) == 0;
			if (good) {
				(*result) [indexFrame] = compute_histogram_file (filename, x1, y1, x2, y2);
				write_histogram_file (file, indexFrame, (*result) [indexFrame]);
				fprintf (stderr, "\r    %d", indexFrame);
				fflush (stderr);
			}
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

cv::Mat compute_difference_background_image (const string &folder, const string &frame_file_type, int index_frame)
{
	cv::Mat background = cv::imread (folder + "/background." + frame_file_type, CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat frame = cv::imread (folder + get_frame_filename (index_frame, frame_file_type), CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat result;
	cv::absdiff (background, frame, result);
#if 1
	cv::Mat result2 = background - frame;
	result2 = abs (result);
	cv::Mat result1 = frame - background;
	result1 = abs (result1);
	print_image (result, "difference between background and frame");
	print_image (result2, "difference between background and frame - 2");
	print_image (result1, "difference between background and frame - 1");
	QVector<double> *histogram = compute_histogram_image (result);
	print_histogram (*histogram, "difference between background and frame");
	delete histogram;
	histogram = compute_histogram_image (result1);
	print_histogram (*histogram, "difference between background and frame - 1");
	delete histogram;
	histogram = compute_histogram_image (result2);
	print_histogram (*histogram, "difference between background and frame - 2");
	delete histogram;
	cv::waitKey (0);
#endif
	return result;
}

vector<QVector<double> > *compute_pixel_count_difference_raw (const Parameters &parameters)
{
	vector<QVector<double> > *result = new vector<QVector<double> > (2 * parameters.number_ROIs);
	string data_filename = parameters.pixel_count_difference_raw_filename ();
	if (access (data_filename.c_str (), F_OK) == 0) {
		FILE *file = fopen (data_filename.c_str (), "r");
		fclose (file);
	}
	else {
		FILE *file = fopen (data_filename.c_str (), "w");
		cv::Mat background = cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
		QVector<double> histogram (NUMBER_COLOUR_LEVELS);

		vector<cv::Mat> frames (parameters.delta_frame);
		cv::Mat masks [parameters.number_ROIs];
		for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
			masks [index_mask] = cv::imread (parameters.mask_filename (index_mask), CV_LOAD_IMAGE_GRAYSCALE);
			print_image (masks [index_mask], "mask");
			cv::waitKey (0);
		}
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			string frame_filename = parameters.frame_filename (index_frame);
			cerr << frame_filename << '\n';
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
				compute_histogram_image (diff, histogram);
				(*result) [index_col++].append (value = number_different_pixels (parameters, histogram));
				if (index_mask > 0) fprintf (file, ",");
				fprintf (file, "%d", value);
				if (index_frame > parameters.delta_frame) {
					cv::absdiff (current_frame, frames [(index_frame + 1) % parameters.delta_frame], diff);
					compute_histogram_image (diff, histogram);
					(*result) [index_col++].append (value = number_different_pixels (parameters, histogram));
					fprintf (file, ",%d", value);
				}
				else {
					(*result) [index_col++].append (-1);
					fprintf (file, ",-1");
				}
			}
			fprintf (file, "\n");
		}
		fclose (file);
	}
	return result;
}


// private functions

QVector<double> *read_histogram_file (FILE *file)
{
	QVector<double> *result = new QVector<double> (NUMBER_COLOUR_LEVELS);
	int value;
	fscanf (file, "%d", &value);
	(*result) [0] = value;
	for (int i = 1; i < NUMBER_COLOUR_LEVELS; i++) {
		fscanf (file, ",%d", &value);
		(*result) [i] = value;
	}
	return result;
}

map<int, QVector<double> *> *read_histograms_file (const string &filename)
{
	map<int, QVector<double> *> *result = new map<int, QVector<double> *> ();
	int indexFrame;
	FILE *file = fopen (filename.c_str (), "r");
	bool good;
	do {
		good = fscanf (file, "%d,", &indexFrame) == 1;
		if (good) {
			(*result) [indexFrame] = read_histogram_file (file);
		}
	} while (good);
	fclose (file);
	return result;
}

void write_histogram_file (FILE *file, int indexFrame, const QVector<double> *histogram)
{
	fprintf (file, "%d", indexFrame);
	for (int i = 0; i < NUMBER_COLOUR_LEVELS; i++)
		fprintf (file, ",%d", (int) (*histogram) [i]);
	fprintf (file, "\n");
}

void write_histogram_file (FILE *file, const QVector<double> *histogram)
{
	fprintf (file, "%d", (int) (*histogram) [0]);
	for (int i = 1; i < NUMBER_COLOUR_LEVELS; i++)
		fprintf (file, ",%d", (int) (*histogram) [i]);
	fprintf (file, "\n");
}

void compute_histogram_image (const cv::Mat &image, QVector<double> &histogram)
{
	// Quantize the saturation to 32 levels
	int sbins = NUMBER_COLOUR_LEVELS;
	int histSize[] = {sbins};
	// saturation varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, NUMBER_COLOUR_LEVELS };
	const float* ranges[] = { sranges };
	cv::MatND hist;
	// we compute the histogram from the 0-th
	int channels[] = {0};
	cv::calcHist (&image, 1, channels, cv::Mat (), // do not use mask
             hist, 1, histSize, ranges,
             true, // the histogram is uniform
             false);
	for (int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		histogram [i] = hist.at<float> (i);
	}
}


QVector<double> *compute_histogram_image (const cv::Mat &image)
{
	QVector<double> *result = new QVector<double> (NUMBER_COLOUR_LEVELS);
	// Quantize the saturation to 32 levels
	int sbins = NUMBER_COLOUR_LEVELS;
	int histSize[] = {sbins};
	// saturation varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, NUMBER_COLOUR_LEVELS };
	const float* ranges[] = { sranges };
	cv::MatND hist;
	// we compute the histogram from the 0-th
	int channels[] = {0};
	cv::calcHist (&image, 1, channels, cv::Mat (), // do not use mask
             hist, 1, histSize, ranges,
             true, // the histogram is uniform
             false);
	for (int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		(*result) [i] = hist.at<float> (i);
	}
	return result;
}

QVector<double> *compute_histogram_file (const string &filename)
{
	return compute_histogram_image (cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE));
}

QVector<double> *compute_histogram_file (const string &filename, int x1, int y1, int x2, int y2)
{
	cv::Mat image = cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat cropped (image, cv::Range (y1, y2), cv::Range (x1, x2));
	return compute_histogram_image (cropped);
}



string get_frame_filename (int index_frame, const string &frameFileType)
{
	string result = "frames-";
	char number[5];
	sprintf (number, "%04d", index_frame);
	result += number;
	result += ".";
	result += frameFileType;
	return result;
}
