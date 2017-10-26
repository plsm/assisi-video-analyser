#include "animate.hpp"

Animate::Animate (const RunParameters &parameters, Ui_MainWindow *ui):
	QObject (),
	parameters (parameters),
	is_playing (false),
	timer (new QTimer (this)),
	ui (ui)
{
	QObject::connect (timer, SIGNAL (timeout ()), this, SLOT (tick ()));
}

Animate::~Animate ()
{
	delete this->timer;
}

void Animate::tick ()
{
	unsigned int current_frame = this->ui->currentFrameSpinBox->value ();
	if (current_frame < this->parameters.number_frames) {
		this->ui->currentFrameSpinBox->setValue (current_frame + 1);
	}
	else {
		this->ui->currentFrameSpinBox->setValue (1);
		this->is_playing = false;
		this->timer->stop ();
		this->ui->playStopButton->setText ("Play");
	}
}

void Animate::play_stop ()
{
	this->is_playing = !this->is_playing;
	if (this->is_playing) {
		this->timer->start (std::max (1, (int) (1000 / this->ui->framesPerSecondSpinBox->value ())));
		this->ui->playStopButton->setText ("Stop");
	}
	else {
		this->timer->stop ();
		this->ui->playStopButton->setText ("Play");
	}
}

void Animate::update_playback_speed (double frames_per_second)
{
 	if (this->is_playing) {
 		this->timer->start (std::max (1, (int) (1000 / frames_per_second)));
 	}
}
