#pragma once

#include "packet_asm.h"
#include <qtcpsocket.h>

class TcpAssoc
{
public:
	 TcpAssoc(QTcpSocket* sock);
	~TcpAssoc();

	QTcpSocket* Sock;
	PacketAssembler PacketAsm;
};