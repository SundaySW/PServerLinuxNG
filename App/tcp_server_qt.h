#pragma once

#include <qtcpserver.h>
#include "tcp_assoc_qt.h"
#include <memory>
#include <map>

class TcpServer : public QObject
{
	Q_OBJECT
public:
	 TcpServer(QObject* parent/*, ILog* log*/);
	~TcpServer();

	void Broadcast(const std::vector<char>& data, const QTcpSocket* skipMe = nullptr);
	void Close();
	bool Restart();
	bool Restart(const QString& address, int port);
	void SendTo(char id, const std::vector<char>& data);
signals:
	void SignalStarted(bool started);
	void SignalPacketRecv(QTcpSocket* sock, std::vector<char>& packet);

protected:
	void OnNewConnection();
	QTcpServer* Server;
	std::map<QTcpSocket*, std::shared_ptr<TcpAssoc>> Assocs;
	//ILog* Log;
};