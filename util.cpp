#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

#include "util.hpp"
#include "image.hpp"
#include "process-image.hpp"

QVector<double> X_COLOURS (NUMBER_COLOUR_LEVELS);

void init ()
{
	for (unsigned int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		X_COLOURS [i] = i;
	}
}

void print_image (const cv::Mat &image, const char *name)
{
	fprintf (stderr, "Information about %s:\n", name);
	fprintf (stderr, "   size is %d x %d\n", image.size ().width, image.size ().height);
	fprintf (stderr, "   number of channels is %d\n", image.channels ());
	fprintf (stderr, "   size in bytes of the frame elements is %ld\n", image.elemSize ());
	cv::Point points[] = {
		cv::Point (100, 100), cv::Point (110, 100), cv::Point (200, 200)
	};
	for (int i = 0; i < 3; i++)
		if (points [i].x < image.size ().width && points [i].y < image.size ().height)
			fprintf (stderr, "   pixel value at %d %d is %d\n", points [i].x, points [i].y, (int) image.at<unsigned char> (points [i]));
	if (image.size ().width > 0 && image.size ().height > 0)
		cv::imshow (name, image);
}

void print_histogram (const QVector<double> &histogram, const char *name)
{
	fprintf (stderr, "Histogram of %s\n  ", name);
	for (unsigned int i = 0; i < NUMBER_COLOUR_LEVELS; i++)
		fprintf (stderr, " %f", histogram [i]);
	fprintf (stderr, "\n");
}

cv::Mat modulo_difference (const cv::Mat &a, const cv::Mat &b)
{
	cv::Mat result (a);
	for (int x = 0; x < a.cols; ++x) {
		for (int y = 0; y < a.rows; ++y) {
			if (a.at<unsigned char> (x, y) > b.at<unsigned char> (x, y))
				result.at<unsigned char> (x, y) -= b.at<unsigned char> (x, y);
			else if (a.at<unsigned char> (x, y) < b.at<unsigned char> (x, y))
				result.at<unsigned char> (x, y) = b.at<unsigned char> (x, y) - result.at<unsigned char> (x, y);
			else
				result.at<unsigned char> (x, y) = 0;
		}
	}
	return result;
}

int number_different_pixels (const RunParameters &parameters, const QVector<double> &histogram)
{
	int result = 0;
	for (unsigned int i = parameters.same_colour_level; i < NUMBER_COLOUR_LEVELS; i++)
		result += histogram [i];
	return result;
}
