#ifndef DIALOGRUNPARAMETERS_HPP
#define DIALOGRUNPARAMETERS_HPP

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class DialogRunParameters;
}

class DialogRunParameters : public QDialog
{
	Q_OBJECT

public:
	explicit DialogRunParameters(QWidget *parent = 0);
	~DialogRunParameters();
public slots:
	void selectVideoWithBackgroundImage ();
private slots:
	void on_selectVideoToAnalysePushButton_clicked ();
	void on_selectWorkingFolderPushButton_clicked ();

	void on_extractFramesPushButton_clicked();

	void on_extractBackgroundPushButton_clicked();

public:
	std::string get_folder () const;
	std::string get_frame_file_type () const;
	int get_number_ROIs () const;
private:
	Ui::DialogRunParameters *ui;
	QFileDialog *file_dialog;
};

#endif // DIALOGRUNPARAMETERS_HPP
