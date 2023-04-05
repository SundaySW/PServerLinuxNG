#include "./protos.h"

namespace Protos
{

	const char* GetMsgTypeName(MSG_TYPE type)
	{
		switch (type)
		{
		case MSG_TYPE::MODULE_MISC_CMD:	return "MISC_CMD";
		case MSG_TYPE::MODULE_SPEC_CMD:	return "SPEC_CMD";
		case MSG_TYPE::MODULE_MISC_ANS:	return "MISC_ANS";
		case MSG_TYPE::MODULE_SPEC_ANS:	return "SPEC_ANS";
		case MSG_TYPE::MODULE_MISC_ERR:	return "MISC_ERR";
		case MSG_TYPE::MODULE_SPEC_ERR:	return "SPEC_ERR";
		case MSG_TYPE::PARAM_REQ:		return "PARAM_REQ";
		case MSG_TYPE::PARAM_ANS:		return "PARAM_ANS";
		case MSG_TYPE::PARAM_SET:		return "PARAM_SET";
		case MSG_TYPE::PARAM_ERR:		return "PARAM_ERR";
		default:
			break;
		};
		return "";
	}

}//namespace Protos