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

QImage Mat2QImage (const cv::Mat &src);

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
		this->updateFrame ();
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
		cv::Mat diff = compute_difference_background_image (this->folder, this->frameFileType, this->indexFrame);
		QImage _image = Mat2QImage (diff);
		image = QPixmap::fromImage (_image);
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

QImage Mat2QImage (const cv::Mat &src)
{
	QImage dest (src.rows, src.cols, QImage::Format_ARGB32);
	for (int y = 0; y < src.rows; ++y) {
		for (int x = 0; x < src.cols; ++x) {
			unsigned int color = src.at<unsigned char> (x, y);
			dest.setPixel (y, x, qRgba (color, color, color, 255));
		}
	}
	return dest;
}
