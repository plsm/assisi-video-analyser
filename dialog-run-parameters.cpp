#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "dialog-run-parameters.hpp"
#include "ui_dialog-run-parameters.h"

using namespace std;

static void split_video (const string &video_path, const string &location, int frames_per_second, const string &frame_template);

DialogRunParameters::DialogRunParameters(QWidget *parent) :
   QDialog (parent),
   ui (new Ui::DialogRunParameters),
   file_dialog (new QFileDialog (this))
{
	ui->setupUi(this);
}

DialogRunParameters::~DialogRunParameters()
{
	delete ui;
}

void DialogRunParameters::selectVideoWithBackgroundImage ()
{
	this->file_dialog->setViewMode (QFileDialog::List);
	this->file_dialog->setOption (QFileDialog::ShowDirsOnly, false);
	this->file_dialog->setFileMode (QFileDialog::ExistingFiles);
	int return_code = this->file_dialog->exec ();
	if (return_code == QDialog::Accepted) {
		QStringList strings = this->file_dialog->selectedFiles ();
		this->ui->videoWithBackgroundImageLineEdit->setText (strings.at (0));
	}
}

void DialogRunParameters::on_selectVideoToAnalysePushButton_clicked()
{
	this->file_dialog->setViewMode (QFileDialog::List);
	this->file_dialog->setOption (QFileDialog::ShowDirsOnly, false);
	this->file_dialog->setFileMode (QFileDialog::ExistingFiles);
	int return_code = this->file_dialog->exec ();
	if (return_code == QDialog::Accepted) {
		QStringList strings = this->file_dialog->selectedFiles ();
		this->ui->videoWithFramesToAnalyseLineEdit->setText (strings.at (0));
	}
}

void DialogRunParameters::on_selectWorkingFolderPushButton_clicked()
{
	this->file_dialog->setViewMode (QFileDialog::List);
	this->file_dialog->setOption (QFileDialog::ShowDirsOnly, true);
	this->file_dialog->setFileMode (QFileDialog::Directory);
	int return_code = this->file_dialog->exec ();
	if (return_code == QDialog::Accepted) {
		QStringList strings = this->file_dialog->selectedFiles ();
		this->ui->workingFolderLineEdit->setText (strings.at (0));
	}
}

void DialogRunParameters::on_extractFramesPushButton_clicked()
{
	string folder = this->get_folder ();
	string video_path = this->ui->videoWithFramesToAnalyseLineEdit->text ().toStdString ();
	if ((access (folder.c_str (), F_OK) == 0) && (access (video_path.c_str (), F_OK | R_OK) == 0)) {
		split_video (video_path, folder, this->ui->framesPerSecondSpinBox->value (), "frames-%4d." + this->get_frame_file_type ());
	}
}

void DialogRunParameters::on_extractBackgroundPushButton_clicked()
{
	string folder = this->get_folder ();
	string video_path = this->ui->videoWithBackgroundImageLineEdit->text ().toStdString ();
	if ((access (folder.c_str (), F_OK) == 0) && (access (video_path.c_str (), F_OK | R_OK) == 0)) {
		split_video (video_path, folder, 1, "background." + this->get_frame_file_type ());
	}
}

std::string DialogRunParameters::get_folder () const
{
	QString s = this->ui->workingFolderLineEdit->text ();
	if (s.isEmpty ())
		return "./";
	else
		return s.toStdString ();
}

std::string DialogRunParameters::get_frame_file_type () const
{
	if (this->ui->PNGFrameFileTypeRadioButton->isChecked ())
		return "png";
	else if (this->ui->JPEGFrameFileTypeRadioButton->isChecked ())
		return "jpg";
	else
		return "";
}

int DialogRunParameters::get_number_ROIs () const
{
	return this->ui->numberROIsSpinBox->value ();
}

void split_video (const string &video_path, const string &location, int frames_per_second, const string &frame_template)
{
	pid_t child;
	switch (child = fork ()) {
	case -1:
		fprintf (stderr, "Could not create child process to split video!\n");
		exit (EXIT_FAILURE);
	case 0:
		execl (
		         "/usr/bin/ffmpeg",
		         "/usr/bin/ffmpeg",
		         "-i", video_path.c_str (),
		         "-r", to_string (frames_per_second).c_str (),
		         "-loglevel", "error",
		         "-f", "image2", (location + "/" + frame_template).c_str (),
		         NULL
		         );
	default:
		waitpid (child, NULL, 0);
	}
}
