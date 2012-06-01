#pragma once

#ifndef ANALYZER_H
#define ANALYZER_H

#include <QtGui/QWidget>
#include "ui_analyzer.h"
#include "injection.h"
#include "Stream/stream_utility.h"

#include <QtNetwork>
#include <QAbstractSocket>

#include <vector>
#include <stdint.h>
#include <sstream>
#include <iomanip>
#include <string>

#define HOST "localhost"		//Hostname/IP
#define PORT 22580				//Port of phConnector or nuConnector that is used for the analyzer

class phAnalyzer : public QWidget
{
	Q_OBJECT

private:

	//User interface
	Ui::phAnalyzerClass ui;

	//TCP socket
	QTcpSocket* socket;

	//Received data
	std::vector<char> data;

	//Packet stuff
	StreamUtility pending_stream;

	//Packet injector UI
	injection* inj;

private slots:

	//Adds an opcode to the ignore list when the enter key is pressed
	void AddIgnoreOpcode();

	//Adds an opcode to the listen list when the enter key is pressed
	void AddListenOpcode();

	//Socket connected
	void Connected();

	//Socket state changed
	void SocketState(QAbstractSocket::SocketState state);

	//Read data
	void readyRead();

	//Saves packets
	void Save();
	
	//Clears the packet window
	void Clear();

	//Displays the packet injection window
	void Inject();

protected:

	//Processes close events
	void closeEvent(QCloseEvent* event);

public:

	//Constructor
	phAnalyzer(QWidget *parent = 0, Qt::WFlags flags = 0);

	//Destructor
	~phAnalyzer();
};

#endif