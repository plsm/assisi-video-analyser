#include <QtCore/qglobal.h>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif
#include <opencv2/opencv.hpp>

#include "animate.hpp"
#include "image.hpp"
#include "process-image.hpp"
#include "ui_video-analyser.h"
#include "arena.hpp"
#include "experiment.hpp"
#include "video-analyser.hpp"
#include "util.hpp"

using namespace std;

int main (int argc, char **argv)
{
	init ();
	QApplication a (argc, argv);
	Parameters parameters = Parameters::parse (argc, argv);
	if (parameters.number_frames == 0) {
		fprintf (stderr, "There are no video frames in folder %s\n", parameters.folder.c_str ());
		return 1;
	}
	UserParameters user_parameters;
	Experiment experiment (parameters);
	StadiumArena3CASUs arena (parameters);
	VideoAnalyser video_analyser (experiment);
	video_analyser.show ();
	return a.exec ();
}
