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
		EXPORT client::~client()
		{
		}

		EXPORT bool client::Init(const char* buffer, const int length)
		{
			switch (this->serviceType)
			{
			case DHCP:
			case BOOTP:
				this->Protocol.dhcp.opcode = BOOTREPLY;
				this->Protocol.dhcp.hwtype = buffer[1];
				this->Protocol.dhcp.hwlength = buffer[2];
				this->Protocol.dhcp.hops = buffer[3];

				memcpy(&this->Protocol.dhcp.xid, &buffer[4], 4);
				memcpy(&this->Protocol.dhcp.secs, &buffer[8], 2);
				memcpy(&this->Protocol.dhcp.flags, &buffer[10], 2);
				memcpy(&this->Protocol.dhcp.ciaddr, &buffer[12], 4);
				memcpy(&this->Protocol.dhcp.yiaddr, &buffer[16], 4);
				memcpy(&this->Protocol.dhcp.siaddr, &buffer[20], 4);
				memcpy(&this->Protocol.dhcp.giaddr, &buffer[24], 4);
				memcpy(&this->Protocol.dhcp.chaddr, &buffer[28], 16);

				ClearBuffer(&this->Protocol.dhcp.sname, sizeof(this->Protocol.dhcp.sname));
				gethostname(this->Protocol.dhcp.sname, sizeof(this->Protocol.dhcp.sname));

				for (auto i = 240; i < length; i++)
				{
					if (static_cast<unsigned char>(buffer[i]) == static_cast<unsigned char>(0xff) ||
						static_cast<unsigned char>(buffer[i]) == -1)
						break;

					if (static_cast<unsigned char>(buffer[i + 1]) == static_cast<unsigned char>(1))
						this->Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 2])));
					else
						this->Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 1]), &buffer[i + 2]));

					i += 1 + buffer[i + 1];
				}

				if (Protocol.dhcp.HasOption(60))
				{
					char vendor[9];
					ClearBuffer(vendor, 9);
					memcpy(&vendor, this->Protocol.dhcp.options.at(60).Value, 9);

					if (memcmp(vendor, "PXEClient", 9) == 0)
						this->Protocol.dhcp.set_vendor(PXEClient);

					if (memcmp(vendor, "PXEServer", 9) == 0)
						this->Protocol.dhcp.set_vendor(PXEServer);

					if (memcmp(vendor, "APPLEBSDP", 9) == 0)
						this->Protocol.dhcp.set_vendor(APPLEBSDP);
				}
				else
					this->Protocol.dhcp.set_vendor(UNKNOWNNO);

				if (this->Protocol.dhcp.HasOption(77))
				{
					char value[4];
					ClearBuffer(value, 4);
					memcpy(&value, this->Protocol.dhcp.options.at(77).Value, 4);

					if (memcmp(value, "iPXE", 4) == 0)
					{
						if (this->Protocol.dhcp.HasOption(175))
							this->Protocol.dhcp.isEtherBootClient = true;

						/* TEST */
						if (Protocol.dhcp.get_flags() == Unicast)
							Protocol.dhcp.set_flags(Broadcast);
					}
					else
						this->Protocol.dhcp.isEtherBootClient = false;
				}
				break;
			default:;
			}

			return true;
		}

		EXPORT client::client(const ServiceType serviceType, const std::string& ident,
			const sockaddr_in remote, const char* buffer, const int length)
		{
			ClearBuffer(&this->toAddr, sizeof(this->toAddr));
			this->toAddr = remote;

			this->serviceType = serviceType;
			this->ident = ident;

			this->toAddr.sin_addr.s_addr = (remote.sin_addr.s_addr == 0)
				? INADDR_BROADCAST : remote.sin_addr.s_addr;

			this->Init(buffer, length);
		}
	}
}
