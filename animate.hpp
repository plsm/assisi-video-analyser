#ifndef __ANIMATE__
#define __ANIMATE__

#include <QtCore/qglobal.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "parameters.hpp"
#include "ui_video-analyser.h"

class Animate:
	public QObject
{
	Q_OBJECT;
	const Parameters &parameters;
	bool is_playing;
	QTimer *timer;
	Ui_MainWindow *ui;
public:
	Animate (const Parameters &parameters, Ui_MainWindow *ui);
	virtual ~Animate ();
public slots:
	void tick ();
	void play_stop ();
	void update_playback_speed (double v);
};

#endif
