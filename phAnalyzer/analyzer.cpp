#include "analyzer.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

//Constructor
phAnalyzer::phAnalyzer(QWidget *parent, Qt::WFlags flags) : QWidget(parent, flags), inj(0)
{
	//Sets up the UI
	ui.setupUi(this);

	//Connect file menu actions
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(Save()));
	connect(ui.actionInject, SIGNAL(triggered()), this, SLOT(Inject()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	//New socket
	socket = new QTcpSocket(this);

	//Create the injection UI
	inj = new injection(0, socket);

	//Setup the connection slots
	connect(socket, SIGNAL(connected()), this, SLOT(Connected()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(SocketState(QAbstractSocket::SocketState)));
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

	//Connect
	socket->connectToHost(HOST, PORT);
}

//Destructor
phAnalyzer::~phAnalyzer()
{
	//Cleanup
	socket->close();
	delete inj;
}

//Processes close events
void phAnalyzer::closeEvent(QCloseEvent* event)
{
	//Packet injector window might be active so close it, otherwise it will get stuck while attempting to exit
	inj->close();
}

//Adds an opcode to the ignore list when the enter key is pressed
void phAnalyzer::AddIgnoreOpcode()
{
	QString text = ui.txtIgnore->text().toUpper();

	//Length check
	if(text.length() == 4)
	{
		//Clear text box
		ui.txtIgnore->setText("");

		//See if the opcode is already in the list
		for(int x = 0; x < ui.lstIgnore->count(); ++x)
		{
			if(ui.lstIgnore->item(x)->text() == text)
				return;
		}

		//Add the opcode to the ignore list
		ui.lstIgnore->addItem(text);
	}
	else
	{
		QMessageBox::warning(this, "Error", "Opcode length must be 4.");
	}
}

//Adds an opcode to the listen list when the enter key is pressed
void phAnalyzer::AddListenOpcode()
{
	QString text = ui.txtListen->text().toUpper();

	//Length check
	if(text.length() == 4)
	{
		//Clear text box
		ui.txtListen->setText("");

		//See if the opcode is already in the list
		for(int x = 0; x < ui.lstListen->count(); ++x)
		{
			if(ui.lstListen->item(x)->text() == text)
				return;
		}

		//Add the opcode to the listen list
		ui.lstListen->addItem(text);
	}
	else
	{
		//Invalid opcode length
		QMessageBox::warning(this, "Error", "Opcode length must be 4.");
	}
}

//Socket connected
void phAnalyzer::Connected()
{
	//Change the window title
	setWindowTitle("phAnalyzer - ProjectHax.com - Connected");

	//Start reading data
	socket->waitForReadyRead(2);
}

//Socket state changed
void phAnalyzer::SocketState(QAbstractSocket::SocketState state)
{
	if(state == QAbstractSocket::UnconnectedState)
	{
		//Change the window title
		setWindowTitle("phAnalyzer - ProjectHax.com - Disconnected");

		//Attempt to reconnect
		socket->connectToHost(HOST, PORT);
	}
}

//Read data
void phAnalyzer::readyRead()
{
	data.resize(socket->bytesAvailable());
	uint64_t count = socket->read(&data[0], socket->bytesAvailable());

	//Write the received data to the end of the stream
	pending_stream.Write<char>(&data[0], count);

	//Total size of stream
	int32_t total_bytes = pending_stream.GetStreamSize();

	//Make sure there are enough bytes for the packet size to be read
	while(total_bytes > 2)
	{
		//Peek the packet size
		uint16_t required_size = pending_stream.Read<uint16_t>(true) + 6;

		//See if there are enough bytes for this packet
		if(required_size <= total_bytes)
		{
			StreamUtility r(pending_stream);

			//Remove this packet from the stream
			pending_stream.Delete(0, required_size);
			pending_stream.SeekRead(0, Seek_Set);
			total_bytes -= required_size;

			//Paused?
			if(ui.chkPause->isChecked())
				continue;

			//Extract packet header information
			uint16_t size = r.Read<uint16_t>();
			uint16_t opcode = r.Read<uint16_t>();
			uint8_t direction = r.Read<uint8_t>();
			uint8_t encrypted = r.Read<uint8_t>();

			//Remove the header
			r.Delete(0, 6);
			r.SeekRead(0, Seek_Set);

			//Player ID
			if(opcode == 0x3020)
			{
				//Have to read it like this otherwise it won't look right
				uint8_t PlayerID0 = r.Read<uint8_t>();
				uint8_t PlayerID1 = r.Read<uint8_t>();
				uint8_t PlayerID2 = r.Read<uint8_t>();
				uint8_t PlayerID3 = r.Read<uint8_t>();

				char temp[16] = {0};
				sprintf_s(temp, "%.2X %.2X %.2X %.2X", PlayerID0, PlayerID1, PlayerID2, PlayerID3);

				ui.PlayerID->setText(temp);
				r.SeekRead(0, Seek_Set);
			}
			//Object selected
			else if(opcode == 0x7045)
			{
				//Have to read it like this otherwise it won't look right
				uint8_t Object0 = r.Read<uint8_t>();
				uint8_t Object1 = r.Read<uint8_t>();
				uint8_t Object2 = r.Read<uint8_t>();
				uint8_t Object3 = r.Read<uint8_t>();

				char temp[16] = {0};
				sprintf_s(temp, "%.2X %.2X %.2X %.2X", Object0, Object1, Object2, Object3);

				ui.SelectedObject->setText(temp);
				r.SeekRead(0, Seek_Set);
			}

			//Check the ignore list
			bool found = false;
			for(int x = 0; x < ui.lstIgnore->count(); ++x)
			{
				bool OK;
				if(ui.lstIgnore->item(x)->text().toUInt(&OK, 16) == opcode)
				{
					found = true;
					break;
				}
			}
			if(found) continue;

			//Check the listen list
			if(ui.lstListen->count())
			{
				found = false;
				for(int x = 0; x < ui.lstListen->count(); ++x)
				{
					bool OK;
					if(ui.lstListen->item(x)->text().toUInt(&OK, 16) == opcode)
					{
						found = true;
						break;
					}
				}

				if(!found)
					continue;
			}

			//Parse the packet for reading
			std::stringstream ss;
			ss << "[" << (direction == 1 ? "C->S" : "S->C") << "] ";
			ss << "[" << size << "] ";
			ss << "[0x" << std::hex << std::setfill('0') << std::setw(4) << opcode << "] " << std::dec;
			ss << (encrypted ? "[E] " : "") << "\n";
			ss << DumpToString(r);

			bool append = false;
			if(direction == 0 && ui.chkServer->isChecked())
			{
				if(ui.chkEncrypted->isChecked() && encrypted)
					append = true;
				if(!ui.chkEncrypted->isChecked())
					append = true;
			}
			else if(direction == 1 && ui.chkClient->isChecked())
			{
				if(ui.chkEncrypted->isChecked() && encrypted)
					append = true;
				if(!ui.chkEncrypted->isChecked())
					append = true;
			}

			//Append the packet to the list
			if(append)
				ui.lstPackets->appendPlainText(ss.str().c_str());
		}
		else
		{
			//Not enough bytes received for this packet
			break;
		}
	}
}

//Saves packets
void phAnalyzer::Save()
{
	//Ask the user where to save the packets
	QString path = QFileDialog::getSaveFileName(this, "Save Packet", "", "Text Documents (*.txt)");

	//Valid path check
	if(!path.isEmpty())
	{
		//Open the file
		QFile file(path);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::critical(this, "Error", "Failed to open file: " + path);
		}
		else
		{
			//Retrieve the packets from the user interface
			QString packets = ui.lstPackets->toPlainText();

			//Write the packets to the text document
			QTextStream out(&file);
			out << packets;

			//Close the file
			file.close();
		}
	}
}
	
//Clears the packet window
void phAnalyzer::Clear()
{
	//Clear packet list
	ui.lstPackets->clear();
}

//Displays the packet injection window
void phAnalyzer::Inject()
{
	//Show the window
	if(inj) inj->show();
}