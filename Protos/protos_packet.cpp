#include "protos_packet.h"

namespace Protos
{

	bool Packet::Is(SL_FLAG flag) const
	{
		unsigned char sl = ID3.Bit.SL;
		return (ID3.Bit.PS == 0 && sl == (unsigned char)flag);
	}

	Packet::Packet(const char* buffer, int len, bool noDLC)
	{
		if (len < sizeof(Header) || len > 12) return;
		
		int i = 0;
		*((Header*)(this)) = *((Header*)buffer);
		len -= sizeof(Header);

		if (noDLC)
			Dlc = len;
		else
			Dlc = buffer[i++];

		for (int j = 0; j < Dlc; j++)
			DataBuffer[j] = buffer[i++];
	}

	std::vector<char> Packet::ToBinary2x(char prefix) const
	{
		std::vector<char> bin;
		bin = ToBinaryWithDlc(bin);

		std::vector<char> bin2x;
		bin2x.resize(2 * bin.size() + (prefix ? 2 : 0));

		int n = 0;
		if (prefix)
			bin2x[n++] = prefix;

		n += Pack2x(bin.data(), bin.size(), &bin2x[n]);

		if (prefix)
			bin2x[n] = '\n';

		return std::move(bin2x);
	}

	std::vector<char> Packet::ToBinary(char prefix) const
	{
		std::vector<char> bin;
		return ToBinary(bin, prefix);
	}

	std::vector<char>& Packet::ToBinaryWithDlc(std::vector<char>& bin) const
	{
		bin.clear();
		bin.reserve(sizeof(Header) + Dlc + 1);
		Protos::ToBinary(*this, bin);
		bin.push_back(Dlc);
		if (Dlc > 0)
			bin.insert(std::end(bin), DataBuffer, DataBuffer + Dlc);
		return bin;
	}

	std::vector<char>& Packet::ToBinary(std::vector<char>& bin, char prefix) const
	{
		bin.clear();
		bin.reserve(sizeof(Header) + Dlc);
		if (prefix) bin.push_back(prefix);
		Protos::ToBinary(*this, bin);
		if (Dlc > 0)
			bin.insert(std::end(bin), DataBuffer, DataBuffer + Dlc);
		if (prefix) bin.push_back('\n');
		return bin;
	}

	Packet& Packet::Short()
	{//make short packet
		ID3.Bit.PS = 0;
		ID3.Bit.SL = 0;
		return *this;
	}

	Packet& Packet::LongFirst(char pri, char src, char dst, MSG_TYPE type, unsigned short len)
	{// make long first packet
		ID3.Bit.PS = 0;
		ID3.Bit.SL = 1;
		ID0.Bit.Type = (unsigned char)type;
		ID3.Bit.Pri = pri;
		Src = src;
		Dst = dst;
		const char* p = (const char*)(&len);
		SetData(p, sizeof(len));
		return *this;
	};

	Packet& Packet::LongNext(unsigned char index, char pri, char src, char dst, const char* data, size_t len)
	{// make long next
		ID3.Bit.Pri = pri;
		ID3.Bit.PS = 0;
		ID3.Bit.SL = 2;
		ID0.Byte = index;
		Src = src;
		Dst = dst;
		SetData(data, len);
		return *this;
	};

	Packet& Packet::FlowControl(FC_FLAG flag, char blockSize, char delay)
	{//make flow control
		ID3.Bit.PS = 0;
		ID3.Bit.Pri = 0;
		ID3.Bit.SL = 3;
		char data[3] = { (char)flag, blockSize, delay };
		SetData(data, 3);
		return *this;
	};

	Packet& Packet::SetData(const char* data, int len)
	{
		if (len > 8) return *this;
		Dlc = len;
		for (int i = 0; i < len; i++)
			DataBuffer[i] = data[i];
		return *this;
	}

}// namespace