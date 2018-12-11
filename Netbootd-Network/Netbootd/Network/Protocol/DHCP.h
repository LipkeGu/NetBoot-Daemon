#pragma once

#define EXPORT __declspec(dllexport)
#define INLINE __inline
#include "../Client/Client.h"

#ifndef NETBOOTD_PROTO_DHCP
#define NETBOOTD_PROTO_DHCP
#include "DHCP_Defines.h"
namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef struct DHCP_Packet
		{
			EXPORT ~DHCP_Packet()
			{
			}

			EXPORT DHCP_Packet() { }

			EXPORT DHCP_Packet(client c)
			{
				memset(this, 0, sizeof(this));
				op = c.Protocol.dhcp.get_opcode();
				hwtype = c.Protocol.dhcp.get_hwtype();
				hwlength = c.Protocol.dhcp.get_hwlength();
				hops = c.Protocol.dhcp.get_hops();
				xid = c.Protocol.dhcp.get_xid();
				secs = c.Protocol.dhcp.get_secs();
				flags = c.Protocol.dhcp.get_flags();

				ciaddr = c.Protocol.dhcp.get_clientIP();
				yiaddr = c.Protocol.dhcp.get_yourIP();
				siaddr = c.Protocol.dhcp.get_nextIP();
				giaddr = c.Protocol.dhcp.get_relayIP();

				memset(chaddr, 0, 16);
				memcpy(&chaddr, c.Protocol.dhcp.get_hwaddress(), 16);

				memset(sname, 0, 64);
				memcpy(&sname, c.Protocol.dhcp.get_servername().c_str(),
					strlen(c.Protocol.dhcp.get_servername().c_str()));

				memset(file, 0, 128);
				memcpy(&file, c.Protocol.dhcp.get_filename().c_str(),
					strlen(c.Protocol.dhcp.get_filename().c_str()));

				// decimal > 99.130.83.99
				cookie[0] = static_cast<unsigned char>(0x63);
				cookie[1] = static_cast<unsigned char>(0x82);
				cookie[2] = static_cast<unsigned char>(0x53);
				cookie[3] = static_cast<unsigned char>(0x63);

				c.Protocol.dhcp.RemoveOption(50);
				c.Protocol.dhcp.RemoveOption(57);
				c.Protocol.dhcp.RemoveOption(61);
				c.Protocol.dhcp.RemoveOption(93);
				c.Protocol.dhcp.RemoveOption(94);

				if (!c.Protocol.dhcp.options.empty())
				{
					memset(options, 0, 1024);
					auto _offset = 0;

					for (auto & option : c.Protocol.dhcp.options)
					{
						_offset += Write(&option.second.Option, sizeof(unsigned char), _offset);
						_offset += Write(&option.second.Length, sizeof(unsigned char), _offset);

						if (option.second.Length != 1)
							_offset += Write(&option.second.Value, option.second.Length, _offset);
						else if (option.second.Length != 0)
							_offset += Write(&option.second.Value, option.second.Length, _offset);
					}
				}
			}

			EXPORT int Write(const void* data, const int length, const int position)
			{
				memcpy(&options[position], data, length);
				return length;
			}

			unsigned char op;
			unsigned char hwtype;
			unsigned char hwlength;
			unsigned char hops;
			unsigned int xid;

			unsigned short secs;
			unsigned short flags;

			unsigned int ciaddr;
			unsigned int yiaddr;
			unsigned int siaddr;
			unsigned int giaddr;

			char chaddr[16];
			char sname[64];
			char file[128];
			unsigned char cookie[4];

			char options[1024];
		} DHCP_Packet;
	}
}
#endif