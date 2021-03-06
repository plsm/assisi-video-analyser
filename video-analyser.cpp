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
static double compute_max_range (const QVector<double> &data);

VideoAnalyser::VideoAnalyser (Experiment &experiment):
	experiment (experiment),
	animate (experiment.parameters, &ui),
	scene (new QGraphicsScene ()),
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
   pixmap (new QGraphicsPixmapItem (NULL, scene)),
	roi (new QGraphicsRectItem (0, 0, experiment.parameters.frame_size.width, experiment.parameters.frame_size.height, NULL, scene)),
#else
   pixmap (new QGraphicsPixmapItem (NULL)),
   roi (new QGraphicsRectItem (0, 0, experiment.parameters.frame_size.width, experiment.parameters.frame_size.height, NULL)),
#endif
   current_frame_line (3),
   most_common_colour_histogram_no_cropping (2),
   most_common_colour_histogram_cropped_rectangle (2)
{
	ui.setupUi (this);
	// default colours to use in plots to distinguish different ROIs
	for (unsigned int i = 0; i < this->experiment.parameters.number_ROIs; i++) {
		QColor color;
		color.setHsl (i * 255 / (this->experiment.parameters.number_ROIs - 1), 255, 127, 192);
		this->mask_colour.push_back (color);
	}
	// initialise the widget where an image is show
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	scene->addItem (pixmap);
	scene->addItem (roi);
#endif
	this->ui.frameView->setScene (this->scene);
	roi->setFlag (QGraphicsItem::ItemIsMovable);
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
	this->intensity_analyse_line = new QCPItemLine (this->ui.histogramSelectedFramesView);
	this->intensity_analyse_line->start->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_analyse_line->start->setCoords (1, 0);
	this->intensity_analyse_line->end->setCoords (1, 1);
	this->intensity_analyse_line->setPen (QPen (QColor (0, 0, 0, 127), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	this->intensity_span_rect = new QCPItemRect (this->ui.histogramSelectedFramesView);
	this->intensity_span_rect->topLeft->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_span_rect->topLeft->setCoords (0, 1);
	this->intensity_span_rect->bottomRight->setTypeY (QCPItemPosition::ptAxisRectRatio);
	this->intensity_span_rect->bottomRight->setCoords (0, 0);
	this->intensity_span_rect->setPen (QPen (QColor (127, 127, 127, 63), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	this->intensity_span_rect->setBrush (QBrush (QColor (127, 127, 127, 63)));
	// setup widgets
	//   same colour threshold
	this->ui.sameColourThresholdSpinBox->setValue (experiment.parameters.get_same_colour_threshold ());
	//   rectangle to analyse
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
	//    setup qcustom plot widgets
	double maximum;
	QCPGraph *graph;
	QColor color;
	auto add_title = [] (auto custom_plot, const char *title) {
		QFont title_font ("sans", 10, QFont::Bold);
		custom_plot->plotLayout ()->insertRow (0);
		custom_plot->plotLayout ()->addElement (0, 0, new QCPTextElement (custom_plot, title, title_font));
	};
	struct Graph_Info {
		QString legend;
		QPen pen;
	};
	Graph_Info graph_info[] = {
	   {.legend = "current frame - no cropping"       , .pen = QPen (Qt::green   , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "background - no cropping"          , .pen = QPen (Qt::magenta , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "current frame - cropped rectangle" , .pen = QPen (Qt::blue    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "background - cropped rectangle"    , .pen = QPen (Qt::red     , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "displayed image"                   , .pen = QPen (QColor (127, 127, 0)  , 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)}
	};
	for (int i = 0; i < 5; i++) {
		graph = ui.histogramSelectedFramesView->addGraph ();
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
	add_title (ui.histogramSelectedFramesView, "Histograms of colour intensity");
	ui.histogramSelectedFramesView->legend->setVisible (true);
	set_colour_axis (ui.histogramSelectedFramesView->xAxis);
	ui.histogramSelectedFramesView->xAxis->setLabel ("intensity level");
	maximum = compute_max_range (*experiment.histogram_background_raw);
	for (pair<const int, Histogram> &h : *experiment.histogram_frames_all_raw)
		maximum = std::max (maximum, compute_max_range (h.second));
	ui.histogramSelectedFramesView->yAxis->setRange (0, maximum);
	ui.histogramSelectedFramesView->yAxis->setLabel ("count");
	//     qcustom plot widget with histogram of all frames
	const string labels[] = {
	  "raw",
	   "light calibrated most common colour (PLSM)",
	   "light calibrated most common colour (LC)",
	};
	int d = 0;
	for (const string &a_label : labels) {
		for (unsigned int i = 0; i < experiment.parameters.number_frames; i++) {
			ui.histogramAllFramesView->setAutoAddPlottableToLegend (i == 0 || i == experiment.parameters.number_frames - 1);
			graph = ui.histogramAllFramesView->addGraph ();
			color = QColor (
			         255 * i / (experiment.parameters.number_frames - 1),
			         i <= experiment.parameters.number_frames / 2 ? 255 * i / experiment.parameters.number_frames / 2 : 0,
			         i > experiment.parameters.number_frames / 2 ? 255 * (i - experiment.parameters.number_frames / 2) / experiment.parameters.number_frames / 2 : 0,
			         192);
			color = QColor (
			         d == 0 ? 255 * i / (experiment.parameters.number_frames - 1) : 63,
			         d == 1 ? 255 * i / (experiment.parameters.number_frames - 1) : 63,
			         d == 2 ? 255 * i / (experiment.parameters.number_frames - 1) : 63,
			         192);
			graph->setPen (QPen (color, 1, Qt::SolidLine));
			if (i == 0)
				graph->setName (("first frame " + a_label).c_str ());
			else if (i == experiment.parameters.number_frames - 1)
				graph->setName (("last frame " + a_label).c_str ());
		}
		d++;
	}
	add_title (ui.histogramAllFramesView, "Histograms of colour intensity");
	ui.histogramAllFramesView->legend->setVisible (true);
	set_colour_axis (ui.histogramAllFramesView->xAxis);
	ui.histogramAllFramesView->xAxis->setLabel ("intensity level");
	ui.histogramAllFramesView->yAxis->setRange (0, maximum);
	ui.histogramAllFramesView->yAxis->setLabel ("count");
	struct Graph_Info_2 {
		string label;
		Qt::PenStyle pen_style;
	};
	Graph_Info_2 feature_info[] = {
	   {.label = "raw", .pen_style = Qt::SolidLine},
	   {.label = "histogram equalisation", .pen_style = Qt::DashDotLine},
	   {.label = "light calibrated most common colour (PLSM)", .pen_style = Qt::DashLine},
	   {.label = "light calibrated most common colour (LC)", .pen_style = Qt::DotLine},
	};
	for (Graph_Info_2 gi : feature_info) {
		for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
			QColor color = this->mask_colour [i];
			graph = this->ui.plotNumberBeesView->addGraph ();
			graph->setName (QString (("ROI " + to_string (i + 1) + " " + gi.label).c_str ()));
			graph->setPen (QPen (color, 1, gi.pen_style));
			graph = this->ui.plotBeeSpeedView->addGraph ();
			graph->setName (QString (("ROI " + to_string (i + 1) + " " + gi.label).c_str ()));
			graph->setPen (QPen (color, 1, gi.pen_style));
		}
	}
	add_title (ui.plotBeeSpeedView, "Bee speed");
	this->ui.plotBeeSpeedView->legend->setVisible (true);
	set_xaxis (this->ui.plotBeeSpeedView->xAxis);
	maximum = 0;
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		maximum = std::max (maximum, compute_max_range ((*experiment.pixel_count_difference_raw) [2 * i + 1]));
		maximum = std::max (maximum, compute_max_range ((*experiment.pixel_count_difference_histogram_equalisation) [2 * i + 1]));
	}
	this->ui.plotBeeSpeedView->yAxis->setRange (0, maximum);
	this->ui.plotBeeSpeedView->yAxis->setLabel ("number pixels");
	this->ui.plotNumberBeesView->legend->setVisible (true);
	set_xaxis (this->ui.plotNumberBeesView->xAxis);
	add_title (ui.plotNumberBeesView, "Number bees");
	maximum = 0;
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		maximum = std::max (maximum, compute_max_range ((*experiment.pixel_count_difference_raw) [2 * i]));
		maximum = std::max (maximum, compute_max_range ((*experiment.pixel_count_difference_histogram_equalisation) [2 * i]));
	}
	this->ui.plotNumberBeesView->yAxis->setRange (0, maximum);
	this->ui.plotNumberBeesView->yAxis->setLabel ("number pixels");
	Graph_Info plot_graph_info[] = {
	   {.legend = "most common intensity - background - no cropping"       , .pen = QPen (Qt::magenta , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "most common intensity - frames - cropped rectangle"     , .pen = QPen (Qt::blue    , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	   {.legend = "most common intensity - background - cropped rectangle" , .pen = QPen (Qt::red     , 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)},
	};
	for (Graph_Info pgi : plot_graph_info) {
		graph = this->ui.plotColourView->addGraph ();
		graph->setName (pgi.legend);
		graph->setPen (pgi.pen);
	}
	this->ui.plotColourView->legend->setVisible (true);
	this->ui.plotColourView->xAxis->setRange (0, experiment.parameters.number_frames);
	this->ui.plotColourView->xAxis->setLabel ("frame");
	set_xaxis (this->ui.plotColourView->xAxis);
	this->ui.plotColourView->yAxis->setLabel ("absolute intensity level");
	set_colour_axis (this->ui.plotColourView->yAxis);
	//   allow moving the plot ranges
	QCustomPlot *range_plots[] = {this->ui.plotColourView, this->ui.plotBeeSpeedView, this->ui.plotNumberBeesView, this->ui.histogramSelectedFramesView};
	for (QCustomPlot *a_plot : range_plots) {
		a_plot->setInteraction (QCP::iRangeDrag, true);
		a_plot->setInteraction (QCP::iRangeZoom, true);
	}
	// show constant data
	ui.histogramSelectedFramesView->graph (1)->setData (X_COLOURS, *experiment.histogram_background_raw);
	for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
		this->ui.plotBeeSpeedView->graph (i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_raw) [i * 2 + 1]);
		this->ui.plotNumberBeesView->graph (i)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_raw) [i * 2]);
		this->ui.plotBeeSpeedView->graph (i + experiment.parameters.number_ROIs)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_histogram_equalisation) [i * 2 + 1]);
		this->ui.plotNumberBeesView->graph (i + experiment.parameters.number_ROIs)->setData (experiment.X_FRAMES, (*experiment.pixel_count_difference_histogram_equalisation) [i * 2]);
	}
	most_common_colour_histogram_no_cropping [0] =
	most_common_colour_histogram_no_cropping [1] = experiment.histogram_background_raw->most_common_colour ();
	this->ui.plotColourView->graph (0)->setData (experiment.X_FIRST_LAST_FRAMES, most_common_colour_histogram_no_cropping);
	for (unsigned int i = 0; i < experiment.parameters.number_frames; i++) {
		this->ui.histogramAllFramesView->graph (i)->setData (X_COLOURS, (*experiment.histogram_frames_all_raw).operator [] (i));
	}
	// setup connection between signals and slots
	QObject::connect (ui.currentFrameSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_data (int)));
	QObject::connect (ui.updateRectPushButton, SIGNAL (clicked ()), this, SLOT (update_rect_data ()));
	QObject::connect (ui.playStopButton, SIGNAL (clicked ()), &this->animate, SLOT (play_stop ()));
	QObject::connect (ui.framesPerSecondSpinBox, SIGNAL (valueChanged (double)), &this->animate, SLOT (update_playback_speed (double)));
	QObject::connect (ui.intensityAnalyseSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_filtered_intensity (int)));
	QObject::connect (ui.sameColourThresholdSpinBox, SIGNAL (valueChanged (int)), this, SLOT (update_filtered_intensity (int)));
	QObject::connect (ui.rawDataCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.showNumberBeesBeeSpeedPlotsHistogramEqualisationCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.showLightCalibratedPlotsLCMethodCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.showLightCalibratedPlotsPLSMMethodCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_pixel_count_difference_plots ()));
	QObject::connect (ui.showHistogramsBackroundCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.showHistogramsCurrentFrameCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.showHistogramDisplayedImageCheckBox, SIGNAL (clicked ()), this, SLOT (update_displayed_histograms ()));
	QObject::connect (ui.x1SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.x2SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.y1SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.y2SpinBox, SIGNAL (valueChanged (int)), this, SLOT (rectangular_area_changed (int)));
	QObject::connect (ui.updateSameColourThresholdDataPushButton, SIGNAL (clicked ()), this, SLOT (update_same_colour_data ()));
	//
	this->update_data (this->ui.currentFrameSpinBox->value ());
}

// SLOTS

void VideoAnalyser::update_data (int current_frame)
{
	this->update_displayed_image ();
	this->update_histograms_current_frame (current_frame);
	this->update_plots (current_frame);
	this->update_plot_bee_speed ();
	this->update_plot_number_bees ();
	this->update_plot_colours ();
	this->ui.showDiffPreviousRadioButton->setEnabled (current_frame > (int) experiment.parameters.delta_frame + 1);
}

void VideoAnalyser::update_displayed_image ()
{
//	printf ("void VideoAnalyser::update_displayed_image ()\n");
	unsigned int current_frame = this->ui.currentFrameSpinBox->value ();
	int x1 = ui.x1SpinBox->value ();
	int y1 = ui.y1SpinBox->value ();
	int x2 = ui.x2SpinBox->value ();
	int y2 = ui.y2SpinBox->value ();
	cv::Mat processed_image;
	// first step
	auto pre_process_background = [&] () {
		if (ui.noPreProcessedImageRadioButton->isChecked ())
			return experiment.background;
		else if (ui.lightCalibratedPLSMMethodRadioButton->isChecked ())
			return experiment.background;
		else if (ui.lightCalibratedLCMethodRadioButton->isChecked ())
			return experiment.background;
		else if (ui.histogramEqualisationRadioButton->isChecked ()) {
			cv::Mat result;
			equalizeHist (experiment.background, result);
			return result;
		}
		throw "missing pre-processing option";
	};
	auto pre_process_frame = [&] (unsigned int index_frame) {
		if (this->ui.noPreProcessedImageRadioButton->isChecked ())
			return read_frame (experiment.parameters, index_frame);
		else if (this->ui.lightCalibratedPLSMMethodRadioButton->isChecked ())
			return light_calibrate (this->experiment, index_frame, x1, y1, x2, y2, light_calibrate_method_PLSM);
		else if (this->ui.lightCalibratedLCMethodRadioButton->isChecked ())
			return light_calibrate (this->experiment, index_frame, x1, y1, x2, y2, light_calibrate_method_LC);
		else if (ui.histogramEqualisationRadioButton->isChecked ()) {
			cv::Mat result;
			cv::Mat frame = read_frame (experiment.parameters, index_frame);
			equalizeHist (frame, result);
			return result;
		}
		throw "missing pre-processing option";
	};
	// second step
	if (this->ui.showBackgroundImageRadioButton->isChecked ())
		processed_image = pre_process_background ();
	else if (this->ui.showVideoFrameRadioButton->isChecked ())
		processed_image = pre_process_frame (current_frame);
	else if (this->ui.showDiffBackgroundRadioButton->isChecked ())
		cv::absdiff (
		         pre_process_background (),
		         pre_process_frame (current_frame),
		         processed_image);
	else if (this->ui.showDiffPreviousRadioButton->isChecked ())
		cv::absdiff (
		         pre_process_frame (current_frame - 1 - experiment.parameters.delta_frame),
		         pre_process_frame (current_frame),
		         processed_image);
	else if (this->ui.specialOperationRadioButton->isChecked ())
		processed_image = compute_threshold_mask_diff_background_diff_previous (this->experiment.parameters, current_frame);
	// third step
	if (this->ui.cropToRectCheckBox->isChecked ())
		displayed_image = cv::Mat (processed_image, cv::Range (y1, y2), cv::Range (x1, x2));
	else
		displayed_image = processed_image;
	// fourth step
	cv::Mat mask;
	if (this->ui.filterToIntensityCheckBox->isChecked ()) {
		cv::Mat mask1, mask2, tmp_image;
		double intensity_analyse;
		double same_intensity_level;
		if (ui.showBackgroundImageRadioButton->isChecked () ||
		    ui.showVideoFrameRadioButton->isChecked ()) {
			intensity_analyse = this->ui.intensityAnalyseSpinBox->value ();
			same_intensity_level = this->ui.sameColourThresholdSpinBox->value () * NUMBER_COLOUR_LEVELS / 100;
		}
		else if (ui.showDiffBackgroundRadioButton->isChecked () ||
		         ui.showDiffPreviousRadioButton->isChecked ()) {
			intensity_analyse = NUMBER_COLOUR_LEVELS;
			same_intensity_level = (100 - this->ui.sameColourThresholdSpinBox->value ()) * NUMBER_COLOUR_LEVELS / 100;
		}
		if (intensity_analyse > same_intensity_level) {
			cv::threshold (displayed_image, mask1, intensity_analyse - same_intensity_level - 1, NUMBER_COLOUR_LEVELS, cv::THRESH_BINARY);
			cv::threshold (displayed_image, tmp_image, intensity_analyse - same_intensity_level - 1, 0, cv::THRESH_TOZERO);
		}
		else  {
			mask1 = cv::Mat::ones (displayed_image.size ().height, displayed_image.size ().width, CV_8UC1);
			tmp_image = displayed_image;
		}
		if (intensity_analyse + same_intensity_level < NUMBER_COLOUR_LEVELS ) {
			cv::threshold (displayed_image, mask2, intensity_analyse + same_intensity_level + 1, NUMBER_COLOUR_LEVELS, cv::THRESH_BINARY_INV);
			cv::threshold (tmp_image, displayed_image, intensity_analyse + same_intensity_level + 1, 0, cv::THRESH_TOZERO_INV);
		}
		else {
			mask2 = cv::Mat::ones (displayed_image.size ().height, displayed_image.size ().width, CV_8UC1);
			displayed_image = tmp_image;
		}
		mask = mask1 & mask2;
	}
	else {
		mask = cv::Mat::ones (displayed_image.size ().height, displayed_image.size ().width, CV_8UC1);
	}
	// update widget
	this->pixmap->setPixmap (QPixmap::fromImage (Mat2QImage (displayed_image, mask)));
	this->ui.frameView->update ();
	this->update_histogram_displayed_image ();
	this->update_image_display_options ();
	this->ui.showHistogramDisplayedImageCheckBox->setEnabled (this->displayed_image_has_histogram_to_show ());
}

void VideoAnalyser::update_rect_data ()
{
	int x1 = this->ui.x1SpinBox->value ();
	int y1 = this->ui.y1SpinBox->value ();
	int x2 = this->ui.x2SpinBox->value ();
	int y2 = this->ui.y2SpinBox->value ();
	int current_frame = this->ui.currentFrameSpinBox->value ();
	this->experiment.set_rect_data (x1, y1, x2, y2);
	// histograms of selected frames
	//    histogram of current frame - no cropping
	this->ui.histogramSelectedFramesView->graph (2)->setData (X_COLOURS, (*this->experiment.histogram_frames_rect_raw) [current_frame]);
	//    histogram of background - cropped rectangle
	cv::Mat cropped (this->experiment.background, cv::Range (y1, y2), cv::Range (x1, x2));
	Histogram histogram;
	compute_histogram (cropped, histogram);
	this->ui.histogramSelectedFramesView->graph (3)->setData (X_COLOURS, histogram);
	this->ui.histogramSelectedFramesView->replot ();
	// plot of most common colour in rectangular area in frame vs frame
	this->ui.plotColourView->graph (1)->setData (this->experiment.X_FRAMES, *this->experiment.highest_colour_level_frames_rect);
	most_common_colour_histogram_cropped_rectangle [0] =
	      most_common_colour_histogram_cropped_rectangle [1] =
	      histogram.most_common_colour ();
	this->ui.plotColourView->graph (2)->setData (experiment.X_FIRST_LAST_FRAMES, most_common_colour_histogram_cropped_rectangle);
	this->ui.plotColourView->replot ();
	// plots with number of bees and bee speed using light calibrated data
	std::vector<QVector<double> > *pixel_count_difference[] = {
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_PLSM,
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_LC,
	};
	int d = 2 * experiment.parameters.number_ROIs;
	for (std::vector<QVector<double> > *pcd : pixel_count_difference) {
		for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
			this->ui.plotBeeSpeedView->graph (d + i)->setData (experiment.X_FRAMES, (*pcd) [i * 2 + 1]);
			this->ui.plotNumberBeesView->graph (d + i)->setData (experiment.X_FRAMES, (*pcd) [i * 2]);
		}
		d += experiment.parameters.number_ROIs;
	}
	this->update_PCD_plots_yAxis_range ();
	this->ui.plotBeeSpeedView->replot ();
	this->ui.plotNumberBeesView->replot ();
	// histograms of all frames that have been light calibrated
	for (unsigned int i = 0; i < experiment.parameters.number_frames; i++) {
		this->ui.histogramAllFramesView->graph (i + experiment.parameters.number_frames)->setData (
		      X_COLOURS,
		      this->experiment.histogram_frames_light_calibrated_most_common_colour_method_PLSM->operator [] (i),
		      true);
		this->ui.histogramAllFramesView->graph (i + 2 * experiment.parameters.number_frames)->setData (
		      X_COLOURS,
		      this->experiment.histogram_frames_light_calibrated_most_common_colour_method_LC->operator [] (i),
		      true);
	}
	this->ui.histogramAllFramesView->replot ();
	// other stuff
	this->ui.showHistogramsAllFramesLightCalibratedLCRadioButton->setEnabled (true);
	this->ui.showHistogramsAllFramesLightCalibratedPLSMRadioButton->setEnabled (true);
}

void VideoAnalyser::update_filtered_intensity (int)
{
	this->update_histogram_item (this->ui.intensityAnalyseSpinBox->value (), this->ui.sameColourThresholdSpinBox->value () * NUMBER_COLOUR_LEVELS / 100);
	if (this->ui.filterToIntensityCheckBox->isChecked ()) {
		this->update_displayed_image ();
	}
}

void VideoAnalyser::update_displayed_pixel_count_difference_plots ()
{
	QCheckBox *check_boxes [] = {
	   this->ui.rawDataCheckBox,
	   this->ui.showNumberBeesBeeSpeedPlotsHistogramEqualisationCheckBox,
	   this->ui.showLightCalibratedPlotsPLSMMethodCheckBox,
	   this->ui.showLightCalibratedPlotsLCMethodCheckBox,
	};
	int d = 0;
	for (QCheckBox *check_box : check_boxes) {
		for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
			this->ui.plotBeeSpeedView->graph (i + d)->setVisible (check_box->isChecked());
			this->ui.plotNumberBeesView->graph (i + d)->setVisible (check_box->isChecked());
		}
		d += experiment.parameters.number_ROIs;
	}
	this->ui.plotBeeSpeedView->replot ();
	this->ui.plotNumberBeesView->replot ();
}

void VideoAnalyser::update_displayed_histograms ()
{
	this->ui.histogramSelectedFramesView->graph (0)->setVisible (this->ui.showHistogramsCurrentFrameCheckBox->isChecked ());
	this->ui.histogramSelectedFramesView->graph (1)->setVisible (this->ui.showHistogramsBackroundCheckBox->isChecked ());
	this->ui.histogramSelectedFramesView->graph (2)->setVisible (this->ui.showHistogramsCurrentFrameCheckBox->isChecked ());
	this->ui.histogramSelectedFramesView->graph (3)->setVisible (this->ui.showHistogramsBackroundCheckBox->isChecked ());
	this->ui.histogramSelectedFramesView->graph (4)->setVisible (
	         this->ui.showHistogramDisplayedImageCheckBox->isChecked ()
	         && this->displayed_image_has_histogram_to_show ());
	this->ui.histogramSelectedFramesView->replot ();
}

void VideoAnalyser::update_displayed_histograms_all_frames ()
{
	static QRadioButton *radio_buttons [] = {
	   this->ui.showHistogramsAllFramesRawRadioButton,
	   this->ui.showHistogramsAllFramesLightCalibratedPLSMRadioButton,
	   this->ui.showHistogramsAllFramesLightCalibratedLCRadioButton,
	};
	int d = 0;
	for (QRadioButton *radio_button : radio_buttons) {
		for (unsigned int i = 0; i < experiment.parameters.number_frames; i++)
			this->ui.histogramAllFramesView->graph (i + d)->setVisible (radio_button->isChecked ());
		d += experiment.parameters.number_frames;
	}
	this->ui.histogramAllFramesView->replot ();
}

void VideoAnalyser::rectangular_area_changed (int)
{
	int x1 = ui.x1SpinBox->value ();
	int y1 = ui.y1SpinBox->value ();
	int x2 = ui.x2SpinBox->value ();
	int y2 = ui.y2SpinBox->value ();
	ui.x1SpinBox->setMaximum (x2 - 1);
	ui.x2SpinBox->setMinimum (x1 + 1);
	ui.y1SpinBox->setMaximum (y2 - 1);
	ui.y2SpinBox->setMinimum (y1 + 1);
	this->roi->setRect (x1, y1, x2 - x1, y2 - y1);
	if (ui.cropToRectCheckBox->isChecked ())
		this->update_displayed_image ();
}

void VideoAnalyser::update_same_colour_data ()
{
	printf ("SCT=%d\n", this->ui.sameColourThresholdSpinBox->value ());
	this->experiment.parameters.set_same_colour_threshold (this->ui.sameColourThresholdSpinBox->value ());
	this->experiment.pixel_count_difference_raw = compute_pixel_count_difference_raw (this->experiment);
	this->experiment.pixel_count_difference_histogram_equalisation = compute_pixel_count_difference_histogram_equalization (this->experiment);
	if (this->experiment.highest_colour_level_frames_rect != NULL) {
		this->experiment.pixel_count_difference_light_calibrated_most_common_colour_method_PLSM =
		      compute_pixel_count_difference_light_calibrated_most_common_colour_method_PLSM (this->experiment);
		this->experiment.pixel_count_difference_light_calibrated_most_common_colour_method_LC =
		      compute_pixel_count_difference_light_calibrated_most_common_colour_method_LC (this->experiment);
	}
	// update the QCustomPlots
	this->update_PCD_plots_yAxis_range ();
	std::vector<QVector<double> > *pixel_count_difference[] = {
	   experiment.pixel_count_difference_raw,
	   experiment.pixel_count_difference_histogram_equalisation,
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_PLSM,
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_LC,
	};
	int d = 0;
	for (std::vector<QVector<double> > *pcd : pixel_count_difference) {
		if (pcd != NULL)
			for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++) {
				this->ui.plotBeeSpeedView->graph (d + i)->setData (experiment.X_FRAMES, (*pcd) [i * 2 + 1]);
				this->ui.plotNumberBeesView->graph (d + i)->setData (experiment.X_FRAMES, (*pcd) [i * 2]);
			}
		d += experiment.parameters.number_ROIs;
	}
	this->ui.plotBeeSpeedView->replot ();
	this->ui.plotNumberBeesView->replot ();
}

// VideoAnalyser PRIVATE METHODS

bool VideoAnalyser::displayed_image_has_histogram_to_show ()
{
	return
	      (this->ui.showVideoFrameRadioButton->isChecked () && (
	         this->ui.lightCalibratedLCMethodRadioButton->isChecked ()
	         || this->ui.lightCalibratedPLSMMethodRadioButton->isChecked ()
	         ))
	      || this->ui.showDiffBackgroundRadioButton->isChecked ()
	      || this->ui.showDiffPreviousRadioButton->isChecked ()
	      ;
}

void VideoAnalyser::update_histograms_current_frame (int current_frame)
{
	const Histogram &histogram = (*this->experiment.histogram_frames_all_raw) [current_frame];
	this->ui.histogramSelectedFramesView->graph (0)->setData (X_COLOURS, histogram);
	if (this->experiment.histogram_frames_rect_raw != NULL) {
		const Histogram &histogram = (*this->experiment.histogram_frames_rect_raw) [current_frame];
		this->ui.histogramSelectedFramesView->graph (2)->setData (X_COLOURS, histogram);
	}
	this->ui.histogramSelectedFramesView->replot ();
}

void VideoAnalyser::update_histogram_displayed_image ()
{
//	printf ("void VideoAnalyser::update_histogram_displayed_image (bool)\n");
	bool visible =
	      this->displayed_image_has_histogram_to_show ()
	      && this->ui.showHistogramDisplayedImageCheckBox->isChecked ();
	this->ui.histogramSelectedFramesView->graph (4)->setVisible (visible);
	if (visible) {
		Histogram histogram;
		compute_histogram (displayed_image, histogram);
		this->ui.histogramSelectedFramesView->graph (4)->setData (X_COLOURS, histogram);
	}
	this->ui.histogramSelectedFramesView->replot ();
}

void VideoAnalyser::update_histogram_item (int intensity_analyse, int same_intensity_level)
{
	this->intensity_analyse_line->start->setCoords (intensity_analyse, 0);
	this->intensity_analyse_line->end->setCoords (intensity_analyse, 1);
	this->intensity_span_rect->topLeft->setCoords (max (intensity_analyse - same_intensity_level, 0), 1);
	this->intensity_span_rect->bottomRight->setCoords (min (intensity_analyse + same_intensity_level, (int) NUMBER_COLOUR_LEVELS), 0);
	this->ui.histogramSelectedFramesView->replot ();
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

void VideoAnalyser::update_image_display_options ()
{
	bool flag = this->ui.showBackgroundImageRadioButton->isChecked () ||
	      this->ui.specialOperationRadioButton->isChecked ();
	this->ui.lightCalibratedLCMethodRadioButton->setEnabled (!flag);
	this->ui.lightCalibratedPLSMMethodRadioButton->setEnabled (!flag);
	this->ui.histogramEqualisationRadioButton->setEnabled (
	         !this->ui.specialOperationRadioButton->isChecked ());
}

void VideoAnalyser::update_PCD_plots_yAxis_range ()
{
	double maximum;
	std::vector<QVector<double> > *PCDs [] = {
	   experiment.pixel_count_difference_raw,
	   experiment.pixel_count_difference_histogram_equalisation,
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_LC,
	   experiment.pixel_count_difference_light_calibrated_most_common_colour_method_PLSM
	};
	maximum = 0;
	for (std::vector<QVector<double> > *a_pcd : PCDs)
		if (a_pcd != NULL) {
			for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++)
				maximum = std::max (maximum, compute_max_range ((*a_pcd) [2 * i + 1]));
		}
	this->ui.plotBeeSpeedView->yAxis->setRange (0, maximum);
	maximum = 0;
	for (std::vector<QVector<double> > *a_pcd : PCDs)
		if (a_pcd != NULL) {
			for (unsigned int i = 0; i < experiment.parameters.number_ROIs; i++)
				maximum = std::max (maximum, compute_max_range ((*a_pcd) [2 * i]));
		}
	this->ui.plotNumberBeesView->yAxis->setRange (0, maximum);
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

double compute_max_range (const QVector<double> &data)
{
	double maximum = data [0];
	for (double x : data)
		maximum = max (x, maximum);
	if (maximum == 0)
		return 0;
	double power = ceil (log10 (maximum));
	double result = pow (10, power);
	double delta = pow (10, power - 1) / 2;
	while (result - delta >= maximum)
		result -= delta;
	return result;
}
