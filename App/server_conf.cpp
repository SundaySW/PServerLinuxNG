#include "server_conf.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qcoreapplication.h>
#include <qfile.h>
#include <vector>
#include <QDir>

namespace Conf
{
	PortDesc Port;
	//PROTOCOL_TYPE Protocol = PROTOCOL_FIXED;
	QString ServerIP("127.0.0.1");
	unsigned short ServerPort = 3699;
	QList<CommandDesc> Commands;
	bool ScanPorts = true;
	bool McpRestart = true;
	int JournalSize = 1000;
	QString MsgLogFn;

	bool Read(const std::string fileName, QString& error)
	{
	    std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.is_open()) return false;

		intptr_t fileSize = file.tellg();
		if (!fileSize) return false;

		file.seekg(0, std::ios::beg);

		std::vector<char> data(fileSize);
		file.read(data.data(), fileSize);

		QJsonParseError e;
		auto conf = QJsonDocument::fromJson(QByteArray(data.data(), data.size()), &e);
		
		bool ok = (e.error == QJsonParseError::NoError);
		if (ok)
		{
			QJsonObject root = conf.object();
			//PortName = root["PortName"].toString();
			//COMBaudRate = root["COMBaudRate"].toInt();
			Port.Type = (Protos::PORT_TYPE)root["PortType"].toInt();
			Port.Params["COM"] = root["COM"].toString();
			Port.Params["COMSettings"] = root["COMSettings"].toString();
			Port.Params["Channel"]  = QString::number(root["PortChannel"].toInt());
			Port.Params["CANBitrate"] = QString::number(root["CANBitrate"].toInt());
			Port.Params["LoopbackMode"] = root["LoopbackMode"].toBool(false) ? "true" : "false";
			Port.Params["CANTimestamp"] = root["CANTimestamp"].toBool(false) ? "true" : "false";
			Port.Params["CANAcceptanceMask"] = root["CANAcceptanceMask"].toString();
			Port.Params["CANAcceptanceFilter"] = root["CANAcceptanceFilter"].toString();
			Port.Params["MoxaID"] = QString::number(root["MoxaID"].toInt());
			Port.Params["MoxaChannel"] = QString::number(root["MoxaChannel"].toInt());

			ServerPort = root["ServerPort"].toInt();
			ServerIP = root["ServerIP"].toString();
			//Protocol = StringToProto(root["Protocol"].toString());
			ScanPorts = root["ScanPorts"].toBool();
			McpRestart = root["McpRestart"].toBool();
			JournalSize = root["JournalSize"].toInt();
			MsgLogFn = root["MsgLogFn"].toString();
			//UILog = root["UILog"].toBool();

			QJsonArray cmd = root["Commands"].toArray();
			Commands.clear();
			for (const auto& cref : cmd)
			{
				QJsonObject c = cref.toObject();
				if (c != QJsonObject())
				{
					CommandDesc cmd;
					cmd.Name = c["Name"].toString();
					cmd.Desc = c["Desc"].toString();
					Commands.push_back(cmd);
				}
			}
		}
		else
		{
			error = e.errorString();
		}
		return ok;
	}

	void Save(QString& error)
	{
		QString ffName = QDir::currentPath();
		ffName.append(("/protos_server.conf"));

		QFile file(ffName);
		if (!file.open(QIODevice::WriteOnly))
		{
			error = QString("Ошибка открытия файла \"%1\"\n%2").arg(ffName).arg(file.errorString());
			return;
		}

		QJsonObject root;
		//root.insert("PortName", PortName);
		//root.insert("COMBaudRate", COMBaudRate);
		root.insert("PortType", (int)Port.Type);
		root.insert("PortID", Port.Params["ID"]);
		root.insert("PortChannel", Port.Params["Channel"]);
		root.insert("PortBaudRate", Port.Params["BaudRate"]);
		root.insert("CANAcceptanceMask", Port.Params["CANAcceptanceMask"]);
		root.insert("CANAcceptanceFilter", Port.Params["CANAcceptanceFilter"]);
		root.insert("ServerIP", ServerIP);
		root.insert("ServerPort", ServerPort);
		//root.insert("Protocol", ProtoToString(Protocol));
		root.insert("ScanPorts", ScanPorts);
		root.insert("JournalSize", JournalSize);
		root.insert("McpRestart", McpRestart);
		root.insert("MsgLogFn", MsgLogFn);
		//root.insert("UILog", UILog);

		QJsonArray cmds;
		for (auto& c : Commands)
		{
			QJsonObject cobj;
			cobj.insert("Name", c.Name);
			cobj.insert("Desc", c.Desc);
			cmds.push_back(cobj);
		}
		root.insert("Commands", cmds);

		QJsonDocument doc;
		doc.setObject(root);
		file.write(doc.toJson(QJsonDocument::Indented));
	}

}//namespace Conf