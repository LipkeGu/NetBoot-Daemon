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

namespace Netbootd
{
	namespace Network
	{
		class client
		{
		public:
			EXPORT client();;

			EXPORT client(ServiceType serviceType, std::string ident,
				sockaddr_in remote, const char* buffer, int length);
			EXPORT ~client();;

			std::string ident;
			sockaddr_in toAddr;

			struct
			{
				struct
				{
					unsigned char opcode;
					std::map<unsigned char, DHCP_Option> options;

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

					EXPORT void set_opcode(const unsigned char op)
					{
						this->opcode = op;
					}

					EXPORT unsigned char get_opcode() const
					{
						return this->opcode;
					}

					EXPORT void set_hwtype(const unsigned char hwtype)
					{
						this->hwtype = hwtype;
					}

					EXPORT unsigned char get_hwtype() const
					{
						return this->hwtype;
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

					EXPORT void set_flags(const unsigned short flags)
					{
						this->secs = flags;
					}

					EXPORT unsigned short get_flags() const
					{
						return this->flags;
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

					EXPORT void set_servername(const std::string& sname)
					{
						memset(&this->sname, 0, 64);
						memcpy(&this->sname, sname.c_str(), sname.size());
					}

					EXPORT std::string get_servername() const
					{
						return std::string(this->sname);
					}

					EXPORT void set_filename(const std::string& file)
					{
						this->filename = file;
					}

					EXPORT std::string get_filename() const
					{
						return this->filename;
					}
				} dhcp;

				struct
				{
					std::string filename;
				} tftp;
			} Protocol;
		};
	}
}
