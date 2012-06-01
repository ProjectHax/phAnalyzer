#include "injection.h"

#include <QMessageBox>

//Constructor
injection::injection(QWidget *parent, QTcpSocket* socket_) : QWidget(parent), socket(socket_)
{
	//Sets up the user interface
	ui.setupUi(this);
}

//Destructor
injection::~injection()
{
}

//Injects a packet into Silkroad
void injection::Inject()
{
	//Connected check
	if(!socket || socket->state() != QAbstractSocket::ConnectedState)
	{
		QMessageBox::critical(this, "Error", "The analyzer is not currently connected to anything.");
		return;
	}

	StreamUtility w;
	QString opcode = ui.txtOpcode->text();
	QString data = ui.txtData->text().replace(" ", "");

	//Opcode length check
	if(opcode.length() != 4)
	{
		QMessageBox::warning(this, "Error", "Packet opcode length must be 4.");
		return;
	}

	//Data length check
	if((data.length() % 2) != 0)
	{
		QMessageBox::warning(this, "Error", "Packet data must be a multiple of 2.");
		return;
	}

	//Convert the opcode
	bool OK;
	uint16_t temp = opcode.toUInt(&OK, 16);

	if(!OK)
	{
		QMessageBox::warning(this, "Error", "There was a problem converting the opcode to an unsigned short.");
		return;
	}

	//Packet size
	w.Write<uint16_t>(data.length() ? data.length() / 2 : 0);

	//Packet opcode
	w.Write<uint16_t>(temp);

	//Direction
	if(ui.radServer->isChecked())
	{
		w.Write<uint8_t>(ui.chkEncrypted->isChecked() ? 3 : 1);
	}
	else
	{
		w.Write<uint8_t>(ui.chkEncrypted->isChecked() ? 4 : 2);
	}

	w.Write<uint8_t>(0);

	//Convert the data to hex bytes
	for(int x = 0; x < data.length(); x += 2)
	{
		uint8_t byte = data.mid(x, 2).toUInt(&OK, 16);
		if(!OK)
		{
			QMessageBox::warning(this, "Error", QString("There was a problem converting a byte in your packet at index %0").arg(x));
			return;
		}

		w.Write<uint8_t>(byte);
	}

	//Inject
	socket->write((const char*)w.GetStreamPtr(), w.GetStreamSize());
}