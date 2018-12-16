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

				memcpy(&this->Protocol.dhcp.xid, &buffer[4], sizeof(unsigned int));
				memcpy(&this->Protocol.dhcp.secs, &buffer[8], sizeof(unsigned short));
				memcpy(&this->Protocol.dhcp.flags, &buffer[10], sizeof(unsigned short));
				memcpy(&this->Protocol.dhcp.ciaddr, &buffer[12], sizeof(unsigned int));
				memcpy(&this->Protocol.dhcp.yiaddr, &buffer[16], sizeof(unsigned int));
				memcpy(&this->Protocol.dhcp.siaddr, &buffer[20], sizeof(unsigned int));
				memcpy(&this->Protocol.dhcp.giaddr, &buffer[24], sizeof(unsigned int));
				memcpy(&this->Protocol.dhcp.chaddr, &buffer[28], (sizeof(unsigned int) * 4));

				ClearBuffer(&this->Protocol.dhcp.sname, sizeof(this->Protocol.dhcp.sname));
				gethostname(this->Protocol.dhcp.sname, sizeof(this->Protocol.dhcp.sname));

				for (auto i = 240; i < length; i++)
				{
					if (static_cast<unsigned char>(buffer[i]) == static_cast<unsigned char>(0xff))
						break;

					if (static_cast<unsigned char>(buffer[i + 1]) == static_cast<unsigned char>(1))
						this->Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 2])));
					else
						this->Protocol.dhcp.AddOption(DHCP_Option(static_cast<unsigned char>(buffer[i]),
							static_cast<unsigned char>(buffer[i + 1]), &buffer[i + 2]));

					i += 1 + buffer[i + 1];
				}

				switch (Protocol.dhcp.get_flags())
				{
				case Broadcast:
					Protocol.dhcp.broadcast = true;
					break;
				case Unicast:
					Protocol.dhcp.broadcast = false;
					break;
				}

				if (Protocol.dhcp.HasOption(60))
				{
					char vendor[9];
					ClearBuffer(vendor, sizeof vendor);
					memcpy(&vendor, this->Protocol.dhcp.options.at(60).Value, sizeof vendor);

					if (memcmp(vendor, "PXEClient", sizeof vendor) == 0)
						this->Protocol.dhcp.set_vendor(PXEClient);

					if (memcmp(vendor, "PXEServer", sizeof vendor) == 0)
						this->Protocol.dhcp.set_vendor(PXEServer);

					if (memcmp(vendor, "APPLEBSDP", sizeof vendor) == 0)
						this->Protocol.dhcp.set_vendor(APPLEBSDP);
				}
				else
					this->Protocol.dhcp.set_vendor(UNKNOWNNO);

				if (this->Protocol.dhcp.HasOption(77))
				{
					char value[4];
					ClearBuffer(value, sizeof value);
					memcpy(&value, this->Protocol.dhcp.options.at(77).Value, sizeof value);

					if (memcmp(value, "iPXE", sizeof value) == 0)
						if (this->Protocol.dhcp.HasOption(175))
							this->Protocol.dhcp.isEtherBootClient = true;
					else
						this->Protocol.dhcp.isEtherBootClient = false;
				}

				if (Protocol.dhcp.HasOption(43))
				{
					char vendorbuffer[512];
					ClearBuffer(vendorbuffer, sizeof vendorbuffer);

					memcpy(vendorbuffer, Protocol.dhcp.options.at(43).Value,
						Protocol.dhcp.options.at(43).Length);

					std::vector<DHCP_Option> option;

					for (auto i = 0; i < length; i++)
					{
						if (static_cast<unsigned char>(vendorbuffer[i]) == static_cast<unsigned char>(0xff) ||
							static_cast<unsigned char>(vendorbuffer[i]) == 0x00)
							break;

						option.emplace_back(static_cast<unsigned char>(vendorbuffer[i]),
							static_cast<unsigned char>(vendorbuffer[i + 1]), &vendorbuffer[i + 2]);
	
						i += 1 + vendorbuffer[i + 1];
					}

					this->Protocol.dhcp.RemoveOption(55);

					for (auto & opt : option)
					{
						if (opt.Option == static_cast<unsigned char>(71))
						{
							unsigned short layer = 0;
							unsigned short type = 0;

							memcpy(&type, &opt.Value[0], sizeof(unsigned short));
							Protocol.dhcp.rbcp.set_item(type);

							memcpy(&layer, &opt.Value[2], sizeof(unsigned short));
							Protocol.dhcp.rbcp.set_layer(layer);
						}
					}
				}

				if (this->Protocol.dhcp.HasOption(93))
				{
					for (auto i = 0; i < Protocol.dhcp.options.at(93).Length; i = i + 2)
					{
						Protocol.dhcp.arch.emplace_back(static_cast<DHCPARCH>
							(Protocol.dhcp.options.at(93).Value[i]));
						
						Protocol.dhcp.arch.emplace_back(static_cast<DHCPARCH>
							(Protocol.dhcp.options.at(93).Value[i + 1]));
					}
				}

				break;
			case TFTP:

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

			this->Protocol.dhcp.broadcast = remote.sin_addr.s_addr == 0;
			this->Init(buffer, length);
		}

		EXPORT void client::Update() const
		{
			
		}
	}
}
