#include <QtCore/qglobal.h>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif
#include <opencv2/opencv.hpp>

#include "arena.hpp"
#include "experiment.hpp"
#include "video-analyser.hpp"
#include "util.hpp"

using namespace std;

Parameters get_parameters (int argc, char *argv[])
{
	UserParameters user_parameters = UserParameters::parse (argc, argv);
	if (user_parameters.number_frames == 0) {
		fprintf (stderr, "There are no video frames in folder %s\n", user_parameters.folder.c_str ());
		fprintf (stderr, "Trying debug user parameters...\n");
		UserParameters default_parameters;
		if (default_parameters.number_frames == 0) {
			fprintf (stderr, "There are no video frames in debug parameters!\n");
			exit (EXIT_FAILURE);
		}
		else {
			return default_parameters;
		}
	}
	else {
		return user_parameters;
	}
}

int main (int argc, char **argv)
{
	init ();
	UserParameters parameters = get_parameters (argc, argv);
	QApplication a (argc, argv);
	Experiment experiment (parameters);
	StadiumArena3CASUs arena (parameters);
	VideoAnalyser video_analyser (experiment);
	video_analyser.show ();
	return a.exec ();
}
