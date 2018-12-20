#pragma once
#define EXPORT __declspec(dllexport)
#define INLINE __inline
#include "../Client/Client.h"

#ifndef NETBOOTD_PROTO_TFTP
#define NETBOOTD_PROTO_TFTP
#include "TFTP_Defines.h"

namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef struct TFTP_Packet
		{

			char* payload = nullptr;

			EXPORT TFTP_Packet() {}
			EXPORT ~TFTP_Packet()
			{
			}

			EXPORT TFTP_Packet(const TFTP_OPCODE opcode,
				const _SIZE_T length, const unsigned short block)
			{
				payload = new char[length];
				ClearBuffer(payload, length);
				auto op =  static_cast<unsigned short>(opcode);
				auto bl = static_cast<unsigned short>(block);

				memcpy(&payload[0], &op, sizeof(unsigned short));
				memcpy(&payload[2], &bl, sizeof(unsigned short));
			}

			EXPORT TFTP_Packet(const TFTP_OPCODE opcode, const _SIZE_T length)
			{
				payload = new char[length];
				ClearBuffer(payload, length);
				auto op = BS16(static_cast<unsigned short>(opcode));
				
				memcpy(&payload[0], &op, sizeof(unsigned short));
			}

			EXPORT _SIZE_T Write(const void* data, const _SIZE_T length, const _SIZE_T offset) const
			{
				memcpy(&payload[offset], data, length);
				return length;
			}
		} TFTP_Packet;
	}
}
#endif
