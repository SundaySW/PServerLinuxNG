#pragma once

#include <qfile.h>
#include "protos_msg.h"

class MsgLogFile : public QFile
{
public:
	bool Open(const QString& fn);
	void Write(const Protos::Packet& msg, const char* dir="rx");
};