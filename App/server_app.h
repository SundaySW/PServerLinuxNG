#pragma once

#include <qobject.h>
#include "msg_log_file.h"
#include "protos_transporter.h"
#include "tcp_server_qt.h"

class ServerApp : public QObject
{
	Q_OBJECT
public:
	ServerApp();
	virtual ~ServerApp() {};

	void Log(const QJsonObject& msg);
	void Start();
	void Stop();

private:
	bool event(QEvent* ev) override;
	void ProcessControlPacket(QTcpSocket* sock, std::vector<char>&& packet);

	TcpServer* Server;			///< TCP server
	Protos::Transporter Port;	///< Sender/Receiver msgs

	MsgLogFile LogFile;
};