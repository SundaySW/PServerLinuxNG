#pragma once

#include "protos.h"
#include <vector>
#include <deque>
#include <cstdint>

extern int Pack2x(const char* src, int len, char* dst);

namespace Protos
{
	#pragma pack(push, 1)
	struct Header
	{
		// ������� ����� ��� � CAN-���������
		union {
			struct
			{
				unsigned char Reserved : 4;
				unsigned char Type : 4;
			}Bit;
			unsigned char Byte = 0;
		}ID0;							  // message type or index(in long msg. sequence)

		unsigned char Src = 0;
		unsigned char Dst = 0;

		union{
			struct
			{
				unsigned char SL  : 2;     // Short/Long sign
				unsigned char Pri : 2;     // Priority
				unsigned char PS  : 1;     // Protocol select: Protos/Raw
				unsigned char Unused : 3;  // Not used in CAN standard
			}Bit;
			unsigned char Byte = 0;
		}ID3;		
	};
	#pragma pack(pop)

	/*!
		Protos packet. Type - unsigned char(!). After format (see. protos_format.h)
		char will be interpreted incorrectly.
	*/
	class  Packet : public Header
	{
	public:
		Packet() {}
		Packet(const char* buffer, int len, bool noDLC);

		bool Is(SL_FLAG flag) const;

		Packet& Short();

		Packet& LongFirst(char pri, char src, char dst, MSG_TYPE type,
			unsigned short len);

		Packet& LongNext(unsigned char index, char pri, char src,
			char dst, const char* data, size_t len);

		Packet& FlowControl(FC_FLAG flag, char blockSize, char delay);
		bool Is(PROTOCOL ps) const { return (PROTOCOL)ID3.Bit.PS == ps; }

		Packet& SetIndex(unsigned char index) { ID0.Byte = index; return *this; }
		Packet& SetPri(char pri) { ID3.Bit.Pri = pri; return *this; }
		Packet& SetSrc(char src) { Src = src; return *this; }
		Packet& SetDst(char dst) { Dst = dst; return *this; }
		Packet& SetType(unsigned char type) { ID0.Bit.Type = type; return *this; }
		Packet& SetData(const char* data, int len);

		std::vector<char> ToBinary2x(char prefix = 0) const;
		std::vector<char> ToBinary(char prefix = 0) const;
		std::vector<char>& ToBinaryWithDlc(std::vector<char>& bin) const;
		std::vector<char>& ToBinary(std::vector<char>& bin, char prefix = 0) const;

		union
		{
			struct
			{
				//Buffer[0]...Buffer[7]
				union {
					FC_FLAG FCFlag;				///< FC flag in control flow packet
				};
				union {
					unsigned char Byte1;
					unsigned char BlockSize;	///< Block size in control flow packet
				};
				union {
					unsigned char Byte2;
					unsigned char Delay;		///< Delay in control flow packet
				};
			}
			Data;
			char DataBuffer[8];
			uint64_t Data64;
		};

		unsigned char Dlc = 0;
	};

	inline std::vector<char> ToBinary(const Header& hdr, std::vector<char>& bin)
	{
		bin.push_back(hdr.ID0.Byte);
		bin.push_back(hdr.Src);
		bin.push_back(hdr.Dst);
		bin.push_back(hdr.ID3.Byte);
		return bin;
	}

	using PacketArray = std::vector<Protos::Packet>;

}//namespace Protos