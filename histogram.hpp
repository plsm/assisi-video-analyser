#ifndef __HISTOGRAM__
#define __HISTOGRAM__

#include <stdio.h>

#include <QVector>

class Histogram:
        public QVector<double>
{
public:
	Histogram ();
	void read (FILE *file);
	void write (FILE *file);
};

#endif
