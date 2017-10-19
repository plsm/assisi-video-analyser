#include <map>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

#include <QtCore/qglobal.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#else
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsScene>
#endif

#include "ui_video-analyser.h"
#include "parameters.hpp"

class Animate:
	public QObject
{
	Q_OBJECT;
	static QVector<double> X_COLOURS;
	const Parameters &parameters;
	UserParameters &user_parameters;
	unsigned int indexFrame;
	bool isPlaying;
	QTimer *timer;
	Ui_MainWindow *mainWindow;
	QGraphicsScene *scene;
	QGraphicsItem *imageItem;
	QVector<double> *histogramBackground;
	std::map<int, QVector<double> *> *histogramFramesAll;
	std::map<int, QVector<double> *> *histogramFramesRect;
	cv::Mat backgroundImage;
	QVector<double> *highest_colour_level_frames_rect;
	QVector<double> X_FRAMES;
	std::vector<QCPItemLine *> currentFrameLine;
public:
	static void init ();
	Animate (const Parameters &parameters, UserParameters &, Ui_MainWindow *mainWindow);
	virtual ~Animate () {}
	void setup ();
public slots:
	void update ();
	void playStop ();
	void updateRect ();
	void updatePlaybackSpeed (double v);
	void updateCurrentFrame (int f);
private:
	void updateFrame ();
	void updateHistogram ();
	void updatePlots ();
	void updateHistogram (const std::string &framePath);
	std::string getFramePath () const;
};
