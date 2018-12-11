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
#include "../../../Include.h"
#include "../Protocol/DHCP_Defines.h"

#ifndef NETBOOTD_CLIENT
#define NETBOOTD_CLIENT
namespace Netbootd
{
	namespace Network
	{
		class client
		{
		public:
			EXPORT client();;

			EXPORT client(const ServiceType serviceType, const std::string& ident,
				const sockaddr_in remote, const char* buffer, const int length);
			EXPORT ~client();
			EXPORT bool Init(const char* buffer, const int length);

			std::string ident;
			sockaddr_in toAddr;
			ServiceType serviceType;

			struct
			{
				struct
				{
					std::map<unsigned char, DHCP_Option> options;
					EXPORT void AddOption(const DHCP_Option option)
					{
						RemoveOption(option.Option);

						options.insert(std::pair<unsigned char,
							DHCP_Option>(option.Option, option));
					}

					EXPORT void RemoveOption(const unsigned char opt)
					{
						if (HasOption(opt))
							options.erase(opt);
					}

					EXPORT bool HasOption(const unsigned char option)
					{
						return options.find(option) != options.end();
					}

					EXPORT void set_opcode(const DHCPOPCODE op)
					{
						this->opcode = op;
					}

					EXPORT DHCPOPCODE get_opcode() const
					{
						return this->opcode;
					}

					EXPORT void set_hwtype(const DHCPHARDWARETYPE hwtype)
					{
						this->hwtype = static_cast<unsigned char>(hwtype);
					}

					EXPORT DHCPHARDWARETYPE get_hwtype() const
					{
						return static_cast<DHCPHARDWARETYPE>(this->hwtype);
					}

					EXPORT void set_hwlength(const unsigned char length)
					{
						this->hwlength = length;
					}

					EXPORT unsigned char get_hwlength() const
					{
						return this->hwlength;
					}

					EXPORT void set_hops(const unsigned char hops)
					{
						this->hops = hops;
					}

					EXPORT unsigned char get_hops() const
					{
						return this->hops;
					}

					EXPORT void set_xid(const unsigned int xid)
					{
						this->xid = xid;
					}

					EXPORT unsigned int get_xid() const
					{
						return this->xid;
					}

					EXPORT void set_secs(const unsigned short secs)
					{
						this->secs = secs;
					}

					EXPORT unsigned short get_secs() const
					{
						return this->secs;
					}

					EXPORT void set_flags(const DHCPFLAGS flags)
					{
						this->secs = static_cast<unsigned short>(flags);
					}

					EXPORT DHCPFLAGS get_flags() const
					{
						return static_cast<DHCPFLAGS>(this->flags);
					}

					EXPORT void set_clientIP(const unsigned int ip)
					{
						this->ciaddr = ip;
					}

					EXPORT unsigned int get_clientIP() const
					{
						return this->ciaddr;
					}

					EXPORT void set_yourIP(const unsigned int ip)
					{
						this->yiaddr = ip;
					}

					EXPORT unsigned int get_yourIP() const
					{
						return this->yiaddr;
					}

					EXPORT void set_nextIP(const unsigned int ip)
					{
						this->siaddr = ip;
						auto x = AddressStr(this->siaddr);

						AddOption(DHCP_Option(66, strlen(x), x));
					}

					EXPORT unsigned int get_nextIP() const
					{
						return this->siaddr;
					}

					EXPORT void set_relayIP(const unsigned int ip)
					{
						this->giaddr = ip;
					}

					EXPORT unsigned int get_relayIP() const
					{
						return this->giaddr;
					}

					EXPORT void set_hwaddress(const char* mac)
					{
						memset(&this->chaddr, 0, 16);
						memcpy(&this->chaddr, mac, this->get_hwlength());
					}

					EXPORT const char* get_hwaddress() const
					{
						return this->chaddr;
					}

					EXPORT void set_vendor(const DHCPPXEVENDOR vendor)
					{
						this->vendor = vendor;
					}

					EXPORT DHCPPXEVENDOR get_vendor() const
					{
						return this->vendor;
					}

					EXPORT void set_servername(const std::string& sname)
					{
						memset(&this->sname, 0, 64);
						memcpy(&this->sname, sname.c_str(), sname.size());
					}

					EXPORT std::string get_servername() const
					{
						return std::string(this->sname);
					}

					EXPORT bool IsEtherBootClient() const
					{
						return this->isEtherBootClient;
					}

					EXPORT void set_filename(const std::string& file)
					{
						this->filename = file;
						AddOption(DHCP_Option(67, this->filename.size(),
							this->filename.c_str()));
					}

					EXPORT std::string get_filename() const
					{
						return this->filename;
					}

					DHCPOPCODE opcode;
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
					std::string filename;
					DHCPPXEVENDOR vendor;

					bool isEtherBootClient = false;

					struct
					{
						BootServerType type;
						EXPORT BootServerType get_bootserverType() const
						{
							return this->type;
						}

						EXPORT void set_bootserverType(const BootServerType type)
						{
							this->type = type;;
						}
					} rbcp;

					struct
					{

					} bsdp;
				} dhcp;

				struct
				{
					std::string filename;
					unsigned int bytesRead;
					unsigned int bytesToRead;

					unsigned short block;
					unsigned short blocksize;
					unsigned short windowsSize;
					unsigned short lastAckedBlock;

					EXPORT unsigned int get_bytesRead() const;
					EXPORT void set_bytesRead(const unsigned int bytes);

					EXPORT unsigned int get_bytesToRead() const;
					EXPORT void set_bytesToRead(const unsigned int bytes);

					EXPORT bool TFTP_HasOption(const char* option, const char* buffer, int length)
					{
						uint32_t i = 0;

						for (auto i = 2; i < length; i++)
							if (memcmp(option, &buffer[i], strlen(option)) == 0)
								return true;

						return false;
					}

				} tftp;
			} Protocol;
		};
	}
}
#endif