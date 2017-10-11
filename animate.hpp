#include <map>
#include <opencv2/opencv.hpp>
#include <qobject.h>
#include <qtimer.h>
#include <qgraphicsscene.h>
#include <qgraphicsitem.h>
#include <string>

#include "ui_video-analyser.h"

class Animate:
	public QObject
{
	Q_OBJECT;
	static QVector<double> X_COLOURS;
	const std::string folder;
	const std::string frameFileType;
	int indexFrame;
	bool isPlaying;
	QTimer *timer;
	Ui_MainWindow *mainWindow;
	QGraphicsScene *scene;
	QGraphicsItem *imageItem;
	QVector<double> *histogramBackground;
	std::map<int, QVector<double> *> *histogramFramesAll;
	std::map<int, QVector<double> *> *histogramFramesRect;
	cv::Mat backgroundImage;
public:
	static void init ();
	Animate (std::string folder, std::string frameFileType, Ui_MainWindow *mainWindow);
	virtual ~Animate () {}
	void setup ();
public slots:
	void update ();
	void playStop ();
	void updateHistogramsRect ();
private:
	void updateFrame ();
	void updateHistogram ();
	void updateHistogram (const std::string &framePath);
	std::string getFramePath () const;
};
