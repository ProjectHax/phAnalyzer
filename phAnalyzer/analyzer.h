#pragma once

#ifndef ANALYZER_H
#define ANALYZER_H

#include <QtGui/QWidget>
#include "ui_analyzer.h"

class phAnalyzer : public QWidget
{
	Q_OBJECT

private:

	//User interface
	Ui::phAnalyzerClass ui;

public:

	//Constructor
	phAnalyzer(QWidget *parent = 0, Qt::WFlags flags = 0);

	//Destructor
	~phAnalyzer();
};

#endif