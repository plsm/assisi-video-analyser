
#include <qapplication.h>
#include <qwidget.h>

#include "animate.hpp"
#include "ui_video-analyser.h"

using namespace std;

int main( int argc, char **argv )
{
	QApplication a( argc, argv );
	QMainWindow mainWindow;

	Ui_MainWindow ui;

	ui.setupUi (&mainWindow);
	Animate animate ("/media/Adamastor/ASSISIbf/results/demo/pha-review/TOP-Freq_770-amp_40-pause_420/run-files_frequency=770Hz_amplitude=40_vibration-period=580ms_pause-period=420ms_R#2/", "jpg", &ui);
	animate.setup ();
	mainWindow.show ();
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &animate, SLOT (playStop ()));
	return a.exec();
}
