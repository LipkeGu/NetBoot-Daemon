#pragma once
#define EXPORT __declspec(dllexport)
#define INLINE __inline
#include "../Client/Client.h"

#ifndef NETBOOTD_PROTO_DHCP
#define NETBOOTD_PROTO_DHCP
#include "TFTP_Defines.h"

namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef struct TFTP_Packet
		{
			unsigned short opcode = 0;
			unsigned short block = 0;
			char* payload = nullptr;

			EXPORT TFTP_Packet() {}
			EXPORT ~TFTP_Packet() {}

			EXPORT TFTP_Packet(const TFTP_OPCODE opcode, const unsigned short block)
			{
				this->opcode = BS16(opcode);
				this->block = BS16(block);
			}

			EXPORT void set_opcode(const TFTP_OPCODE opcode)
			{
				this->opcode = BS16(opcode);
			}

			EXPORT TFTP_OPCODE get_opcode() const
			{
				return static_cast<TFTP_OPCODE>(BS16(this->opcode));
			}

			EXPORT void set_block(const unsigned short block)
			{
				this->block = BS16(block);
			}

			EXPORT unsigned short get_block() const
			{
				return static_cast<unsigned short>(BS16(this->block));
			}

			EXPORT void set_payload(const char* data, const _SIZE_T length)
			{
				ClearBuffer(this->payload, length);
				memcpy(&this->payload, data, length);
			}
		};
	}
}
#endif
