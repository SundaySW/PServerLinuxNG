#include "tcp_assoc_qt.h"

namespace
{
	bool EnableKeepAlive(QTcpSocket* socket, unsigned long timeout_ms,
		unsigned long repeatInterval_ms)
	{
		socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
		return true;
	}

}//namespace

TcpAssoc::TcpAssoc(QTcpSocket* sock)
	: Sock(sock)
	, PacketAsm(PacketAssembler::PROTOCOL_FIXED)
{
	EnableKeepAlive(Sock, 3600000/*hour*/, 10000/*10sec*/);
}

TcpAssoc::~TcpAssoc()
{
	//Socket ��������� �� heap ��������;
}