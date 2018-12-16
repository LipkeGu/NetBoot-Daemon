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
#include "../Include.h"
#include "Network/Server/Server.h"
#include "Network/Protocol/DHCP.h"
#include "NetBootd.h"
#include "../../Netbootd-System/System/Filesystem.h"

EXPORT Netbootd::Network::Server server;
EXPORT Netbootd::System::Filesystem fs;
EXPORT void Handle_Request(const std::string& ident, const ServerMode serverMode,
	const ServiceType serviceType, const char*, Netbootd::Network::client _client);
EXPORT Netbootd::Network::client createIdent(const char* buffer, ServiceType serviceType,
	Netbootd::Network::client& client);

EXPORT void Select_Bootfile(Netbootd::Network::client* c);

EXPORT void Handle_DHCP_Discover(const std::string& ident, Netbootd::Network::client* c,
	Netbootd::Network::DHCP_Packet* packet);

EXPORT void Handle_DHCP_Request(Netbootd::Network::client* c,
	Netbootd::Network::DHCP_Packet* packet);



EXPORT void GenerateBootMenue(const Netbootd::Network::client* client,
	std::vector<DHCP_Option>* vendorOpts);

EXPORT void GenerateMulticastOption(const std::string& ident, Netbootd::Network::
	client* client, std::vector<DHCP_Option>* vendorOpts);

EXPORT NetBootd::NetBootd()
= default;

EXPORT void NetBootd::Init()
{
	printf("Network Boot daemon 0.4 (BETA)\nStarting Storage subsystem...\n");

	if (fs.Init())
	{
#ifdef _WIN32
		if (!Netbootd::Network::Server::init_winsock())
			return;
#endif
		server.AddEndpoint(UDP,
			DHCP, "DHCP-Proxy");

		server.AddEndpoint(UDP,
			BOOTP, "DHCP-Boot");

		server.AddEndpoint(UDPMCAST,
			TFTP, "TFTP-Server");
	}
}

EXPORT int StripPacket(const char* buffer, int buflen)
{
	for (auto i = buflen; buflen > 240; i = i - 1)
		if (static_cast<unsigned char>(buffer[i])
			== static_cast<unsigned char>(0xff))
			return i + 1;

	return buflen;
}

EXPORT const char* AddressStr(const unsigned ip)
{
	struct in_addr addr {};
	ClearBuffer(&addr, sizeof(addr));

	addr.s_addr = ip;
	return inet_ntoa(addr);
}

EXPORT Netbootd::Network::client createIdent(const char* buffer, ServiceType serviceType,
	Netbootd::Network::client& client)
{
	char* hwaddr;
	uint8_t hwadr[6];
	switch (serviceType)
	{
	case BOOTP:
	case DHCP:
		hwaddr = new char[buffer[2] + 12];
		ClearBuffer(hwaddr, buffer[2] + 12);

		memcpy(&hwadr, &buffer[28], buffer[2]);

		sprintf(hwaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwadr[0],
			hwadr[1], hwadr[2], hwadr[3], hwadr[4], hwadr[5]);

		client.ident = std::string(hwaddr);
		delete[] hwaddr;

		break;
	case TFTP:
	default:
		client.ident = inet_ntoa(client.toAddr.sin_addr);
		break;
	}

	return client;
}


EXPORT void Handle_DHCP_Discover(const std::string& ident, Netbootd::Network::client* c,
	Netbootd::Network::DHCP_Packet* packet)
{
	std::vector<DHCP_Option> vendorops;

	c->Protocol.dhcp.AddOption(DHCP_Option(static_cast
		<unsigned char>(53), static_cast<unsigned char>
		(Netbootd::Network::OFFER)));

	switch (c->Protocol.dhcp.get_vendor())
	{
	case Netbootd::Network::PXEServer:
	case Netbootd::Network::PXEClient:
		if (c->Protocol.dhcp.rbcp.get_AllowBootMenue())
		{
			GenerateMulticastOption(ident, c, &vendorops);
			GenerateBootMenue(c, &vendorops);

			c->Protocol.dhcp.set_nextIP(server.LocalIP());

			if (!vendorops.empty())
			{
				vendorops.emplace_back(255);
				c->Protocol.dhcp.AddOption(DHCP_Option(43, vendorops));
			}
		}

		
		break;
	default:;
	}
}

EXPORT void Handle_DHCP_Request(const std::string& ident,
	Netbootd::Network::client* c, Netbootd::Network::DHCP_Packet* packet)
{
	std::vector<DHCP_Option> vendorops;
	char item[4];
	unsigned short bootitem = 0;
	unsigned short layer = 0;
	ClearBuffer(item, 4);

	c->Protocol.dhcp.AddOption(DHCP_Option(static_cast
		<unsigned char>(53), static_cast<unsigned char>
		(Netbootd::Network::ACK)));

	switch (c->Protocol.dhcp.get_vendor())
	{
	case  Netbootd::Network::PXEClient:
	case  Netbootd::Network::PXEServer:
		if (c->Protocol.dhcp.rbcp.get_AllowBootMenue())
		{
			bootitem = c->Protocol.dhcp.rbcp.get_item();
			layer = c->Protocol.dhcp.rbcp.get_layer();

			GenerateMulticastOption(ident, c, &vendorops);

			if (BS16(bootitem) != BS16(static_cast<unsigned short>(0)))
			{
				memcpy(&item[0], &bootitem, 2);
				memcpy(&item[2], &layer, 2);
				vendorops.emplace_back(71, 4, item);

				c->Protocol.dhcp.set_nextIP(server.LocalIP());
				c->Protocol.dhcp.set_servername(server.GetHostName());
				Select_Bootfile(c);
			}
		}
		else
		{
			Select_Bootfile(c);

			c->Protocol.dhcp.set_nextIP(server.LocalIP());
			c->Protocol.dhcp.AddOption(DHCP_Option(static_cast
				<unsigned char>(54), server.LocalIP()));
			c->Protocol.dhcp.set_servername(server.GetHostName());
		}

		if (!vendorops.empty())
		{
			vendorops.emplace_back(255);
			c->Protocol.dhcp.AddOption(DHCP_Option(43, vendorops));
		}
	default:;
	}
}

EXPORT void Select_Bootfile(Netbootd::Network::client* c)
{
	for (auto & a : c->Protocol.dhcp.arch)
	{
		switch (a)
		{
		case Netbootd::Network::INTEL_X86:
			c->Protocol.dhcp.set_filename("Boot/x86/wdsnbp.com");
			break;
		case Netbootd::Network::NEC_PC98:
			break;
		case Netbootd::Network::EFI_ITAN:
			break;
		case Netbootd::Network::DEC_ALPHA:
			break;
		case Netbootd::Network::ARC_X86:
			break;
		case Netbootd::Network::INTEL_LEAN:
			break;
		case Netbootd::Network::EFI_IA32:
			break;
		case Netbootd::Network::EFI_BC:
			c->Protocol.dhcp.set_filename("Boot/x64/efi/wdsmgfw.efi");
			break;
		case Netbootd::Network::EFI_XSCALE:
			break;
		case Netbootd::Network::EFI_X86X64:
			c->Protocol.dhcp.set_filename("Boot/x86/wdsnbp.com");
			break;
		}
	}
}

EXPORT void Handle_Request(const std::string& ident, const ServerMode serverMode,
	const ServiceType serviceType, const char* buffer, Netbootd::Network::client _client)
{
	char data[16385];
	auto pktsize = 0;
	ClearBuffer(data, sizeof 16385);
	Netbootd::Network::DHCP_Packet response;
	Netbootd::Network::client c;

	switch (serverMode)
	{
	case UDPMCAST:
	case UDP:
		server.AddClient(createIdent(buffer, serviceType, _client));
		c = server.GetClient(_client.ident);

		switch (serviceType)
		{
		case BOOTP:
		case DHCP:
			if (c.Protocol.dhcp.get_relayIP() != INADDR_ANY)
			{
				printf("[D] %s: Relay Agent: %s\n", __FUNCTION__,
					AddressStr(c.Protocol.dhcp.get_relayIP()));

				if (c.Protocol.dhcp.get_relayIP() != INADDR_ANY)
				{
					c.toAddr.sin_addr.s_addr = c.Protocol.dhcp.get_relayIP();
					c.toAddr.sin_port = serviceType == DHCP ? 67 : 4011;
				}
			}
			else
			{
				switch (c.Protocol.dhcp.get_vendor())
				{
				case Netbootd::Network::PXEClient:
				case Netbootd::Network::PXEServer:
					printf("Got PXE request from: %s\n", c.ident.c_str());
					break;
				default: return;
				}

				c.Protocol.dhcp.set_opcode(Netbootd::Network::BOOTREPLY);

				switch (c.Protocol.dhcp.get_vendor())
				{
				case Netbootd::Network::PXEServer:
					c.Protocol.dhcp.AddOption(DHCP_Option(static_cast
						<unsigned char>(60), 9, "PXEServer"));
					break;
				case Netbootd::Network::PXEClient:
					c.Protocol.dhcp.AddOption(DHCP_Option(static_cast
						<unsigned char>(60), 9, "PXEClient"));
					break;
				default:
					return;
				}

				switch (static_cast<Netbootd::Network::DHCPMSGTYPE>(
					c.Protocol.dhcp.options.at(53).Value[0]))
				{
				case Netbootd::Network::DISCOVER:
					Handle_DHCP_Discover(ident, &c, &response);
					break;
				case Netbootd::Network::REQUEST:
				case Netbootd::Network::INFORM:
					Handle_DHCP_Request(ident, &c, &response);
					break;
				case Netbootd::Network::RELEASE:
					server.RemoveClient(c.ident);
					break;
				default:
					return;;
				}

				c.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(255)));
			}

			response = Netbootd::Network::DHCP_Packet(c);
			pktsize = sizeof(response);

			memcpy(&data, &response, pktsize);
			pktsize = StripPacket(data, pktsize);
			
			server.Send(ident, c, data, pktsize);
			server.RemoveClient(c.ident);
			break;
		case TFTP:
			server.AddClient(createIdent(buffer, serviceType, _client));
			c = server.GetClient(_client.ident);

			printf("Got TFTP request from: %s\n", c.ident.c_str());
			break;
		default:;
		}
		break;
	default:;
	}
}

EXPORT void GenerateMulticastOption(const std::string& ident, Netbootd::Network::
	client* client, std::vector<DHCP_Option>* vendorOpts)
{
	vendorOpts->emplace_back(static_cast<unsigned char>(6),
		static_cast<unsigned char>(SETTINGS.DISCOVERY_MODE));

	client->Protocol.dhcp.rbcp.mcastIP = server.LocalMCASTIP(ident);

	vendorOpts->emplace_back(static_cast<unsigned char>(1),
		static_cast<unsigned int>(client->Protocol.dhcp.rbcp.mcastIP));

	vendorOpts->emplace_back(static_cast<unsigned char>(2),
		static_cast<unsigned short>(client->Protocol.dhcp.rbcp.mcast_port));
}

EXPORT void GenerateBootMenue(const Netbootd::Network::client* client,
	std::vector<DHCP_Option>* vendorOpts)
{
	std::vector<Netbootd::Network::BootMenuEntry> BootMenu;
	std::vector<Netbootd::Network::BootServerEntry> bootserver;

	BootMenu.emplace_back(static_cast<unsigned short>(BootMenu.size()),
		"Local Boot");

	BootMenu.emplace_back(static_cast<unsigned short>(BootMenu.size()),
		server.GetHostName(), server.LocalIP());

	BootMenu.emplace_back(static_cast<unsigned short>(BootMenu.size()),
		"FBS01.FBLIPKE.DE", inet_addr("10.20.0.1"));

	_SIZE_T offset = 0;
	unsigned short id = 0;
	char menubuffer[1024];
	char serverbuffer[1024];

	ClearBuffer(menubuffer, sizeof menubuffer);
	ClearBuffer(serverbuffer, sizeof serverbuffer);

	for (auto & entry : BootMenu)
	{
		unsigned short x = BS16(id++);
		memcpy(&menubuffer[offset], &x, 2);
		offset += 2;

		/* desc len */
		auto length = static_cast<unsigned char>(strlen(entry.Description.c_str()));
		memcpy(&menubuffer[offset], &length, sizeof(unsigned char));
		offset += 1;

		/* desc */
		memcpy(&menubuffer[offset], entry.Description.c_str(), length);
		offset += length;

		bootserver.emplace_back(x, entry.Address);
	}

	vendorOpts->emplace_back(9, offset, menubuffer);
	offset = 0;

	for (auto & entry : bootserver)
	{
		memcpy(&serverbuffer[offset], &entry.ident, sizeof(unsigned short));
		offset += sizeof(unsigned short);

		memcpy(&serverbuffer[offset], &entry.Type, sizeof(unsigned char));
		offset += sizeof(unsigned char);

		memcpy(&serverbuffer[offset], &entry.Addresses, sizeof(unsigned int));
		offset += sizeof(unsigned int);
	}

	vendorOpts->emplace_back(8, offset, serverbuffer);

	/* Menue prompt */
	char promptbuffer[512];
	ClearBuffer(menubuffer, sizeof promptbuffer);
	offset = 0;

	auto timeout = 0x0a;

	memcpy(&promptbuffer[offset], &timeout, sizeof(unsigned char));
	offset += 1;
	const std::string prompt = "Press [F8] to select a Server...";

	memcpy(&promptbuffer[offset], prompt.c_str(),
		static_cast<_SIZE_T>(prompt.size()));

	offset += static_cast<_SIZE_T>(prompt.size());

	vendorOpts->emplace_back(10, offset, promptbuffer);
}

EXPORT void NetBootd::Listen()
{
	if (server.Init() && server.Bind())
		server.Listen(&Handle_Request);
}

EXPORT INLINE void NetBootd::Close()
{
	if (!server.Close())
		printf("Failed to shutdown server!\n");
}

EXPORT NetBootd::~NetBootd()
{
#ifdef _WIN32
	Netbootd::Network::Server::close_winsock();
#endif
}
