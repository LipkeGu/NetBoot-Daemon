#pragma once
#include "../../../Include.h"
namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef enum TFTP_OPCODE
		{
			TFTP_Read = 0x0001,
			TFTP_Write = 0x0002,
			TFTP_Data = 0x0003,
			TFTP_Ack = 0x0004,
			TFTP_Error = 0x0005,
			TFTP_OACK = 0x0006
		} TFTP_OPCODE;
	}
}
