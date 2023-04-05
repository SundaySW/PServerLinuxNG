#pragma once

#include <qstringlist.h>
#include <fstream>
#include <qlist.h>
#include <string>
#include <map>
#include <protos.h>

namespace Conf
{
	struct PortDesc
	{
		Protos::PORT_TYPE Type = Protos::PORT_CAN_HAT;
		std::map<QString, QString> Params;
	};

	extern PortDesc Port;
	extern int Protocol;
	extern unsigned short ServerPort;
	extern QString ServerIP;
	extern bool ScanPorts;
	extern bool McpRestart;
	extern int JournalSize;
	extern QString MsgLogFn;

	struct CommandDesc
	{
		QString Name, Desc;
	};
	extern QList<CommandDesc> Commands;

	extern bool Read(const std::string fileName, QString& error);
	extern void Save(QString& error);
}