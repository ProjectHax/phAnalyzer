#include "analyzer.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	phAnalyzer w;
	w.show();
	return a.exec();
}