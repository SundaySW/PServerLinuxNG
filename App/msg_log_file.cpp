#include "msg_log_file.h"
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <QDir>

bool MsgLogFile::Open(const QString& fn)
{
	if (fn.isEmpty())
		return false;

	QString ffn = QDir::currentPath();
	ffn.append(QStringLiteral("/%1.csv").arg(fn));

	bool exist = QFile::exists(ffn);
	
	setFileName(ffn);
	if (!open(QIODevice::Append|QIODevice::Text))
		return false;
	
	if (!exist)
	{
		QTextStream s(this);
		s << "dir,date,time,id3,src,dst,id0,dlc,data\r\n";
		s.flush();
	}

	return true;
}

void MsgLogFile::Write(const Protos::Packet& packet, const char* dir)
{
	if (!isOpen()) return;
	auto timestamp = QDateTime::currentDateTime();
	
	QString str = QString("%1,\"%2\",\"%3\",%4,%5,%6,%7,%8")
		.arg(dir)
		.arg(timestamp.toString("dd.MM.yyyy"))
		.arg(timestamp.toString("hh.mm.ss:zzz"))
		.arg(QString::number(packet.ID3.Byte, 16).toUpper(), 2, '0')
		.arg(QString::number(packet.Src, 16).toUpper(), 2, '0')
		.arg(QString::number(packet.Dst, 16).toUpper(), 2, '0')
		.arg(QString::number(packet.ID0.Byte, 16).toUpper(), 2, '0')
		.arg(QString::number(packet.Dlc, 16).toUpper(), 2, '0');
	
	if (packet.Dlc)
	{
		QTextStream ss(&str);
		
		ss.setPadChar('0');
		for (int i = 0; i < packet.Dlc; i++)
		{
			ss.setFieldWidth(1);
			ss << ',';

			ss.setFieldWidth(2);
			ss << QString::number((unsigned char)packet.DataBuffer[i], 16).toUpper();
		}
	}

	str.append("\n");

	QTextStream fs(this);
	fs << str;
}