#pragma once

namespace Protos
{

	// Protos message types
	enum class MSG_TYPE : unsigned char
	{
		MODULE_MISC_CMD = 0x1,
		MODULE_SPEC_CMD = 0x2,
		MODULE_MISC_ANS = 0x3,
		MODULE_SPEC_ANS = 0x4,
		MODULE_MISC_ERR = 0x5,
		MODULE_SPEC_ERR = 0x6,
		PARAM_ANS	    = 0x8,
		PARAM_SET		= 0x9,
		PARAM_REQ		= 0x7,
		PARAM_ERR		= 0xA,
	};

	// Parameter's field
	enum class FIELD_ID : unsigned char
	{
		FIELD_VALUE				 = 0x2,
		FIELD_NAME				 = 0x3,
		FIELD_SEND_RATE			 = 0x4,
		FIELD_READ_ONLY			 = 0x5,
		FIELD_SEND_TIME			 = 0x6,
		FIELD_UPDATE_BEFORE_READ = 0x7, // 1 bit true\false, update before read
		FIELD_UPDATE_RATE		 = 0x8, //
		FIELD_UPDATE_TIME		 = 0x9, // внутренний счетчик на update
	};

	// Type of parameter's field
	enum class VALUE_TYPE : unsigned char
	{
		TYPE_VOID  = 0x0,
		TYPE_CHAR  = 0x1,
		TYPE_SHORT = 0x2,
		TYPE_LONG  = 0x3,
		TYPE_FLOAT = 0x4,
		TYPE_STR   = 0x5
	};

	// Flow control flag
	enum class FC_FLAG : unsigned char
	{
		FC_ACCEPT = 1,
		FC_WAIT   = 2,
		FC_ABORT  = 3
	};

	// Priority of message
	enum class PRIORITY : unsigned char
	{
		PRIORITY0 = 0,
		PRIORITY1,
		PRIORITY2,
		PRIORITY3
	};

	// Short/long packet flag
	enum class SL_FLAG : unsigned char
	{
		SL_SHORT = 0,
		SL_LONG_FIRST,
		SL_LONG_NEXT,
		SL_FLOW_CTRL
	};

	enum class PROTOCOL : unsigned char
	{
		PROTOS = 0,
		RAW    = 1
	};

	enum PORT_TYPE
	{
		PORT_PICA = 0,		///< Через COM порт (PICA)
		PORT_CAN_HAT,		///< Через Waveshare CAH-HAT
		PORT_CAN_USB_M      ///< TITAN CAN-USB-M adapter
	};

	extern const char* GetMsgTypeName(MSG_TYPE type);

}//namespace Protos