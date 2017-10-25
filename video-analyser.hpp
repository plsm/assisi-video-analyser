#ifndef __VIDEO_ANALYSER__
#define __VIDEO_ANALYSER__

#include <QtCore/qglobal.h>

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
	UserParameters user_parameters;
public:
	VideoAnalyser (Experiment &experiment);
public slots:
	void update_data (int current_frame);
	void histogram_equalisation (int new_state);
	void update_displayed_image ();
	void crop_to_rect ();
	void update_rect_data ();
	void filter_to_intensity ();
	void update_filtered_intensity (int);
private:
	Animate animate;
	QGraphicsItem *imageItem;
	QGraphicsScene *scene;
	std::vector<QCPItemLine *> current_frame_line;
	QCPItemLine *intensity_analyse_line;
	QCPItemRect *intensity_span_rect;
	void update_image (int current_frame);
	void update_histogram_data (int current_frame);
	void update_histogram_item (int intensity_analyse, int same_intensity_level);
	void update_plots (int current_frame);
	void update_plot_bee_speed ();
	void update_plot_number_bees ();
	void update_plot_colours ();
};

#endif
