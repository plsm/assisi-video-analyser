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

void setup (const Parameters &parameters, Ui_MainWindow &mainWindow);

int go (int argc, char **argv)
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
	VideoAnalyser video_analyser (experiment);
	video_analyser.show ();
	return a.exec ();
}

int main( int argc, char **argv )
{
	return go (argc, argv);

	QApplication a (argc, argv);
	QMainWindow mainWindow;
	Animate::init ();
	Ui_MainWindow ui;

	ui.setupUi (&mainWindow);
	Parameters parameters = Parameters::parse (argc, argv);
	if (parameters.number_frames == 0) {
		fprintf (stderr, "There are no video frames in folder %s\n", parameters.folder.c_str ());
		return 1;
	}
	UserParameters user_parameters;
	Experiment experiment (parameters);




	Animate animate (parameters, user_parameters, &ui);
	setup (parameters, ui);
	StadiumArena3CASUs arena (parameters);
	cout << arena << "\n";
	animate.setup ();
	mainWindow.show ();
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &animate, SLOT (playStop ()));
	QObject::connect (ui.updateRectPushButton, SIGNAL (clicked ()), &animate, SLOT (updateRect ()));
	QObject::connect (ui.framesPerSecondSpinBox, SIGNAL (valueChanged (double)), &animate, SLOT (updatePlaybackSpeed (double)));
	QObject::connect (ui.currentFrameSpinBox, SIGNAL (valueChanged (int)), &animate, SLOT (updateCurrentFrame (int)));
	return a.exec();
}



void setup (const Parameters &parameters, Ui_MainWindow &mainWindow)
{
	cv::Mat image = cv::imread (parameters.background_filename (), CV_LOAD_IMAGE_GRAYSCALE);
	QSpinBox *horizontal_spinBoxes[] = {mainWindow.x1SpinBox, mainWindow.x2SpinBox};
	QSpinBox *vertical_spinBoxes[] = {mainWindow.y1SpinBox, mainWindow.y2SpinBox};
	for (int i = 0; i < 2; i++) {
		horizontal_spinBoxes [i]->setMinimum (0);
		horizontal_spinBoxes [i]->setMaximum (image.size ().width);
		vertical_spinBoxes [i]->setMinimum (0);
		vertical_spinBoxes [i]->setMaximum (image.size ().height);
	}
	mainWindow.currentFrameSpinBox->setMaximum (1);
	mainWindow.currentFrameSpinBox->setMaximum (parameters.number_frames);
	struct Graph_Info {
		QString legend;
		QPen pen;
	};
	Graph_Info graph_info[] = {
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
	for (unsigned int i = 0; i < parameters.number_ROIs; i++) {
		QCPGraph *graph = mainWindow.plotNumberBeesView->addGraph ();
		graph->setName (QString (("number bees in ROI " + to_string (i + 1)).c_str ()));
		graph->setPen (QPen (QColor (
											  255 * i / (parameters.number_ROIs - 1),
											  255 * (parameters.number_ROIs - 1 - i) / (parameters.number_ROIs - 1),
											  128,
											  192)));
		graph = mainWindow.plotBeeSpeedView->addGraph ();
		graph->setName (QString (("bee speed in ROI " + to_string (i + 1)).c_str ()));
		graph->setPen (QPen (QColor (
											  255 * i / (parameters.number_ROIs - 1),
											  255 * (parameters.number_ROIs - 1 - i) / (parameters.number_ROIs - 1),
											  128,
											  192)));
	}
	mainWindow.plotBeeSpeedView->legend->setVisible (true);
	mainWindow.plotBeeSpeedView->xAxis->setRange (0, parameters.number_frames);
	mainWindow.plotBeeSpeedView->yAxis->setRange (0, 2000);
	mainWindow.plotNumberBeesView->legend->setVisible (true);
	mainWindow.plotNumberBeesView->xAxis->setRange (0, parameters.number_frames);
	mainWindow.plotNumberBeesView->yAxis->setRange (0, 3500);
	Graph_Info plot_graph_info[] = {
		{.legend = "high colour"   , .pen = QPen (Qt::red  , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
	};
	for (int i = 0; i < 1; i++) {
		QCPGraph *graph = mainWindow.plotColourView->addGraph ();
		graph->setName (plot_graph_info [i].legend);
		graph->setPen (plot_graph_info [i].pen);
	}
	mainWindow.plotColourView->legend->setVisible (true);
	mainWindow.plotColourView->xAxis->setRange (0, parameters.number_frames);
	mainWindow.plotColourView->yAxis->setRange (0, 256);
	mainWindow.plotColourView->yAxis->setLabel ("absolute colour level");
}
