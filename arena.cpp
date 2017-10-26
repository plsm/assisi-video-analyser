#include <yaml-cpp/yaml.h>

#include "arena.hpp"

using namespace std;

StadiumArena3CASUs::StadiumArena3CASUs (const RunParameters &parameters):
	Arena (3)
{
	YAML::Node config = YAML::LoadFile (parameters.folder + "/roi.properties");
	for (int i = 1; i <= 3; i++) {
		int center_x, center_y, width, height;
		center_x = config ["ROI_" + std::to_string (i)  + "_center_x"].as<int> ();
		center_y = config ["ROI_" + std::to_string (i)  + "_center_y"].as<int> ();
		width = config ["ROI_" + std::to_string (i)  + "_width"].as<int> ();
		height = config ["ROI_" + std::to_string (i)  + "_height"].as<int> ();
		this->rois [i - 1] = ROI (
	 		cv::Range (center_x - width / 2, center_x + width / 2),
	 		cv::Range (parameters.frame_size.height - center_y - height / 2, parameters.frame_size.height - center_y + height / 2));
	}
	// cv::FileStorage fs (parameters.folder + "/roi.yaml", cv::FileStorage::READ);
	// for (int i = 1; i <= 3; i++) {
	// 	int center_x, center_y, width, height;
	// 	fs ["ROI_" + std::to_string (i)  + "_center_x"] >> center_x;
	// 	fs ["ROI_" + std::to_string (i)  + "_center_y"] >> center_y;
	// 	fs ["ROI_" + std::to_string (i)  + "_width"] >> width;
	// 	fs ["ROI_" + std::to_string (i)  + "_height"] >> height;
	// 	this->rois [i - 1] = ROI (
	// 		cv::Range (center_x - width / 2, center_x + width / 2),
	// 		cv::Range (parameters.frame_size.height - center_y - height / 2, parameters.frame_size.height - center_y + height / 2));
	// }
}

ostream &operator<< (ostream &os, const ROI &roi)
{
	os << "roi horizontal span " << roi.span_col.start << "-" << roi.span_col.end
		<< " vertical span " << roi.span_row.start << "-" << roi.span_row.end;
	return os;
}

ostream &operator<< (ostream &os, const Arena &arena)
{
	os << "arena ROIs";
	for (const ROI &roi : arena.rois) {
		os << "   ";
		os << roi;
	}
	return os;
}
