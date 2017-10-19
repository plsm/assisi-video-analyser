#ifndef __EXPERIMENT__
#define __EXPERIMENT__

#include <opencv2/core/core.hpp>
#include <vector>

#include "parameters.hpp"

class Experiment {
public:
	const Parameters &parameters;
	cv::Mat background;
	std::vector<cv::Mat> masks;
	Experiment (const Parameters &parameters);
};

#endif
