#include "protos_format.h"
#include "protos_msg.h"
#include <qregularexpression.h>
#include <qstring.h>
#include <qtextstream.h>

namespace Protos
{

	QString& Format(const Msg& msg, QString& dst, FORMAT fmt)
	{
		if (fmt == FORMAT_DEFAULT)
		{
			QString mt(GetMsgTypeName((Protos::MSG_TYPE)(msg.ID0.Bit.Type)));

			dst.append(QString("pri=%1 src=%2 dst=%3 type=%4")
				.arg(QString::number(msg.ID3.Bit.Pri, 16).toUpper(), 2, '0')
				.arg(QString::number(msg.Src, 16).toUpper(), 2, '0')
				.arg(QString::number(msg.Dst, 16).toUpper(), 2, '0')
				.arg(mt));

			if (!msg.Data.empty())
			{
				dst.append(" data=");
				for (auto byte : msg.Data)
					dst.append(QString("%1 ").arg(QString::number((unsigned char)byte, 16).toUpper(), 2, '0'));
			}
		}
		return dst;
	}

	QString Format(const Msg& msg, FORMAT fmt)
	{
		QString res;
		return Format(msg, res, fmt);
	}

	QString Format(const MsgView& msg, FORMAT fmt)
	{
		QString res;
		return Format(msg, res, fmt);
	}
	
	QString& Format(const MsgView& msg, QString& dst, FORMAT fmt)
	{
		if (fmt == FORMAT_DEFAULT)
		{
			QString mt(GetMsgTypeName((MSG_TYPE)msg.Type()));

			dst.append(QString("pri=%1 src=%2 dst=%3 type=%4")
				.arg(QString::number(msg.Pri(), 16).toUpper(), 2, '0')
				.arg(QString::number(msg.Src(), 16).toUpper(), 2, '0')
				.arg(QString::number(msg.Dst(), 16).toUpper(), 2, '0')
				.arg(mt));

			if (msg.Dlc() > 0)
			{
				dst.append(" data=");
				auto* data = msg.Data();
				for (int i = 0; i < msg.Dlc(); i++)
					dst.append(QString("%1 ").arg(QString::number((unsigned char)data[i], 16).toUpper(), 2, '0'));
			}
		}
		return dst;
	}

	bool ParseMsgStr(const QString& src, Msg& dst, FORMAT fmt)
	{
		if (fmt != FORMAT_DEFAULT) return false;

		/*CAUTION: do not capture 'da' of 'data' as byte value*/
		QRegularExpression re("(data|pri|src|dst|type)\\s*=\\s*(?(?![0-9A-Fa-f]{2}\\s*data)((?:[A-Fa-f0-9]{2}\\s*)*)|([0-9A-Fa-f]{2}))");
		auto it = re.globalMatch(src);
		if (!it.hasNext())
			return false;

		auto parseByte = [](const QString& field, const QString& data, unsigned char& var)
		{
			bool ok = false;
			var = (char)data.toInt(&ok, 16);
			if (!ok) throw field;
		};

		dst = Msg();
		try
		{
			while (it.hasNext())
			{
				auto match = it.next();
				auto field = match.captured(1);
				auto data = match.captured(2);
				if (data.isEmpty())
					data = match.captured(3);

				if (field.compare("pri", Qt::CaseInsensitive) == 0)
				{
					unsigned char pri;
					parseByte(field, data, pri);
					dst.ID3.Bit.Pri = pri;
				}
				else if (field.compare("src", Qt::CaseInsensitive) == 0)
					parseByte(field, data, dst.Src);
				else if (field.compare("dst", Qt::CaseInsensitive) == 0)
					parseByte(field, data, dst.Dst);
				else if (field.compare("type", Qt::CaseInsensitive) == 0)
				{
					unsigned char type = 0;
					parseByte(field, data, type);
					dst.ID0.Bit.Type = type;
				}
				else if (field.compare("data", Qt::CaseInsensitive) == 0)
				{
					QTextStream s(&data);
					s.setIntegerBase(16);
					dst.Data.resize(0);
					bool ok;
					while (!s.atEnd())
					{
						s.skipWhiteSpace();
						QString sb = s.read(2);
						dst.Data.push_back(sb.toInt(&ok,16));
						if (!ok) throw field;
					}
				}
			}
			return true;
		}
		catch (...)
		{
		}
		return false;
	}

}//namespace Protos