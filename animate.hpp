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
	const std::string folder;
	const std::string frameFileType;
	int indexFrame;
	bool isPlaying;
	QTimer *timer;
	Ui_MainWindow *mainWindow;
	QGraphicsScene *scene;
	QGraphicsItem *imageItem;
public:
	Animate (std::string folder, std::string frameFileType, Ui_MainWindow *mainWindow);
	virtual ~Animate () {}
	void setup ();
public slots:
	void update ();
	void playStop ();
private:
	void updateHistogram (const std::string &framePath);
	std::string getFramePath () const;
};
