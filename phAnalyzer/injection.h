#pragma once

#ifndef INJECTION_H
#define INJECTION_H

#include <QWidget>
#include "ui_injection.h"
#include <QtNetwork>

#include <stdint.h>
#include "Stream/stream_utility.h"

class injection : public QWidget
{
	Q_OBJECT

private:

	//User interface
	Ui::injection ui;

	//Socket
	QTcpSocket* socket;

private slots:

	//Injects a packet into Silkroad
	void Inject();

public:

	//Constructor
	injection(QWidget *parent = 0, QTcpSocket* socket_ = 0);

	//Destructor
	~injection();
};

#endif