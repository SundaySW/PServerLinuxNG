
#include "USBCANM2.h"

std::optional<int> USBCANM2::GetLastError() const
{
	//if (Status.Success()) return {};
	//char res[] = { (char)Status.Cmd, (char)Status.CAN_Status, 0, 0 };
	//return *(int*)res;
	return {};
}

bool USBCANM2::Close()
{
	//Status.Reset();
    return closeConnection();
}

inline bool USBCANM2::closeConnection(){
    if (IsOpen())
    {
        COM->write("C\r");
        COM->flush();
        COM->close();
    }
    return true;
}

QString USBCANM2::FormatError(int error) const
{
	QString res;
	/*const char* err = (const char*)(&error);

	switch ((CMD)err[0])
	{
	case CMD::READ:  res.append("Read port error."); break;
	case CMD::WRITE: res.append("Write port error."); break;
	case CMD::OPEN:  res.append("Open port error."); break;
	case CMD::CLOSE: res.append("Close port error."); break;
	};

	switch (err[1])
	{
	case CAN_ERR_ERR: res.append("Error communicating with COM port.");  break;
	case CAN_ERR_OPEN_CHANNEL: res.append("Error in opening channel."); break;
	case CAN_ERR_PARAMETER: res.append("Error in parameter settings.");  break;
	case CAN_ERR_NOT_OPEN: res.append("CAN channel is not open.");       break;
	};

	QString details;
	switch (err[2] & 0x7)
	{
	case CAN_STATUS_LEC_STUFF_ERR: details.append("Stuff error."); break;
	case CAN_STATUS_LEC_FORM_ERR: details.append("Form error."); break;
	case CAN_STATUS_LEC_ACK_ERR: details.append("ACK error."); break;
	case CAN_STATUS_LEC_BIT1_ERR: details.append("Bit1 error."); break;
	case CAN_STATUS_LEC_BIT0_ERR: details.append("Bit0 error."); break;
	case CAN_STATUS_LEC_CRC_ERR: details.append("CRC error."); break;
	};

	if (err[3] & CAN_STATUS_EPASS)
		details.append("Error passive state.");

	if (err[2] & CAN_STATUS_EWARN)
		details.append("Error counter warning.");

	if (err[2] & CAN_STATUS_BOFF)
		details.append("Bus off state.");

	if (!details.isEmpty())
		res.append("CAN status: ").append(details);*/

	return res;
}

void USBCANM2::HandleReadyRead()
{
	while (COM->bytesAvailable())
	{
		char prefix;
		COM->read(&prefix, 1);

		if (prefix == 'T' || prefix == 't')
		{
			char buffer[26] = { 0 };
			int n = 0;

			if (prefix == 't') {
				buffer[0] = '0';
				n++;
			}
			
			while (COM->read(&buffer[n], 1) && n < sizeof(buffer))
			{
				if (buffer[n] == '\r') break;
				n++;
			}

			if (n < sizeof(buffer) && buffer[n] == '\r')
			{
				Protos::Packet packet;
				if (ParsePacket(prefix, buffer, n, packet))
					RxQueue.push_back(packet);
				else
					;//parse error
			}
		}
		else if (prefix == '\r')
		{//ok ans
			;
		}
		else if (prefix == 0x07)
		{//error ans
			;
		}
		else
		{
			;
		}
	}
}

bool USBCANM2::IsOpen() const
{
	return COM->isOpen();
}

void USBCANM2::OnPortErrorOccured(QSerialPort::SerialPortError e)
{

}

bool USBCANM2::Open(const StringMap& params)
{
	//Status.Reset();

	struct Param {
		QString Name;
		int Value;
	};

	QString id;
	Param parity       { "COMParity",   QSerialPort::Parity::NoParity },
		dataBits       { "COMDataBits", QSerialPort::DataBits::Data8 },
		stopBits       { "COMStopBits", QSerialPort::StopBits::OneStop },
		baudRate       { "COMBaudRate", 3000000 },
		 canBitrate_bps{ "CANBitrate",	500000 };
	
	bool loopbackMode = false;
	bool canTimestamp = false;

	try {
		
		auto it = params.find("COM");
		if (it == params.end())
			throw QString("COM");
		id = it->second;

		it = params.find("LoopbackMode");
		if (it != params.end())
			loopbackMode = (it->second.compare("true", Qt::CaseInsensitive) == 0);

		it = params.find("CANTimestamp");
		if (it != params.end())
			canTimestamp = (it->second.compare("true", Qt::CaseInsensitive) == 0);

		it = params.find("COMSettings");
		if (it != params.end())
		{
			QRegExp re("^(\\d+),(\\d),(N|P),(\\d)$");
			if (re.indexIn(it->second, 0) != -1)
			{
				baudRate.Value = re.cap(1).toInt();
				dataBits.Value = re.cap(2).toInt();
				parity.Value = (re.cap(3).compare("N") == 0) ? QSerialPort::Parity::NoParity 
															 : QSerialPort::Parity::EvenParity;
				stopBits.Value = re.cap(4).toInt();
			}
		}
		/*bool ok;
		for (auto param : { &parity , &dataBits, &stopBits, &baudRate })
		{
			auto it = params.find(param->Name);
			if (it == params.end()) continue;
			int value = params.at(param->Name).toInt(&ok);
			if (ok) param->Value = value; else throw param->Name;
		}*/
	}
	catch (QString paramName) {
		//Status = { CMD2::OPEN, CAN_ERR_PARAMETER, QStringLiteral("Не задан или задан некорректно параметр-\"%1\" COM порта.")
		//	.arg(paramName) };
		return false;
	}

	COM->setPortName(id);
	if (!COM->open(QIODevice::ReadWrite))
	{
		//Status = { CMD2::OPEN, CAN_ERR_OPEN_CHANNEL, COM->errorString() };
		return false;
	}

	try {
		if (!COM->setParity((QSerialPort::Parity)parity.Value))
			throw ("Не удалось задать Parity для COM порта.");

		if (!COM->setBaudRate(baudRate.Value))
			throw ("Не удалось задать Parity для COM порта.");

		if (!COM->setDataBits((QSerialPort::DataBits)dataBits.Value))
			throw ("Не удалось задать DataBits для COM порта.");

		if (!COM->setStopBits((QSerialPort::StopBits)stopBits.Value))
			throw ("Не удалось задать StopBits для COM порта.");

		if (!COM->setFlowControl(QSerialPort::FlowControl::NoFlowControl))
			throw ("Не удалось задать \"no flow control\" для COM порта.");
		
		if (!COM->setRequestToSend(true))
			throw ("Не удалось задать RTS=true для COM порта.");

		QString initCmds = GetCANBitrateCmd(canBitrate_bps.Value / 1000);
		initCmds.append(canTimestamp ? "Z1\r" : "Z0\r");

		auto it = params.find("CANAcceptanceMask");
		if (it != params.end())
			initCmds.append(QString("m%1\r").arg(it->second));

		it = params.find("CANAcceptanceFilter");
		if (it != params.end())
			initCmds.append(QString("M%1\r").arg(it->second));

		initCmds.append(loopbackMode ? "Y\r" : "O\r");

		COM->write(initCmds.toLatin1());
		return true;
	}
	catch (QString error) {
		//Status = { CMD2::OPEN, CAN_ERR_PARAMETER, error };
		COM->close();
		return false;
	}
}

int USBCANM2::Read(Protos::Packet* packets, int count)
{
	int n = std::min(count, (int)RxQueue.size());
	for (int i = 0; i != n; i++) {
		packets[i] = RxQueue.front();
		RxQueue.pop_front();
	}
	return n;
}

bool USBCANM2::Write(const Protos::Packet& packet)
{
	char id[9];
	char invHeader[4] = { static_cast<char>(packet.ID3.Byte), static_cast<char>(packet.Dst), static_cast<char>(packet.Src), static_cast<char>(packet.ID0.Byte) };
	Pack2x((const char*)&invHeader, sizeof(invHeader), id);
	id[8] = 0;

	char data[17];
	int n = Pack2x((const char*)&packet.DataBuffer, 
		std::min((unsigned char)8, packet.Dlc), data);
	data[n] = 0;

	QString msg;
	msg.reserve(28);
	msg.append("T");
	msg.append(id);
	msg.append(QString::number(packet.Dlc));
	if (packet.Dlc)
		msg.append(data);
	msg.append('\r');

	COM->write(msg.toLatin1());
	COM->flush();
	return true;
}

bool USBCANM2::ParsePacket(char type, const char *buffer, int len, Packet &packet) {
    const int headerLen = 2 * (type == 'T' ? sizeof(Protos::Header) : 2);
    const int dlcLen    = 1;
    if (len < + headerLen + dlcLen) return false;

    char invHeader[4];
    Unpack2x(buffer, headerLen, (char*)invHeader, false);
    packet.ID3.Byte = invHeader[0];
    packet.Dst      = invHeader[1];
    if (type == 'T') {
        packet.Src      = invHeader[2];
        packet.ID0.Byte = invHeader[3];
    }
    else
    {//standard can packet no protos
        packet.Src      = 0;
        packet.ID0.Byte = 0;
    }
    packet.Dlc = buffer[headerLen] - '0';

    buffer += headerLen + dlcLen;
    len    -= (headerLen + dlcLen);
    int dataLen = 2 * packet.Dlc;
    if (len < dataLen)
        return false;

    Unpack2x(buffer, dataLen, (char*)packet.DataBuffer, false);
    buffer += dataLen;
    len    -= dataLen;

    short timestamp = 0;
    int timestampLen = 2 * sizeof(timestamp);
    if (len >= timestampLen)
        Unpack2x(buffer, timestampLen, (char*)(&timestamp), false);
    return true;
}

std::unique_ptr<Protos::Port> USBCANM2::CreateUSBCANM2()
{
	return std::unique_ptr<Protos::Port>(new USBCANM2());
}