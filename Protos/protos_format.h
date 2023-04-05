#pragma once

class QString;

namespace Protos
{
	struct Msg;
	class MsgView;

	enum FORMAT
	{
		FORMAT_DEFAULT = 0
	};

	extern QString  Format(const Msg& msg, FORMAT fmt = FORMAT_DEFAULT);
	extern QString& Format(const Msg& msg, QString& dst, FORMAT fmt = FORMAT_DEFAULT);
	extern QString  Format(const MsgView& msg, FORMAT fmt = FORMAT_DEFAULT);
	extern QString& Format(const MsgView& msg, QString& dst, FORMAT fmt = FORMAT_DEFAULT);

	extern bool ParseMsgStr(const QString& src, Msg& dst, FORMAT fmt = FORMAT_DEFAULT);

}//namespace Protos
