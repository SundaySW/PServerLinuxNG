#pragma once

#include "protos_packet.h"

namespace Protos
{

	struct Msg : public Header
	{
		Msg() {}

		Msg(const char* buffer, int len)
		{
			if (len < sizeof(Header)) return;
			*(Header*)(this) = *((Header*)(buffer));
			len -= sizeof(unsigned int);
			Data.reserve(len);
			buffer += sizeof(Header);
			Data.insert(Data.end(), buffer, buffer + len);
		}

		std::vector<char> ToBinary() const
		{
			std::vector<char> bin;
			return ToBinary(bin);
		}

		std::vector<char>& ToBinary(std::vector<char>& bin) const
		{
			bin.resize(0);
			bin.reserve(Data.size() + 4);
			Protos::ToBinary((Header&)(*this), bin);
			bin.insert(std::end(bin), Data.begin(), Data.end());
			return bin;
		}

		bool IsLong() const
		{
			return (Data.size() > 8);
		}

		std::deque<Packet> ToPackets() const
		{
			std::deque<Packet> q;
			if (IsLong())
			{
				int n = Data.size() / 8;
				if (Data.size() % 8)
					n++;

				q.resize(n/*data packets*/ + 1/*start packet*/);

				auto& firstPacket = q.front();
				firstPacket.LongFirst(ID3.Byte, Src, Dst, (Protos::MSG_TYPE)(ID0.Bit.Type), (unsigned short)Data.size());

				const char* data = Data.data();
				int len = Data.size();

				unsigned char index = 1;
				auto nextPacket = std::next(q.begin());

				while (nextPacket != q.end() && len > 0)
				{
					n = (len > 8) ? 8 : len;
					nextPacket->LongNext(index++, ID3.Byte, Src, Dst, data, n);
					len -= n;
					data += n;
					nextPacket = std::next(nextPacket);
				}
			}
			else
			{
				q.emplace_back();
				auto& packet = q.back();
				packet.Src = Src;
				packet.Dst = Dst;
				packet.ID3.Byte = ID3.Byte;
				packet.ID0.Byte = ID0.Byte;
				packet.SetData(Data.data(), Data.size());
			}
			return q;
		}

		std::vector<char> Data;
	};

	class MsgView
	{
	public:
		MsgView(const char* buffer, int size)
			: Buffer(buffer)
			, Size(size)
		{}
		bool IsLong() const { return Dlc() > 8; }
		unsigned char Pri() const { return Buffer[0]; }
		unsigned char Src() const { return Buffer[1]; }
		unsigned char Dst() const { return Buffer[2]; }
		unsigned char Type()const { return (unsigned char)Buffer[3]; }
		const char* Data() const { return &Buffer[4]; }
		int Dlc() const { return Size - 4; }
		operator bool() const { return Buffer && Size >= 4; }
	private:
		const char* Buffer;
		int Size;
	};

}//Protos