#include <map>
#include <qvector.h>

extern const int NUMBER_COLOUR_LEVELS;

/**
 * Compute the histogram for the backround image located in the given folder.
 */
QVector<double> *compute_histogram_background (const std::string &folder, const std::string &frameFileType);

/**
 * Compute the histogram for all the video frames located in the given folder.
 */
std::map<int, QVector<double> *> *compute_histogram_frames_all (const std::string &folder, const std::string &frameFileType);

std::map<int, QVector<double> *> *compute_histogram_frames_ROI (const std::string &folder, const std::string &frameFileType, int indexROI);

std::map<int, QVector<double> *> *compute_histogram_frames_rect (const std::string &folder, const std::string &frameFileType, int x1, int y1, int x2, int y2);

void delete_histograms (std::map<int, QVector<double> *> *histograms);
