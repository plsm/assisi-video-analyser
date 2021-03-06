#ifndef __VIDEO_ANALYSER__
#define __VIDEO_ANALYSER__

#include <QtCore/qglobal.h>
#include <QGraphicsRectItem>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsScene>
#endif

#include "experiment.hpp"
#include "ui_video-analyser.h"
#include "animate.hpp"

class VideoAnalyser:
	public QMainWindow
{
	Q_OBJECT
	Ui_MainWindow ui;
	Experiment &experiment;
public:
	VideoAnalyser (Experiment &experiment);
public slots:
	void update_data (int current_frame);
	void update_displayed_image ();
	void update_rect_data ();
	void update_filtered_intensity (int);
	void update_displayed_pixel_count_difference_plots ();
	void update_displayed_histograms ();
	void update_displayed_histograms_all_frames ();
	void rectangular_area_changed (int);
	void update_same_colour_data ();
private:
	Animate animate;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *pixmap;
	QGraphicsRectItem *roi;
	std::vector<QCPItemLine *> current_frame_line;
	QCPItemLine *intensity_analyse_line;
	QCPItemRect *intensity_span_rect;
	QVector<double> most_common_colour_histogram_no_cropping;
	QVector<double> most_common_colour_histogram_cropped_rectangle;
	std::vector<QColor> mask_colour;
	cv::Mat displayed_image;
	bool displayed_image_has_histogram_to_show ();
	void update_histograms_current_frame (int current_frame);
	void update_histogram_displayed_image ();
	void update_histogram_item (int intensity_analyse, int same_intensity_level);
	void update_plots (int current_frame);
	void update_plot_bee_speed ();
	void update_plot_number_bees ();
	void update_plot_colours ();
	void update_image_display_options ();
	void update_PCD_plots_yAxis_range ();
};

#endif
