#include "tcp_server_qt.h"
#include "server_conf.h"
#include "packet_utils.h"
#include <chrono>

namespace
{
	const std::chrono::milliseconds AUTH_TIMEOUT(15000);
	const int ASSOC_MAXIMUM	= 32;
}//namespace

TcpServer::TcpServer(QObject* parent/*, ILog* log*/)
	: Server(new QTcpServer(parent))
	//, Log(log)
{
	QObject::connect(Server, &QTcpServer::newConnection, this, &TcpServer::OnNewConnection);
}

TcpServer::~TcpServer()
{
	delete Server;
}

void TcpServer::OnNewConnection()
{
	// Сокет создается в heap, QTcpServer - parent;
	QTcpSocket* socket = Server->nextPendingConnection();
	if (!socket) return;
	
	if (Assocs.size() == ASSOC_MAXIMUM)
	{
		socket->close();
		socket->deleteLater();
		return;
	}

	QString address = socket->peerAddress().toString();
	int port = socket->peerPort();

	//Log->AddMsg(QStringLiteral("Подключился клиент %1:%2").arg(address).arg(port));

	QObject::connect(socket, &QTcpSocket::disconnected,
		[this, socket]() mutable
	{
		/*Log->AddMsg(QStringLiteral("Разорвано соединение с %1:%2")
			.arg(socket->peerAddress().toString()
			.arg(socket->peerPort())));*/

		Assocs.erase(socket);
		socket->deleteLater();
	});

	QObject::connect(socket, &QTcpSocket::readyRead, [this, socket]()
	{
		auto assoc = Assocs[socket];
		if (!assoc)
			return;

		auto data = socket->readAll();
		
		// retransmit data to other clients:
		auto it = Assocs.begin();
		while (it != Assocs.end())
		{
			auto sock = it->first;
			if (socket != sock)
				sock->write(data.data(), data.size());
			// write may cause disconnect, what can invalidate iterator.
			it = Assocs.upper_bound(sock);
		}
		
		// parse packet:
		for (auto byte : data)
		{
			assoc->PacketAsm.Assemble(byte);
			if (assoc->PacketAsm.IsComplete())
			{
				SignalPacketRecv(assoc->Sock, assoc->PacketAsm.Packet);
				assoc->PacketAsm.Reset();
			}
		}
	});
		
	std::shared_ptr<TcpAssoc> assoc(new TcpAssoc(socket));
	Assocs[socket] = assoc;
}

void TcpServer::Broadcast(const std::vector<char>& data, const QTcpSocket* skipMe)
{
	auto it = Assocs.begin();
	while (it != Assocs.end())
	{
		auto sock = it->first;
		if (!skipMe || skipMe != sock)
			sock->write(data.data(), data.size());
		// write может вызвать disconnct, который invalidate итератор.
		it = Assocs.upper_bound(sock);
	}
}

void TcpServer::Close()
{
	//Log->AddMsg(QStringLiteral("Остановить TCP сервер."));
	while (Assocs.size())
	{
		// Удаление из Assocs идет в QTcpSocket::disconnected
		QTcpSocket* socket = Assocs.begin()->first;
		Assocs.erase(Assocs.begin());
		socket->close();
	}
	// SocketEngine(внутри QTcpServer) будет удален на следующем цикле EventLoop
	// Но можно использовать тот же Server для нового listen
	Server->close();
	// Log->AddMsg(QStringLiteral("Сервер остановлен"));
	SignalStarted(false);
}

bool TcpServer::Restart()
{
	return Restart(Conf::ServerIP, Conf::ServerPort);
}

bool TcpServer::Restart(const QString& address, int port)
{
	Close();
	bool result;
    result = Server->listen(QHostAddress(address), port);
	SignalStarted(result);
	return result;
}

void TcpServer::SendTo(char id, const std::vector<char>& data)
{
	auto it = Assocs.begin();
	while (it != Assocs.end())
	{
		auto sock = it->first;
		sock->write(data.data(), data.size());
        // write may cause disconnect, what can invalidate iterator.
		it = Assocs.upper_bound(sock);
	}
}