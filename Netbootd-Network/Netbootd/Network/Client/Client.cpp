/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "Client.h"

namespace Netbootd
{
	namespace Network
	{
		EXPORT client::client()
		{
		}

		EXPORT client::~client()
		{
		}

		EXPORT client::client(ServiceType serviceType, const std::string ident,
			const sockaddr_in remote, const char* buffer, const int length)
		{
			ClearBuffer(&toAddr, sizeof(toAddr));
			toAddr.sin_addr.s_addr = (remote.sin_addr.s_addr == 0)
			? INADDR_BROADCAST : remote.sin_addr.s_addr;

			this->ident = ident;
			switch (serviceType)
			{
			case DHCP:
			case BOOTP:
				toAddr.sin_family = remote.sin_family;
				toAddr.sin_port = remote.sin_port;
				Protocol.dhcp.opcode = 2;
				Protocol.dhcp.hwtype = buffer[1];
				Protocol.dhcp.hwlength = buffer[2];
				Protocol.dhcp.hops = buffer[3];

				memcpy(&Protocol.dhcp.xid, &buffer[4], 4);
				memcpy(&Protocol.dhcp.secs, &buffer[8], 2);
				memcpy(&Protocol.dhcp.flags, &buffer[10], 2);
				memcpy(&Protocol.dhcp.ciaddr, &buffer[12], 4);
				memcpy(&Protocol.dhcp.yiaddr, &buffer[16], 4);
				memcpy(&Protocol.dhcp.siaddr, &buffer[20], 4);
				memcpy(&Protocol.dhcp.giaddr, &buffer[24], 4);
				memcpy(&Protocol.dhcp.chaddr, &buffer[28], 16);

				ClearBuffer(&Protocol.dhcp.sname, sizeof(Protocol.dhcp.sname));
				gethostname(Protocol.dhcp.sname, sizeof(Protocol.dhcp.sname));

				for (auto i = 240; i < length; i++)
				{
					if (static_cast<unsigned char>(buffer[i]) == static_cast<unsigned char>(0xff) ||
						static_cast<unsigned char>(buffer[i]) == -1)
						break;

					if (static_cast<unsigned char>(buffer[i + 1]) == static_cast<unsigned char>(1))
						Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 2])));
					else
						Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 1]), &buffer[i + 2]));

					i += 1 + buffer[i + 1];
				}
				break;
			default:;
			}
		}
	}
}
