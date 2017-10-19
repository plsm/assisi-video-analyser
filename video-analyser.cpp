#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>

#include "video-analyser.hpp"

#include "process-image.hpp"

using namespace std;

static QImage Mat2QImage (const cv::Mat &src);

VideoAnalyser::VideoAnalyser (Experiment &experiment):
	experiment (experiment),
	imageItem (NULL)
{
	ui.setupUi (this);
	new QRubberBand (QRubberBand::Rectangle, ui.frameView);
	this->scene = new QGraphicsScene ();
	this->ui.frameView->setScene (this->scene);
	QObject::connect (ui.specialOperationRadioButton, SIGNAL (clicked (bool)), this, SLOT (update_displayed_frame (bool)));
	QObject::connect (ui.showDiffPreviousRadioButton, SIGNAL (clicked (bool)), this, SLOT (update_displayed_frame (bool)));
	QObject::connect (ui.showDiffBackgroundRadioButton, SIGNAL (clicked (bool)), this, SLOT (update_displayed_frame (bool)));
	QObject::connect (ui.showVideoFrameRadioButton, SIGNAL (clicked (bool)), this, SLOT (update_displayed_frame (bool)));
	QObject::connect (ui.histogramEqualisationCheckBox, SIGNAL (stateChanged (int)), this, SLOT (histogram_equalisation (int)));
	QObject::connect (ui.currentFrameSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_data (int)));
}

void VideoAnalyser::update_data (int current_frame)
{
	this->update_displayed_frame (true);
}

void VideoAnalyser::histogram_equalisation (int new_state)
{
	this->user_parameters.equalize_histograms = this->ui.histogramEqualisationCheckBox->isChecked ();
	if (this->ui.showDiffPreviousRadioButton->isChecked () ||
		 this->ui.showDiffBackgroundRadioButton->isChecked ())
		this->update_displayed_frame (true);
}


void VideoAnalyser::update_displayed_frame (bool checked)
{
	if (!checked) {cout << "Olha!\n"; return ;}
	QPixmap image;
	// what we are going to display?
	if (this->ui.showVideoFrameRadioButton->isChecked ()) {
		image.load (this->experiment.parameters.frame_filename (this->ui.currentFrameSpinBox->value ()).c_str ());
	}
	else if (this->ui.showDiffPreviousRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		cv::Mat diff = compute_difference_previous_image (this->experiment.parameters, this->user_parameters, this->ui.currentFrameSpinBox->value ());
		image = QPixmap::fromImage (Mat2QImage (diff));
	}
	else if (this->ui.showDiffBackgroundRadioButton->isChecked ()) {
		cv::Mat diff = compute_difference_background_image (this->experiment.parameters, this->user_parameters, this->ui.currentFrameSpinBox->value ());
		QImage _image = Mat2QImage (diff);
		image = QPixmap::fromImage (_image);
	}
	else if (this->ui.specialOperationRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		cv::Mat _image = compute_threshold_mask_diff_background_diff_previous (this->experiment.parameters, this->ui.currentFrameSpinBox->value ());
		image = QPixmap::fromImage (Mat2QImage (_image));
	}
	// update widget
	if (this->imageItem != NULL) {
		this->scene->removeItem (this->imageItem);
	}
	if (this->ui.cropToRectCheckBox->isChecked ())
		image = image.copy (
		 this->ui.x1SpinBox->value (),
		 this->ui.y1SpinBox->value (),
		 this->ui.x2SpinBox->value () - this->ui.x1SpinBox->value (),
		 this->ui.y2SpinBox->value () - this->ui.y1SpinBox->value ());
	this->imageItem = this->scene->addPixmap (image);
	this->ui.frameView->update ();
}

void VideoAnalyser::update_frame ()
{
	QPixmap image;
	switch (this->user_parameters.data_2_view) {
	case CURRENT_FRAME:
		image.load (this->experiment.parameters.frame_filename (this->ui.currentFrameSpinBox->value ()).c_str ());
		break;
	case DIFF_BACKGROUND_IMAGE: {
		cv::Mat diff = compute_difference_background_image (this->experiment.parameters, this->user_parameters, this->ui.currentFrameSpinBox->value ());
		image = QPixmap::fromImage (Mat2QImage (diff));
		break;
	}
	case DIFF_PREVIOUS_IMAGE: {
		cv::Mat diff = compute_difference_previous_image (this->experiment.parameters, this->user_parameters, this->ui.currentFrameSpinBox->value ());
		image = QPixmap::fromImage (Mat2QImage (diff));
		break;
	}
	case SPECIAL_DATA: {
		cv::Mat _image = compute_threshold_mask_diff_background_diff_previous (this->experiment.parameters, this->ui.currentFrameSpinBox->value ());
		image = QPixmap::fromImage (Mat2QImage (_image));
		break;
	}
	}
	if (this->imageItem != NULL) {
		this->scene->removeItem (this->imageItem);
	}
	if (this->ui.cropToRectCheckBox->isChecked ())
		image = image.copy (
		 this->ui.x1SpinBox->value (),
		 this->ui.y1SpinBox->value (),
		 this->ui.x2SpinBox->value () - this->ui.x1SpinBox->value (),
		 this->ui.y2SpinBox->value () - this->ui.y1SpinBox->value ());
	this->imageItem = this->scene->addPixmap (image);
	this->ui.frameView->update ();
}

void VideoAnalyser::update_histogram ()
{
}
	void update_plot_bee_speed ();
	void update_plot_number_bees ();
	void update_plot_colours ();


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
