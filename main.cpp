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
#include "dialog-run-parameters.hpp"

using namespace std;

Parameters get_parameters (int argc, char *argv[])
{
	UserParameters user_parameters = UserParameters::parse (argc, argv);
	if (user_parameters.number_frames == 0) {
		DialogRunParameters dialog (NULL);
		dialog.exec ();
		UserParameters gui_parameters (dialog.get_folder (), dialog.get_frame_file_type (), dialog.get_number_ROIs ());
		if (gui_parameters.number_frames == 0) {
			fprintf (stderr, "There are no video frames to analyse!\n");
			exit (EXIT_FAILURE);
		}
		else {
			return gui_parameters;
		}
	}
	else {
		return user_parameters;
	}
}

int main (int argc, char **argv)
{
	init ();
	QApplication a (argc, argv);
	UserParameters parameters = get_parameters (argc, argv);
	Experiment experiment (parameters);
	VideoAnalyser video_analyser (experiment);
	video_analyser.show ();
	return a.exec ();
}
