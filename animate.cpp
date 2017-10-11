#include <iostream>
#include <unistd.h>
#include <qpixmap.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

#include <vector>

#include "animate.hpp"
#include "process-image.hpp"

using namespace std;

QVector<double> Animate::X_COLOURS (NUMBER_COLOUR_LEVELS);

void Animate::init ()
{
	for (int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		X_COLOURS [i] = i;
	}
}

Animate::Animate (string folder, string frameFileType, Ui_MainWindow *mainWindow):
	QObject (),
	folder (folder),
	frameFileType (frameFileType),
	indexFrame (1),
	isPlaying (false),
	mainWindow (mainWindow),
	imageItem (NULL),
	histogramBackground (compute_histogram_background (folder, frameFileType)),
	histogramFramesAll (compute_histogram_frames_all (folder, frameFileType)),
	histogramFramesRect (NULL),
	backgroundImage (cv::imread (folder + "/background." + frameFileType, CV_LOAD_IMAGE_GRAYSCALE))
{
	this->scene = new QGraphicsScene ();
	this->mainWindow->frameView->setScene (this->scene);
	// this->mainWindow->histogramView->addGraph ();
	// this->mainWindow->histogramView->addGraph ();
	// QVector<double> y (256);
	// for (int i = 0; i < 256; i++) {
	// 	y [i] = 0;
	// }
	// this->mainWindow->histogramView->graph (0)->setData (X_COLOURS, y);
}

void Animate::setup ()
{
	this->timer = new QTimer (this);
	QObject::connect (timer, SIGNAL (timeout ()), this, SLOT (update ()));
	this->mainWindow->histogramView->graph (2)->setData (X_COLOURS, *(this->histogramBackground));
}

void Animate::update ()
{
	cout << "void Animate::update ()\n";
	string frame_path = this->getFramePath ();
	if (access (frame_path.c_str (), R_OK) == 0) {
		QPixmap image;
		image.load (frame_path.c_str ());
		if (this->imageItem != NULL) {
			this->scene->removeItem (this->imageItem);
		}
		this->imageItem = this->scene->addPixmap (image);
		this->mainWindow->frameView->update ();
		this->updateHistogram ();
		this->mainWindow->currentFrameSpinBox->setValue (this->indexFrame);
		this->indexFrame += 1;
	}
	else {
		this->indexFrame = 1;
		this->isPlaying = false;
		this->timer->stop ();
		this->mainWindow->playStopButton->setText ("Play");
	}
}

void Animate::updateFrame ()
{
	QPixmap image;
	if (this->mainWindow->showVideoFrameRadioButton->isChecked ()) {
		string frame_path = this->getFramePath ();
		image.load (frame_path.c_str ());
	}
	else if (this->mainWindow->showDiffPreviousRadioButton->isChecked ()) {
	}
	else if (this->mainWindow->showDiffBackgroundRadioButton->isChecked ()) {
		string frame_path = this->getFramePath ();
		image.load (frame_path.c_str ());
		image = image - this->backgroundImage;
		image = cv::abs (image);
	}
	if (this->imageItem != NULL) {
		this->scene->removeItem (this->imageItem);
	}
	this->imageItem = this->scene->addPixmap (image);
	this->mainWindow->frameView->update ();
}

void Animate::updateHistogram ()
{
	QVector<double> *histogram;
	histogram = (*this->histogramFramesAll) [this->indexFrame];
	this->mainWindow->histogramView->graph (0)->setData (X_COLOURS, *histogram);
	if (this->histogramFramesRect != NULL) {
		histogram = (*this->histogramFramesRect) [this->indexFrame];
		this->mainWindow->histogramView->graph (1)->setData (X_COLOURS, *histogram);
	}
	this->mainWindow->histogramView->replot ();
}

void Animate::updateHistogram (const string &framePath)
{
	cv::Mat image = cv::imread (framePath, CV_LOAD_IMAGE_GRAYSCALE);
	// Quantize the saturation to 32 levels
	int sbins = 256;
	int histSize[] = {sbins};
	// saturation varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, 256 };
	const float* ranges[] = { sranges };
	cv::MatND histogram;
	// we compute the histogram from the 0-th
	int channels[] = {0};
	cv::calcHist (&image, 1, channels, cv::Mat (), // do not use mask
             histogram, 1, histSize, ranges,
             true, // the histogram is uniform
             false);
	QVector<double> x (256), y (256);
	double maxCount = 0;
	for (int i = 0; i < 256; i++) {
		x [i] = i;
		y [i] = histogram.at<float> (i);
		maxCount = std::max (maxCount, y [i]);
	}
	this->mainWindow->histogramView->graph (0)->setData (x, y);
	this->mainWindow->histogramView->yAxis->setRange (0, maxCount);
	this->mainWindow->histogramView->replot ();


	// cout << "Histogram of " << framePath << " contains " << histogram.total () << " elements\n"
	// 	<< "The number of rows is " << histogram.rows << " and the number of columns is " << histogram.cols << "\n";
	cout << "Histogram is:\n";
	for (unsigned int i = 0; i < 256; i++)
		cout << " " << histogram.at<float> (i);
	cout << "\n";

	// this->mainWindow->histogramView
}

void Animate::playStop ()
{
	cout << "void Animate::playStop ()\n";
	this->isPlaying = !this->isPlaying;
	if (this->isPlaying) {
		this->timer->start (1000);
		this->mainWindow->playStopButton->setText ("Stop");
	}
	else {
		this->timer->stop ();
		this->mainWindow->playStopButton->setText ("Play");
	}
}

void Animate::updateHistogramsRect ()
{
	delete_histograms (this->histogramFramesRect);
	this->histogramFramesRect = compute_histogram_frames_rect
		(
		 this->folder, this->frameFileType,
		 this->mainWindow->x1SpinBox->value (),
		 this->mainWindow->y1SpinBox->value (),
		 this->mainWindow->x2SpinBox->value (),
		 this->mainWindow->y2SpinBox->value ());
}

string Animate::getFramePath () const
{
	string result = this->folder;
	result += "/frames-";
	char number[5];
	sprintf (number, "%04d", this->indexFrame);
	result += number;
	result += ".";
	result += this->frameFileType;
	return result;
}
