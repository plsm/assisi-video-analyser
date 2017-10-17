#ifndef __ARENA__
#define __ARENA__

#include <iostream>
#include <opencv2/core/core.hpp>

#include "parameters.hpp"

class ROI
{
public:
	cv::Range span_col;
	cv::Range span_row;
	ROI () {}
	ROI (cv::Range span_col, cv::Range span_row):
		span_col (span_col),
		span_row (span_row)
	{
	}
	friend std::ostream &operator<< (std::ostream &os, const ROI &roi);
};

class Arena
{
public:
	unsigned int number_ROIs;
	std::vector<ROI> rois;

	Arena (unsigned int number_ROIs):
		number_ROIs (number_ROIs),
		rois (number_ROIs)
	{
	}
	friend std::ostream &operator<< (std::ostream &os, const Arena &arena);
};

class StadiumArena3CASUs: public Arena
{
public:
	StadiumArena3CASUs (const Parameters &parameters);
};

#endif

