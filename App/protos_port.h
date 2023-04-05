#pragma once

#include <qstring.h>
#include <map>
#include <protos_packet.h>
#include <optional>

/*!
	����� ����� ��� ���������(COM port, MOXA CP602)
	�����������/������������ ������(������) �� Protos-����.
*/
namespace Protos
{
	class Port
	{
	public:
		using StringMap = std::map<QString, QString>;

		virtual ~Port() {};
		virtual bool Close() = 0;
		virtual QString FormatError(int error) const = 0;
		virtual std::optional<int> GetLastError() const = 0;
		virtual bool IsOpen() const = 0;
		virtual bool Open(const StringMap& params) = 0;
		virtual int Read(Packet* packets, int count) = 0;
		virtual bool Write(const Packet& packet) = 0;
	};
}