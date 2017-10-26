#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "video-analyser.hpp"

#include "image.hpp"
#include "process-image.hpp"
#include "util.hpp"

using namespace std;

static QImage Mat2QImage (const cv::Mat &image, const cv::Mat &mask);

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
		this->current_frame_line [i]->end->setTypeY (QCPItemPosition::ptAxisRectRatio);
		this->current_frame_line [i]->end->setCoords (1, 1);
		this->current_frame_line [i]->setPen (QPen (QColor (0, 0, 0, 127), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}
	// initialise the line representing the intensity to analyse
	this->intensity_analyse_line = new QCPItemLine (this->ui.histogramView);
	this->intensity_analyse_line->start->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_analyse_line->start->setCoords (1, 0);
	this->intensity_analyse_line->end->setCoords (1, 1);
	this->intensity_analyse_line->setPen (QPen (QColor (0, 0, 0, 127), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	this->intensity_span_rect = new QCPItemRect (this->ui.histogramView);
	this->intensity_span_rect->topLeft->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_span_rect->topLeft->setCoords (0, 1);
	this->intensity_span_rect->bottomRight->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_span_rect->bottomRight->setCoords (0, 0);
	this->intensity_span_rect->setPen (QPen (QColor (127, 127, 127, 63), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	this->intensity_span_rect->setBrush (QBrush (QColor (127, 127, 127, 63)));
	// setup widgets
	QSpinBox *horizontal_spinBoxes[] = {ui.x1SpinBox, ui.x2SpinBox};
	QSpinBox *vertical_spinBoxes[] = {ui.y1SpinBox, ui.y2SpinBox};
	for (int i = 0; i < 2; i++) {
		horizontal_spinBoxes [i]->setMinimum (0);
		horizontal_spinBoxes [i]->setMaximum (experiment.background.size ().width);
		vertical_spinBoxes [i]->setMinimum (0);
		vertical_spinBoxes [i]->setMaximum (experiment.background.size ().height);
	}
	ui.x2SpinBox->setValue (experiment.background.size ().width);
	ui.y2SpinBox->setValue (experiment.background.size ().height);
	ui.currentFrameSpinBox->setMinimum (1);
	ui.currentFrameSpinBox->setMaximum (experiment.parameters.number_frames);
	QFont title_font ("sans", 10, QFont::Bold);
	struct Graph_Info {
		QString legend;
		QPen pen;
	};
	Graph_Info graph_info[] = {
	   {.legend = "current frame - no cropping"       , .pen = QPen (Qt::green   , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "background - no cropping"          , .pen = QPen (Qt::magenta , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "current frame - cropped rectangle" , .pen = QPen (Qt::blue    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "background - cropped rectangle"    , .pen = QPen (Qt::red     , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
		{.legend = "current frame light calibrated"    , .pen = QPen (QColor (127, 127, 0)  , 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
	};
	for (int i = 0; i < 5; i++) {
		QCPGraph *graph = ui.histogramView->addGraph ();
		graph->setName (graph_info [i].legend);
		graph->setPen (graph_info [i].pen);
	}
	auto set_xaxis = [&] (auto axis) {
		axis->setLabel ("frame");
		axis->setRange (0, experiment.parameters.number_frames);
	};
	auto set_colour_axis = [&] (auto axis) {
		QSharedPointer<QCPAxisTickerText> textTicker (new QCPAxisTickerText);
		axis->setRange (0, NUMBER_COLOUR_LEVELS);
		textTicker->addTick (0, "0\nblack");
		textTicker->addTick (63, "63");
		textTicker->addTick (127, "127");
		textTicker->addTick (191, "191");
		textTicker->addTick (255, "255\nwhite");
		axis->setTicker (textTicker);
	};
	ui.histogramView->plotLayout ()->insertRow (0);
	ui.histogramView->plotLayout ()->addElement (0, 0, new QCPTextElement (ui.histogramView, "Histograms of colour intensity", title_font));
	ui.histogramView->legend->setVisible (true);
	set_colour_axis (ui.histogramView->xAxis);
	ui.histogramView->xAxis->setLabel ("intensity level");
	ui.histogramView->yAxis->setRange (0, 50000);
	ui.histogramView->yAxis->setLabel ("count");
	struct Graph_Info_2 {
		string label;
		Qt::PenStyle pen_style;
	};
	Graph_Info_2 feature_info[] = {
	   {.label = "raw", .pen_style = Qt::SolidLine},
	   {.label = "light calibrated most common colour", .pen_style = Qt::DashLine}
	};
	for (Graph_Info_2 gi : feature_info) {
		for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
			QCPGraph *graph = this->ui.plotNumberBeesView->addGraph ();
			graph->setName (QString (("ROI " + to_string (i + 1) + " " + gi.label).c_str ()));
			graph->setPen (QPen (QColor (
			                             255 * i / (experiment.parameters.number_ROIs - 1),
			                             255 * (experiment.parameters.number_ROIs - 1 - i) / (experiment.parameters.number_ROIs - 1),
			                             128,
			                             192),
			                     1, gi.pen_style));
			graph = this->ui.plotBeeSpeedView->addGraph ();
			graph->setName (QString (("ROI " + to_string (i + 1) + " " + gi.label).c_str ()));
			graph->setPen (QPen (QColor (
			                             255 * i / (experiment.parameters.number_ROIs - 1),
			                             255 * (experiment.parameters.number_ROIs - 1 - i) / (experiment.parameters.number_ROIs - 1),
			                             128,
			                             192),
			                     1, gi.pen_style));
		}
	}
	ui.plotBeeSpeedView->plotLayout ()->insertRow (0);
	ui.plotBeeSpeedView->plotLayout ()->addElement (0, 0, new QCPTextElement (ui.plotBeeSpeedView, "Bee speed", title_font));
	this->ui.plotBeeSpeedView->legend->setVisible (true);
	set_xaxis (this->ui.plotBeeSpeedView->xAxis);
	this->ui.plotBeeSpeedView->yAxis->setRange (0, 2000);
	this->ui.plotBeeSpeedView->yAxis->setLabel ("number pixels");
	this->ui.plotNumberBeesView->legend->setVisible (true);
	set_xaxis (this->ui.plotNumberBeesView->xAxis);
	ui.plotNumberBeesView->plotLayout ()->insertRow (0);
	ui.plotNumberBeesView->plotLayout ()->addElement (0, 0, new QCPTextElement (ui.plotNumberBeesView, "Number bees", title_font));
	this->ui.plotNumberBeesView->yAxis->setRange (0, 3500);
	this->ui.plotNumberBeesView->yAxis->setLabel ("number pixels");
	Graph_Info plot_graph_info[] = {
		{.legend = "most common intensity - cropped rectangle"   , .pen = QPen (Qt::red  , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
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
	this->ui.plotColourView->yAxis->setLabel ("absolute intensity level");
	set_colour_axis (this->ui.plotColourView->yAxis);
	//   allow moving the plot ranges
	QCustomPlot *range_plots[] = {this->ui.plotColourView, this->ui.plotBeeSpeedView, this->ui.plotNumberBeesView, this->ui.histogramView};
	for (QCustomPlot *a_plot : range_plots) {
		a_plot->setInteraction (QCP::iRangeDrag, true);
		a_plot->setInteraction (QCP::iRangeZoom, true);
	}
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
	QObject::connect (ui.lightCalibratedRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.showBackgroundImageRadioButton, SIGNAL (clicked ()), this, SLOT (update_displayed_image ()));
	QObject::connect (ui.cropToRectCheckBox, SIGNAL (clicked ()), this, SLOT (crop_to_rect ()));
	QObject::connect (ui.histogramEqualisationCheckBox, SIGNAL (stateChanged (int)), this, SLOT (histogram_equalisation (int)));
	QObject::connect (ui.currentFrameSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_data (int)));
	QObject::connect (ui.updateRectPushButton, SIGNAL (clicked ()), this, SLOT (update_rect_data ()));
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &this->animate, SLOT (play_stop ()));
	QObject::connect (ui.framesPerSecondSpinBox, SIGNAL (valueChanged (double)), &this->animate, SLOT (update_playback_speed (double)));
	QObject::connect (ui.filterToIntensityCheckBox, SIGNAL (clicked ()), this, SLOT (filter_to_intensity ()));
	QObject::connect (ui.intensityAnalyseSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_filtered_intensity (int)));
	QObject::connect (ui.sameColourThresholdSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_filtered_intensity (int)));
	QObject::connect (ui.rawDataCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.lightCalibratedPlotsCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.histogramBackroundCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.currentFrameCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.lightCalibratedHistogramsCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.x1SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.x2SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.y1SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.y2SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	//
	this->experiment.parameters.equalize_histograms = this->ui.histogramEqualisationCheckBox->isChecked ();
	this->update_data (this->ui.currentFrameSpinBox->value ());
}

// SLOTS

void VideoAnalyser::update_data (int current_frame)
{
	this->update_image (current_frame);
	this->update_histogram_data (current_frame);
	this->update_plots (current_frame);
	this->update_plot_bee_speed ();
	this->update_plot_number_bees ();
	this->update_plot_colours ();
}

void VideoAnalyser::histogram_equalisation (int)
{
	this->experiment.parameters.equalize_histograms = this->ui.histogramEqualisationCheckBox->isChecked ();
	if (this->ui.showDiffPreviousRadioButton->isChecked () ||
		 this->ui.showDiffBackgroundRadioButton->isChecked ())
		this->update_image (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_displayed_image ()
{
	bool change = false;
	if (this->ui.showVideoFrameRadioButton->isChecked ()) {
		change = this->experiment.parameters.image_data != CURRENT_FRAME;
		this->experiment.parameters.image_data = CURRENT_FRAME;
	}
	else if (this->ui.showBackgroundImageRadioButton->isChecked ()) {
		change = this->experiment.parameters.image_data != BACKGROUND_IMAGE;
		this->experiment.parameters.image_data = BACKGROUND_IMAGE;
	}
	else if (this->ui.showDiffPreviousRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		change = this->experiment.parameters.image_data != DIFF_PREVIOUS_IMAGE;
		this->experiment.parameters.image_data = DIFF_PREVIOUS_IMAGE;
	}
	else if (this->ui.showDiffBackgroundRadioButton->isChecked ()) {
		change = this->experiment.parameters.image_data != DIFF_BACKGROUND_IMAGE;
		this->experiment.parameters.image_data = DIFF_BACKGROUND_IMAGE;
	}
	else if (this->ui.specialOperationRadioButton->isChecked () && (unsigned) this->ui.currentFrameSpinBox->value () > this->experiment.parameters.delta_frame) {
		change = this->experiment.parameters.image_data != SPECIAL_DATA;
		this->experiment.parameters.image_data = SPECIAL_DATA;
	}
	else if (this->ui.lightCalibratedRadioButton->isChecked ()
	         && this->experiment.highest_colour_level_frames_rect != NULL){
		change = this->experiment.parameters.image_data != LIGHT_CALIBRATED;
		this->experiment.parameters.image_data = LIGHT_CALIBRATED;
	}
	if (change) {
		this->update_image (this->ui.currentFrameSpinBox->value ());
		this->update_histogram_data (this->ui.currentFrameSpinBox->value ());
	}
}

void VideoAnalyser::crop_to_rect ()
{
	this->update_image (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_rect_data ()
{
	this->experiment.parameters.x1 = this->ui.x1SpinBox->value ();
	this->experiment.parameters.y1 = this->ui.y1SpinBox->value ();
	this->experiment.parameters.x2 = this->ui.x2SpinBox->value ();
	this->experiment.parameters.y2 = this->ui.y2SpinBox->value ();
	int current_frame = this->ui.currentFrameSpinBox->value ();
	delete this->experiment.histogram_frames_rect_raw;
	this->experiment.histogram_frames_rect_raw = compute_histogram_frames_rect (this->experiment.parameters);
	this->ui.histogramView->graph (2)->setData (X_COLOURS, (*this->experiment.histogram_frames_rect_raw) [current_frame]);
	cv::Mat cropped (
	         this->experiment.background,
	         cv::Range (this->experiment.parameters.y1, this->experiment.parameters.y2),
	         cv::Range (this->experiment.parameters.x1, this->experiment.parameters.x2));
	Histogram histogram;
	compute_histogram (cropped, histogram);
	this->ui.histogramView->graph (3)->setData (X_COLOURS, histogram);
	this->ui.histogramView->replot ();
	delete this->experiment.highest_colour_level_frames_rect;
	this->experiment.highest_colour_level_frames_rect = compute_highest_colour_level_frames_rect (this->experiment.parameters);
	this->ui.plotColourView->graph (0)->setData (this->experiment.X_FRAMES, *this->experiment.highest_colour_level_frames_rect);
	this->ui.plotColourView->replot ();
	delete this->experiment.pixel_count_difference_light_calibrated_most_common_colour;
	this->experiment.pixel_count_difference_light_calibrated_most_common_colour = compute_pixel_count_difference_light_calibrated_most_common_colour (this->experiment);
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		this->ui.plotBeeSpeedView->graph (experiment.parameters.number_ROIs + i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_light_calibrated_most_common_colour) [i * 2 + 1]);
		this->ui.plotNumberBeesView->graph (experiment.parameters.number_ROIs + i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_light_calibrated_most_common_colour) [i * 2]);
	}
}

void VideoAnalyser::filter_to_intensity ()
{
	this->update_image (this->ui.currentFrameSpinBox->value ());
	this->update_histogram_data (this->ui.currentFrameSpinBox->value ());
}

void VideoAnalyser::update_filtered_intensity (int)
{
	this->update_histogram_item (this->ui.intensityAnalyseSpinBox->value (), this->ui.sameColourThresholdSpinBox->value () * NUMBER_COLOUR_LEVELS / 100);
	if (this->ui.filterToIntensityCheckBox->isChecked ()) {
		this->update_image (this->ui.currentFrameSpinBox->value ());
	}
}

void VideoAnalyser::update_displayed_pixel_count_difference_plots ()
{
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		this->ui.plotBeeSpeedView->graph (i)->setVisible (this->ui.rawDataCheckBox->isChecked());
		this->ui.plotNumberBeesView->graph (i)->setVisible (this->ui.rawDataCheckBox->isChecked());
		this->ui.plotBeeSpeedView->graph (experiment.parameters.number_ROIs + i)->setVisible (this->ui.lightCalibratedPlotsCheckBox->isChecked());
		this->ui.plotNumberBeesView->graph (experiment.parameters.number_ROIs + i)->setVisible (this->ui.lightCalibratedPlotsCheckBox->isChecked());
	}
	this->ui.plotBeeSpeedView->replot ();
	this->ui.plotNumberBeesView->replot ();
}

void VideoAnalyser::update_displayed_histograms ()
{
	this->ui.histogramView->graph (0)->setVisible (this->ui.currentFrameCheckBox->isChecked ());
	this->ui.histogramView->graph (1)->setVisible (this->ui.histogramBackroundCheckBox->isChecked ());
	this->ui.histogramView->graph (2)->setVisible (this->ui.currentFrameCheckBox->isChecked ());
	this->ui.histogramView->graph (3)->setVisible (this->ui.histogramBackroundCheckBox->isChecked ());
	this->ui.histogramView->graph (4)->setVisible (this->ui.lightCalibratedHistogramsCheckBox->isChecked ());
	this->ui.histogramView->replot ();
}

void VideoAnalyser::rectangular_area_changed (int)
{
	ui.x1SpinBox->setMaximum (ui.x2SpinBox->value () - 1);
	ui.x2SpinBox->setMinimum (ui.x1SpinBox->value () + 1);
	ui.y1SpinBox->setMaximum (ui.y2SpinBox->value () - 1);
	ui.y2SpinBox->setMinimum (ui.y1SpinBox->value () + 1);
	if (ui.cropToRectCheckBox->isChecked ())
		this->update_image (ui.currentFrameSpinBox->value ());
}

// VideoAnalyser PRIVATE METHODS

void VideoAnalyser::update_image (int current_frame)
{
	cv::Mat image, mask;
	switch (this->experiment.parameters.image_data) {
	case BACKGROUND_IMAGE:
		image = this->experiment.background;
		break;
	case CURRENT_FRAME:
		image = read_frame (this->experiment.parameters, current_frame);
		break;
	case DIFF_BACKGROUND_IMAGE:
		image = compute_difference_background_image (this->experiment.parameters, current_frame);
		break;
	case DIFF_PREVIOUS_IMAGE:
		image = compute_difference_previous_image (this->experiment.parameters, current_frame);
		break;
	case SPECIAL_DATA:
		image = compute_threshold_mask_diff_background_diff_previous (this->experiment.parameters, current_frame);
		break;
	case LIGHT_CALIBRATED:
		image = light_calibration (this->experiment, current_frame);
		break;
	}
	if (this->ui.cropToRectCheckBox->isChecked ())
		image = cv::Mat (image, cv::Range (this->ui.y1SpinBox->value (), this->ui.y2SpinBox->value ()), cv::Range (this->ui.x1SpinBox->value (), this->ui.x2SpinBox->value ()));
	if (this->ui.filterToIntensityCheckBox->isChecked ()) {
		cv::Mat mask1, mask2, tmp_image;
		double intensity_analyse = this->ui.intensityAnalyseSpinBox->value ();
		double same_intensity_level = this->ui.sameColourThresholdSpinBox->value () * NUMBER_COLOUR_LEVELS / 100;
		if (intensity_analyse > same_intensity_level) {
			cv::threshold (image, mask1, intensity_analyse - same_intensity_level - 1, NUMBER_COLOUR_LEVELS, cv::THRESH_BINARY);
			cv::threshold (image, tmp_image, intensity_analyse - same_intensity_level - 1, 0, cv::THRESH_TOZERO);
		}
		else  {
			mask1 = cv::Mat::ones (image.size ().height, image.size ().width, CV_8UC1);
			tmp_image = image;
		}
		if (intensity_analyse + same_intensity_level < NUMBER_COLOUR_LEVELS ) {
			cv::threshold (image, mask2, intensity_analyse + same_intensity_level + 1, NUMBER_COLOUR_LEVELS, cv::THRESH_BINARY_INV);
			cv::threshold (tmp_image, image, intensity_analyse + same_intensity_level + 1, 0, cv::THRESH_TOZERO_INV);
		}
		else {
			mask2 = cv::Mat::ones (image.size ().height, image.size ().width, CV_8UC1);
			image = tmp_image;
		}
		mask = mask1 & mask2;
	}
	else {
		mask = cv::Mat::ones (image.size ().height, image.size ().width, CV_8UC1);
	}
	if (this->imageItem != NULL) {
		this->scene->removeItem (this->imageItem);
	}
	this->imageItem = this->scene->addPixmap (QPixmap::fromImage (Mat2QImage (image, mask)));
	this->ui.frameView->update ();
}

void VideoAnalyser::update_histogram_data (int current_frame)
{
	const Histogram &histogram = (*this->experiment.histogram_frames_all_raw) [current_frame];
	this->ui.histogramView->graph (0)->setData (X_COLOURS, histogram);
	if (this->experiment.histogram_frames_rect_raw != NULL) {
		const Histogram &histogram = (*this->experiment.histogram_frames_rect_raw) [current_frame];
		this->ui.histogramView->graph (2)->setData (X_COLOURS, histogram);
	}
	this->ui.histogramView->graph (4)->setVisible (this->experiment.parameters.image_data == LIGHT_CALIBRATED);
	if (this->experiment.parameters.image_data == LIGHT_CALIBRATED) {
		Histogram histogram;
		cv::Mat _image = light_calibration (this->experiment, current_frame);
		compute_histogram (_image, histogram);
		this->ui.histogramView->graph (4)->setData (X_COLOURS, histogram);
	}
	this->ui.histogramView->replot ();
}

void VideoAnalyser::update_histogram_item (int intensity_analyse, int same_intensity_level)
{
	this->intensity_analyse_line->start->setCoords (intensity_analyse, 0);
	this->intensity_analyse_line->end->setCoords (intensity_analyse, 1);
	this->intensity_span_rect->topLeft->setCoords (max (intensity_analyse - same_intensity_level, 0), 1);
	this->intensity_span_rect->bottomRight->setCoords (min (intensity_analyse + same_intensity_level, (int) NUMBER_COLOUR_LEVELS), 0);
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

QImage Mat2QImage (const cv::Mat &image, const cv::Mat &mask)
{
	QImage dest (image.cols, image.rows, QImage::Format_ARGB32);
	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			unsigned char visible = mask.at<unsigned char> (y, x);
			if (visible != 0) {
				unsigned int color = image.at<unsigned char> (y, x);
				dest.setPixel (x, y, qRgba (color, color, color, 255));
			}
			else {
				dest.setPixel (x, y, qRgba (0, 192, 0, 255));
			}
		}
	}
	return dest;
}
