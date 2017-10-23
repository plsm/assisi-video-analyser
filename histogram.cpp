#include "histogram.hpp"
#include "image.hpp"

Histogram::Histogram ():
   QVector<double> (NUMBER_COLOUR_LEVELS)
{
}

void Histogram::read (FILE *file)
{
	int value;
	fscanf (file, "%d", &value);
	(*this) [0] = value;
	for (unsigned int i = 1; i < NUMBER_COLOUR_LEVELS; i++) {
		fscanf (file, ",%d", &value);
		(*this) [i] = value;
	}
}

void Histogram::write (FILE *file)
{
	fprintf (file, "%d", (int) (*this) [0]);
	for (unsigned int i = 1; i < NUMBER_COLOUR_LEVELS; i++)
		fprintf (file, ",%d", (int) (*this) [i]);
}

int Histogram::most_common_colour () const
{
	int result = 0;
	double best = (*this) [0];
	for (unsigned int colour = 1; colour < NUMBER_COLOUR_LEVELS; colour++)
		if ((*this) [colour] > best) {
			best = (*this) [colour];
			result = colour;
		}
	return result;
}
