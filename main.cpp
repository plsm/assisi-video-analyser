
#include <qapplication.h>
#include <qwidget.h>
#include <opencv2/opencv.hpp>

#include "animate.hpp"
#include "process-image.hpp"
#include "ui_video-analyser.h"

using namespace std;

void setup (const string &folder, const string &frame_file_type, Ui_MainWindow &mainWindow);

int main( int argc, char **argv )
{
	QApplication a( argc, argv );
	QMainWindow mainWindow;
	Animate::init ();
	Ui_MainWindow ui;

	ui.setupUi (&mainWindow);
	string folder ("/media/Adamastor/ASSISIbf/results/demo/pha-review/TOP-Freq_770-amp_40-pause_420/run-files_frequency=770Hz_amplitude=40_vibration-period=580ms_pause-period=420ms_R#2/");
	string frame_file_type ("jpg");
	Animate animate (folder, "jpg", &ui);
	setup (folder, frame_file_type, ui);
	animate.setup ();
	mainWindow.show ();
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &animate, SLOT (playStop ()));
	QObject::connect (ui.updateRectPushButton, SIGNAL (clicked ()), &animate, SLOT (updateHistogramsRect ()));
	return a.exec();
}

void setup (const string &folder, const string &frame_file_type, Ui_MainWindow &mainWindow)
{
	string filename = folder + "/background." + frame_file_type;
	cv::Mat image = cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
	fprintf (stderr, "The size of the background image (and the video frames) is %d x %d\n", image.size ().width, image.size ().height);
	QSpinBox *horizontal_spinBoxes[] = {mainWindow.x1SpinBox, mainWindow.x2SpinBox};
	QSpinBox *vertical_spinBoxes[] = {mainWindow.y1SpinBox, mainWindow.y2SpinBox};
	for (int i = 0; i < 2; i++) {
		horizontal_spinBoxes [i]->setMinimum (0);
		horizontal_spinBoxes [i]->setMaximum (image.size ().width);
		vertical_spinBoxes [i]->setMinimum (0);
		vertical_spinBoxes [i]->setMaximum (image.size ().height);
	}
	struct {
		QString legend;
		QPen pen;
	} graph_info[] = {
		{.legend = "current frame" , .pen = QPen (Qt::green   , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "rectangle"     , .pen = QPen (Qt::blue    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "background"    , .pen = QPen (Qt::magenta , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
	};
	for (int i = 0; i < 3; i++) {
		QCPGraph *graph = mainWindow.histogramView->addGraph ();
		graph->setName (graph_info [i].legend);
		graph->setPen (graph_info [i].pen);
	}
	mainWindow.histogramView->legend->setVisible (true);
	mainWindow.histogramView->xAxis->setRange (0, NUMBER_COLOUR_LEVELS);
	mainWindow.histogramView->yAxis->setRange (0, 50000);
}
