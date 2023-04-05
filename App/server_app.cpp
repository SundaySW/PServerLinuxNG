#include "server_app.h"
#include "server_conf.h"
#include "fixed_packet.h"
#include "protos_format.h"
#include "QCoreApplication"
#include <qcoreapplication.h>
#include <qserialportinfo.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qtimer.h>

namespace
{
	void SendToUI(QTcpSocket* sock, const QJsonObject& ans)
	{//������� ����� � UI-����� �������
		QJsonDocument doc(ans);
		auto json = doc.toBinaryData();
		auto packet = FixedPacket()
			.AppendData(std::vector<char>(json.begin(), json.end()))
			.ToBinary('!');
		sock->write(packet.data(), packet.size());
	}

	void LoadProductNameAndVersion(QString& prodName, QString& version)
	{
		auto file = QCoreApplication::applicationFilePath();
		char data[4096];
//		if (GetFileVersionInfo(file.toStdWString().data(), NULL, sizeof(data), data))
//		{
//			struct LANGANDCODEPAGE {
//				WORD wLanguage;
//				WORD wCodePage;
//			} *lpTranslate;
//
//			UINT cbTranslate;
//			if (!VerQueryValue(data, L"\\VarFileInfo\\Translation",
//				(LPVOID*)&lpTranslate, &cbTranslate))
//				return;
//
//			auto makeKey = [lpTranslate](const wchar_t* paramName, wchar_t* key)->bool
//			{
//				auto hr = StringCchPrintf((STRSAFE_LPWSTR)key, 64,
//					QString("\\StringFileInfo\\%04x%04x\\%1").arg(paramName).toStdWString().data(),
//					lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
//
//				return SUCCEEDED(hr);
//			};
//
//			wchar_t key[64];
//			for (int i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
//			{
//				WCHAR* buffer;
//				UINT nbytes;
//
//				if (makeKey(L"ProductName", key)) {
//					if (VerQueryValue(data, key, (LPVOID*)(&buffer), &nbytes))
//						prodName = QString::fromWCharArray(buffer);
//				}
//
//				if (makeKey(L"ProductVersion", key)) {
//					if (VerQueryValue(data, key, (LPVOID*)(&buffer), &nbytes))
//						version = QString::fromWCharArray(buffer);
//				}
//
//				break;
//			}
//		}
	}

}//namespace


ServerApp::ServerApp()
	: Server(new TcpServer(this))
{
	connect(Server, &TcpServer::SignalPacketRecv,
		[this](QTcpSocket* sock, std::vector<char>& packet)
	{
		if (packet[0] == '!')
		{//fixed packet from UI client
			ProcessControlPacket(sock, std::move(packet));
		}
		else
		{//fixed packet from (not UI) client
			FixedPacketView pv(packet.data(), packet.size());
			if (Port.IsStarted())
				Port.Write({ pv.Data(), (int)pv.Dlc() });

			auto fixedPacket = FixedPacket().AppendData(packet).ToBinary('#');
			Server->Broadcast(fixedPacket, sock);
		}
	});

	Port.OnPacket = [this](const char* dir, const Protos::Packet& packet, const QString& error)
	{
		auto binMsg = packet.ToBinary();

		auto fixedPacket = FixedPacket().AppendData(binMsg).ToBinary('#');
		Server->Broadcast(fixedPacket);

		QJsonObject m;
		m["Id"]  = dir;
		m["Res"] = error.isEmpty();

		if (!error.isEmpty())
			m["Err"] = error;

		QJsonArray arr;
		for (auto& byte : binMsg)
			arr.push_back((unsigned char)byte);
		m["Packet"] = arr;

		Log(m);

		if (LogFile.isOpen())
			LogFile.Write(packet, dir);
	};

	Port.OnReadError = [this](const QString& error) 
	{
		QJsonObject m;
		m["Id"]  = "rx";
		m["Res"] = false;
		m["Err"] = error;
		Log(m);
	};
}

bool ServerApp::event(QEvent* ev)
{
	if (ev->type() == Protos::Transporter::PORT_EVENT)
	{
		Port.OnEvents();
		ev->accept();
		return true;
	}
	return QObject::event(ev);
}

/*!
	Control packet from UI client(plugin).
*/
void ServerApp::ProcessControlPacket(QTcpSocket* sock, std::vector<char>&& packet)
{
	FixedPacketView pv(packet.data(), packet.size());

	QByteArray arr;
	auto dlc = pv.Dlc();
	arr.reserve(dlc);
	arr.append(pv.Data(), dlc);

	auto doc = QJsonDocument::fromBinaryData(arr);
	if (doc.isNull())
		return;

	auto msg = doc.object();
	if (msg.isEmpty())
		return;

	QJsonObject ans;
	auto id = msg["Id"].toString();
	
	if (id == "op")
	{//open port
		ans["Res"] = true;
		if (!Port.IsStarted())
		{
			QString portError;
			if (!Port.Restart(portError))
			{
				ans["Err"] = portError;
				ans["Res"] = false;
			}
		}
	}
	else if (id == "cp")
	{//close port
		Port.Stop();
		ans["Res"] = true;
	}
	else if (id == "tx")
	{
		if (Port.IsStarted())
		{
			Protos::Msg pmsg;
			pmsg.ID3.Bit.Pri = msg["Pri"].toInt(0);
			pmsg.Src = msg["Src"].toInt(0);
			pmsg.Dst = msg["Dst"].toInt(0);
			pmsg.ID0.Bit.Type = (unsigned char)msg["Type"].toInt(0);
			
			auto dataArr = msg["Data"].toArray();
			pmsg.Data.reserve(dataArr.size());
			for (auto d : dataArr)
				pmsg.Data.push_back(d.toInt());

			Port.Write(pmsg);
		}
		else
		{
			ans["Res"] = false;
			ans["Err"] = QString("Port closed");
		}
	}
	else if (id == "gcfg")
	{//get config
		QFile file;
		file.setFileName(QCoreApplication::applicationDirPath() + "/protos_server.conf");
		if (!file.open(QFile::ReadOnly))
			return;

		auto doc = QJsonDocument::fromJson(file.readAll());
		ans["Res"] = doc.object();
	}
	else if (id == "scfg")
	{
		QFile file;
		QString ffn = QCoreApplication::applicationDirPath() + "/protos_server.conf";
		file.setFileName(ffn);
		if (file.open(QFile::WriteOnly))
		{
			file.write(QJsonDocument(msg["Res"].toObject()).toJson(QJsonDocument::Indented));
			file.flush();
			file.close();
			ans["Res"] = true;
			QString error;
			Conf::Read(ffn.toStdString(), error);
		}
		else
		{
			ans["Res"] = false;
			ans["Err"] = file.errorString();
		}
		ans["JournalSize"] = Conf::JournalSize;
	}
	else if (id == "scanp")
	{
		auto portType = msg["PortType"].toString();

		QJsonArray arr;
		if (portType.compare("PICA", Qt::CaseInsensitive) == 0)
		{
			auto ports = QSerialPortInfo::availablePorts();
			for (auto& port : ports)
				arr.push_back(port.portName());
		}
		else
			return;

		ans["Res"] = arr;
		ans["PortType"] = portType;
	}
	else if (id == "state")
	{
		ans["PortState"] = Port.IsStarted();
	}
	else if (id == "ver")
	{
		QString name, version;
		LoadProductNameAndVersion(name, version);
		ans["Res"] = version;
	}
	else if (id == "logm")
	{
		const bool enable = msg["Enable"].toBool();
		
		bool ok = true;
		if (LogFile.isOpen())
		{
			LogFile.flush();
			LogFile.close();
			Conf::MsgLogFn.clear();
		}
		
		if (enable)
		{
			auto fn = msg["Fn"].toString();
			if (LogFile.Open(fn))
				Conf::MsgLogFn = fn;
		}

		QString e;
		Conf::Save(e);

		ans["Enabled"] = LogFile.isOpen();
	}
	else if (id == "hi")
	{
		if (LogFile.isOpen())
		{
			ans["Id"] = "logm";
			ans["Enabled"] = true;
			ans["Fn"] = Conf::MsgLogFn;
			SendToUI(sock, ans);
		}
		return;
	}

	if (!ans.isEmpty())
	{
		ans["Id"] = id;
		SendToUI(sock, ans);
	}
}

void ServerApp::Log(const QJsonObject& msg)
{
	QJsonDocument doc(msg);
	auto json = doc.toBinaryData();
	auto packet = FixedPacket()
		.AppendData(std::vector<char>(json.begin(), json.end()))
		.ToBinary('!');
	Server->SendTo(0xFF, packet);
}

void ServerApp::Start()
{
	if (!Conf::MsgLogFn.isEmpty())
		LogFile.Open(Conf::MsgLogFn);

	QString portError;
	Port.Restart(portError);

	Server->Restart();
	QCoreApplication::instance()->exec();
}

void ServerApp::Stop()
{
	Server->Close();
	Port.Stop();
	QCoreApplication::instance()->exit();
}