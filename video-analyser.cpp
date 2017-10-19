#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>

#include "video-analyser.hpp"

#include "image.hpp"
#include "process-image.hpp"
#include "util.hpp"

using namespace std;

static QImage Mat2QImage (const cv::Mat &src);

VideoAnalyser::VideoAnalyser (Experiment &experiment):
	experiment (experiment),
	animate (experiment.parameters, &ui),
	imageItem (NULL),
	scene (new QGraphicsScene ()),
	current_frame_line (3)
{
	ui.setupUi (this);
	this->ui.frameView->setScene (this->scene);
	// initialise the line representing the current frame in the plots
	QCustomPlot *owners[] = {this->ui.plotColourView, this->ui.plotBeeSpeedView, this->ui.plotNumberBeesView};
	for (int i = 0; i < 3; i++) {
		this->current_frame_line [i] = new QCPItemLine (owners [i]);
		this->current_frame_line [i]->start->setTypeY (QCPItemPosition::ptAxisRectRatio);
		this->current_frame_line [i]->start->setCoords (1, 0);
		this->current_frame_line [i]->end->setCoords (1, 1);
	}
	// setup widgets
	QSpinBox *horizontal_spinBoxes[] = {ui.x1SpinBox, ui.x2SpinBox};
	QSpinBox *vertical_spinBoxes[] = {ui.y1SpinBox, ui.y2SpinBox};
	for (int i = 0; i < 2; i++) {
		horizontal_spinBoxes [i]->setMinimum (0);
		horizontal_spinBoxes [i]->setMaximum (experiment.background.size ().width);
		vertical_spinBoxes [i]->setMinimum (0);
		vertical_spinBoxes [i]->setMaximum (experiment.background.size ().height);
	}
	ui.currentFrameSpinBox->setMinimum (1);
	ui.currentFrameSpinBox->setMaximum (experiment.parameters.number_frames);
	struct Graph_Info {
		QString legend;
		QPen pen;
	};
	Graph_Info graph_info[] = {
		{.legend = "current frame all"       , .pen = QPen (Qt::green   , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "background all"          , .pen = QPen (Qt::magenta , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "current frame rectangle" , .pen = QPen (Qt::blue    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "background rectangle"    , .pen = QPen (Qt::red    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	};
	for (int i = 0; i < 4; i++) {
		QCPGraph *graph = ui.histogramView->addGraph ();
		graph->setName (graph_info [i].legend);
		graph->setPen (graph_info [i].pen);
	}
	auto set_xaxis = [&] (auto axis) {
		axis->setLabel ("frame");
		axis->setRange (0, experiment.parameters.number_frames);
	};
	ui.histogramView->legend->setVisible (true);
	ui.histogramView->xAxis->setRange (0, NUMBER_COLOUR_LEVELS);
	ui.histogramView->yAxis->setRange (0, 50000);
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		QCPGraph *graph = this->ui.plotNumberBeesView->addGraph ();
		graph->setName (QString (("number bees in ROI " + to_string (i + 1)).c_str ()));
		graph->setPen (QPen (QColor (
											  255 * i / (experiment.parameters.number_ROIs - 1),
											  255 * (experiment.parameters.number_ROIs - 1 - i) / (experiment.parameters.number_ROIs - 1),
											  128,
											  192)));
		graph = this->ui.plotBeeSpeedView->addGraph ();
		graph->setName (QString (("bee speed in ROI " + to_string (i + 1)).c_str ()));
		graph->setPen (QPen (QColor (
											  255 * i / (experiment.parameters.number_ROIs - 1),
											  255 * (experiment.parameters.number_ROIs - 1 - i) / (experiment.parameters.number_ROIs - 1),
											  128,
											  192)));
	}
	this->ui.plotBeeSpeedView->legend->setVisible (true);
	this->ui.plotBeeSpeedView->xAxis->setRange (0, experiment.parameters.number_frames);
	this->ui.plotBeeSpeedView->yAxis->setRange (0, 2000);
	this->ui.plotNumberBeesView->legend->setVisible (true);
	this->ui.plotNumberBeesView->xAxis->setRange (0, experiment.parameters.number_frames);
	this->ui.plotNumberBeesView->yAxis->setRange (0, 3500);
	Graph_Info plot_graph_info[] = {
		{.legend = "most common colour"   , .pen = QPen (Qt::red  , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
	};
	for (int i = 0; i < 1; i++) {
		QCPGraph *graph = this->ui.plotColourView->addGraph ();
		graph->setName (plot_graph_info [i].legend);
		graph->setPen (plot_graph_info [i].pen);
	}
	this->ui.plotColourView->legend->setVisible (true);
	this->ui.plotColourView->xAxis->setRange (0, experiment.parameters.number_frames);
	this->ui.plotColourView->xAxis->setLabel ("frame");
	set_xaxis (this->ui.plotColourView->xAxis);
	this->ui.plotColourView->yAxis->setRange (0, 256);
	this->ui.plotColourView->yAxis->setLabel ("absolute colour level");
	// show constant data
	ui.histogramView->graph (1)->setData (X_COLOURS, *experiment.histogram_background_raw);
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		this->ui.plotBeeSpeedView->graph (i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_raw) [i * 2 + 1]);
		this->ui.plotNumberBeesView->graph (i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_raw) [i * 2]);
	}
	// setup connection between signals and slots
	QObject::connect (ui.specialOperationRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.showDiffPreviousRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.showDiffBackgroundRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.showVideoFrameRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.cropToRectCheckBox, SIGNAL (clicked ()), this, SLOT (crop_to_rect ()));
	QObject::connect (ui.histogramEqualisationCheckBox, SIGNAL (stateChanged (int)), this, SLOT (histogram_equalisation (int)));
	QObject::connect (ui.currentFrameSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_data (int)));
	QObject::connect (ui.updateRectPushButton, SIGNAL (clicked ()), this, SLOT (update_rect_data ()));
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &this->animate, SLOT (play_stop ()));
	QObject::connect (ui.framesPerSecondSpinBox, SIGNAL (valueChanged (double)), &this->animate, SLOT (update_playback_speed (double)));
	//
	this->user_parameters.equalize_histograms = this->ui.histogramEqualisationCheckBox->isChecked ();
	this->update_data (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_data (int current_frame)
{
	this->update_image (current_frame);
	this->update_histogram (current_frame);
	this->update_plots (current_frame);
	this->update_plot_bee_speed ();
	this->update_plot_number_bees ();
	this->update_plot_colours ();
}

void VideoAnalyser::histogram_equalisation (int new_state)
{
	this->user_parameters.equalize_histograms = this->ui.histogramEqualisationCheckBox->isChecked ();
	if (this->ui.showDiffPreviousRadioButton->isChecked () ||
		 this->ui.showDiffBackgroundRadioButton->isChecked ())
		this->update_image (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_displayed_image ()
{
	bool change = false;
	if (this->ui.showVideoFrameRadioButton->isChecked ()) {
		change = this->user_parameters.image_data != CURRENT_FRAME;
		this->user_parameters.image_data = CURRENT_FRAME;
	}
	else if (this->ui.showDiffPreviousRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		change = this->user_parameters.image_data != DIFF_PREVIOUS_IMAGE;
		this->user_parameters.image_data = DIFF_PREVIOUS_IMAGE;
	}
	else if (this->ui.showDiffBackgroundRadioButton->isChecked ()) {
		change = this->user_parameters.image_data != DIFF_BACKGROUND_IMAGE;
		this->user_parameters.image_data = DIFF_BACKGROUND_IMAGE;
	}
	else if (this->ui.specialOperationRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		change = this->user_parameters.image_data != SPECIAL_DATA;
		this->user_parameters.image_data = SPECIAL_DATA;
	}
	if (change)
		this->update_image (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::crop_to_rect ()
{
	this->update_image (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_rect_data ()
{
	int x1 = this->ui.x1SpinBox->value ();
	int y1 = this->ui.y1SpinBox->value ();
	int x2 = this->ui.x2SpinBox->value ();
	int y2 = this->ui.y2SpinBox->value ();
	int current_frame = this->ui.currentFrameSpinBox->value ();
	delete_histograms (this->experiment.histogram_frames_rect_raw);
	this->experiment.histogram_frames_rect_raw = compute_histogram_frames_rect (this->experiment.parameters, x1, y1, x2, y2);
	Histogram *histogram = (*this->experiment.histogram_frames_rect_raw) [current_frame];
	this->ui.histogramView->graph (2)->setData (X_COLOURS, *histogram);
	delete histogram;
	cv::Mat cropped (this->experiment.background, cv::Range (y1, y2), cv::Range (x1, x2));
	histogram = compute_histogram_image (cropped);
	this->ui.histogramView->graph (3)->setData (X_COLOURS, *histogram);
	delete histogram;
	this->ui.histogramView->replot ();
	delete this->experiment.highest_colour_level_frames_rect;
	this->experiment.highest_colour_level_frames_rect = compute_highest_colour_level_frames_rect (this->experiment.parameters, x1, y1, x2, y2);
	this->ui.plotColourView->graph (0)->setData (this->experiment.X_FRAMES, *this->experiment.highest_colour_level_frames_rect);
	this->ui.plotColourView->replot ();
}

void VideoAnalyser::update_image (int current_frame)
{
	QPixmap image;
	switch (this->user_parameters.image_data) {
	case CURRENT_FRAME:
		image.load (this->experiment.parameters.frame_filename (current_frame).c_str ());
		break;
	case DIFF_BACKGROUND_IMAGE: {
		cv::Mat diff = compute_difference_background_image (this->experiment.parameters, this->user_parameters, current_frame);
		image = QPixmap::fromImage (Mat2QImage (diff));
		break;
	}
	case DIFF_PREVIOUS_IMAGE: {
		cv::Mat diff = compute_difference_previous_image (this->experiment.parameters, this->user_parameters, current_frame);
		image = QPixmap::fromImage (Mat2QImage (diff));
		break;
	}
	case SPECIAL_DATA: {
		cv::Mat _image = compute_threshold_mask_diff_background_diff_previous (this->experiment.parameters, current_frame);
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

void VideoAnalyser::update_histogram (int current_frame)
{
	QVector<double> *histogram = (*this->experiment.histogram_frames_all_raw) [current_frame];
	this->ui.histogramView->graph (0)->setData (X_COLOURS, *histogram);
	if (this->experiment.histogram_frames_rect_raw != NULL) {
		histogram = (*this->experiment.histogram_frames_rect_raw) [current_frame];
		this->ui.histogramView->graph (2)->setData (X_COLOURS, *histogram);
	}
	this->ui.histogramView->replot ();
}

void VideoAnalyser::update_plots (int current_frame)
{
	for (QCPItemLine *cf : this->current_frame_line) {
		cf->start->setCoords (current_frame, 0);
		cf->end->setCoords (current_frame, 1);
	}
	this->ui.plotColourView->replot ();
	this->ui.plotBeeSpeedView->replot ();
	this->ui.plotNumberBeesView->replot ();
}

void VideoAnalyser::update_plot_bee_speed ()
{
}

void VideoAnalyser::update_plot_number_bees ()
{
}

void VideoAnalyser::update_plot_colours ()
{
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
