#include <iostream>
#include <unistd.h>
#include <qpixmap.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

#include <vector>

#include "animate.hpp"
#include "process-image.hpp"
#include "image.hpp"

using namespace std;

QVector<double> Animate::X_COLOURS (NUMBER_COLOUR_LEVELS);

QImage Mat2QImage (const cv::Mat &src);

void Animate::init ()
{
	for (unsigned int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		X_COLOURS [i] = i;
	}
}

Animate::Animate (const Parameters &parameters, Ui_MainWindow *mainWindow):
	QObject (),
	parameters (parameters),
	indexFrame (1),
	isPlaying (false),
	mainWindow (mainWindow),
	imageItem (NULL),
	histogramBackground (compute_histogram_background (parameters)),
	histogramFramesAll (compute_histogram_frames_all (parameters)),
	histogramFramesRect (NULL),
	backgroundImage (read_background (parameters)),
	highest_colour_level_frames_rect (NULL),
	X_FRAMES (parameters.number_frames),
	currentFrameLine (3)
{
	for (unsigned int i = 1; i <= parameters.number_frames; i++) {
		X_FRAMES [i] = i;
	}
	this->scene = new QGraphicsScene ();
	this->mainWindow->frameView->setScene (this->scene);
	// initialise the line representing the current frame in the plots
	QCustomPlot *owners[] = {this->mainWindow->plotColourView, this->mainWindow->plotBeeSpeedView, this->mainWindow->plotNumberBeesView};
	for (int i = 0; i < 3; i++) {
		this->currentFrameLine [i] = new QCPItemLine (owners [i]);
		this->currentFrameLine [i]->start->setTypeY (QCPItemPosition::ptAxisRectRatio);
		this->currentFrameLine [i]->start->setCoords (1, 0);
		this->currentFrameLine [i]->end->setCoords (1, 1);
	}
}

void Animate::setup ()
{
	this->timer = new QTimer (this);
	QObject::connect (timer, SIGNAL (timeout ()), this, SLOT (update ()));
	this->mainWindow->histogramView->graph (2)->setData (X_COLOURS, *(this->histogramBackground));
	vector<QVector<double> >*pixel_count_difference_raw = compute_pixel_count_difference_raw (parameters);
	for (unsigned int i = 0; i < parameters.number_ROIs; i++) {
		this->mainWindow->plotBeeSpeedView->graph (i)->setData (X_FRAMES, (*pixel_count_difference_raw)[i * 2 + 1]);
		this->mainWindow->plotNumberBeesView->graph (i)->setData (X_FRAMES, (*pixel_count_difference_raw)[i * 2]);
	}
}

void Animate::update ()
{
	cout << "void Animate::update ()\n";
	if (this->indexFrame < this->parameters.number_frames) {
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
	cout << "void Animate::updateFrame ()\n";
	QPixmap image;
	if (this->mainWindow->showVideoFrameRadioButton->isChecked ()) {
		string frame_path = this->getFramePath ();
		image.load (frame_path.c_str ());
	}
	else if (this->mainWindow->showDiffPreviousRadioButton->isChecked ()) {
	}
	else if (this->mainWindow->showDiffBackgroundRadioButton->isChecked ()) {
		cv::Mat diff = compute_difference_background_image (this->parameters, this->indexFrame);
		QImage _image = Mat2QImage (diff);
		image = QPixmap::fromImage (_image);
	}
	if (this->imageItem != NULL) {
		this->scene->removeItem (this->imageItem);
	}
	if (this->mainWindow->cropToRectCheckBox->isChecked ())
		image = image.copy (
		 this->mainWindow->x1SpinBox->value (),
		 this->mainWindow->y1SpinBox->value (),
		 this->mainWindow->x2SpinBox->value () - this->mainWindow->x1SpinBox->value (),
		 this->mainWindow->y2SpinBox->value () - this->mainWindow->y1SpinBox->value ());
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

void Animate::updatePlots ()
{
	cout << "void Animate::updatePlots ()\n";
	for (QCPItemLine *cf : this->currentFrameLine) {
		cf->start->setCoords (this->indexFrame, 0);
		cf->end->setCoords (this->indexFrame, 1);
	}
	this->mainWindow->plotColourView->replot ();
	this->mainWindow->plotBeeSpeedView->replot ();
	this->mainWindow->plotNumberBeesView->replot ();
}

void Animate::playStop ()
{
	cout << "void Animate::playStop ()\n";
	this->isPlaying = !this->isPlaying;
	if (this->isPlaying) {
		this->timer->start (std::max (1, (int) (1000 / this->mainWindow->framesPerSecondSpinBox->value ())));
		this->mainWindow->playStopButton->setText ("Stop");
	}
	else {
		this->timer->stop ();
		this->mainWindow->playStopButton->setText ("Play");
	}
}

void Animate::updateCurrentFrame (int value)
{
	cout << "void Animate::updateCurrentFrame (int value)\n";
	this->indexFrame = value;
	this->updateFrame ();
	this->updateHistogram ();
	this->updatePlots ();
}

void Animate::updateRect ()
{
	delete_histograms (this->histogramFramesRect);
	this->histogramFramesRect = compute_histogram_frames_rect (
		 this->parameters,
		 this->mainWindow->x1SpinBox->value (),
		 this->mainWindow->y1SpinBox->value (),
		 this->mainWindow->x2SpinBox->value (),
		 this->mainWindow->y2SpinBox->value ());
	delete this->highest_colour_level_frames_rect;
	this->highest_colour_level_frames_rect = compute_highest_colour_level_frames_rect (
		 this->parameters,
		 this->mainWindow->x1SpinBox->value (),
		 this->mainWindow->y1SpinBox->value (),
		 this->mainWindow->x2SpinBox->value (),
		 this->mainWindow->y2SpinBox->value ());
	this->mainWindow->plotColourView->graph (0)->setData (X_FRAMES, *this->highest_colour_level_frames_rect);
	this->mainWindow->plotColourView->replot ();
}

void Animate::updatePlaybackSpeed (double framesPerSecond)
{
	if (this->isPlaying) {
		this->timer->start (std::max (1, (int) (1000 / framesPerSecond)));
	}
}

string Animate::getFramePath () const
{
	return this->parameters.frame_filename (this->indexFrame);
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
