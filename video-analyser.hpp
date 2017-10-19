#ifndef __VIDEO_ANALYSER__
#define __VIDEO_ANALYSER__

#include <QtGui/QMainWindow>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#else
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsScene>
#endif

#include "experiment.hpp"
#include "ui_video-analyser.h"

class VideoAnalyser:
	public QMainWindow
{
	Q_OBJECT;
	Ui_MainWindow ui;
	Experiment &experiment;
	UserParameters user_parameters;
public:
	VideoAnalyser (Experiment &experiment);
public slots:
	void update_data (int current_frame);
	void histogram_equalisation (int new_state);
	void update_displayed_frame (bool _);
private:
	QGraphicsItem *imageItem;
	QGraphicsScene *scene;
	void update_frame ();
	void update_histogram ();
	void update_plot_bee_speed ();
	void update_plot_number_bees ();
	void update_plot_colours ();
};

#endif
