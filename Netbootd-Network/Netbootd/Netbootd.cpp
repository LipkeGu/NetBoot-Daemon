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
EXPORT void Handle_Request(const ServerMode serverMode,
	const ServiceType serviceType, Netbootd::Network::client client);

EXPORT NetBootd::NetBootd()
= default;

EXPORT void NetBootd::Init()
{
	printf("Network Boot daemon 0.4 (BETA)\nStarting Storage subsystem...\n");

	fs.Init();

#ifdef _WIN32
	if (!Netbootd::Network::Server::init_winsock())
		return;
#endif
	server.Add(UDP,
		DHCP, "DHCP-Proxy");

	server.Add(UDP,
		BOOTP, "DHCP-Boot");
}

EXPORT int StripPacket(const char* buffer, int buflen)
{
	for (auto i = buflen; buflen > 240; i = i - 1)
		if (static_cast<unsigned char>(buffer[i])
			== static_cast<unsigned char>(0xff))
			return i + 1;

	return buflen;
}

EXPORT const char* AddressStr(unsigned ip)
{
	struct in_addr addr {};
	ClearBuffer(&addr, sizeof(addr));

	addr.s_addr = ip;
	return inet_ntoa(addr);
}

EXPORT void Handle_Request(const ServerMode serverMode,
	const ServiceType serviceType, Netbootd::Network::client client)
{
	char data[16385];
	int pktsize = 0;
	ClearBuffer(data, sizeof(16385));
	Netbootd::Network::DHCP_Packet response;
	
	std::vector<DHCP_Option> vendorops;

	auto discovery_enc_opt = DHCP_Option(static_cast<unsigned char>(6),
		static_cast<unsigned int>(3));

	switch (serverMode)
	{
	case UDP:
		switch (serviceType)
		{
		case BOOTP:
		case DHCP:
			if (!client.Protocol.dhcp.HasOption(60))
				return;

			if (memcmp(&client.Protocol.dhcp.options.at(60).Value,
				"PXEClient", strlen("PXEClient")) != 0)
				return;

			client.Protocol.dhcp.RemoveOption(55);
			if (client.Protocol.dhcp.get_relayIP() != INADDR_ANY)
			{
				if (client.Protocol.dhcp.get_nextIP()
					== client.Protocol.dhcp.get_relayIP())
					return;

				printf("[D] %s: Relay Agent: %s\n", __FUNCTION__,
					AddressStr(client.Protocol.dhcp.get_relayIP()));
			}

			client.Protocol.dhcp.set_opcode(Netbootd::Network::BOOTREPLY);
			client.Protocol.dhcp.set_nextIP(server.LocalIP().s_addr);

			
			client.Protocol.dhcp.set_servername(server.GetHostName());

			// Set the Relayagent adress as response address...
			if (client.Protocol.dhcp.get_relayIP() != INADDR_ANY)
				client.toAddr.sin_addr.s_addr = client.Protocol.dhcp.get_relayIP();

			client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
				<unsigned char>(54), server.LocalIP().s_addr));

			switch (client.Protocol.dhcp.get_vendor())
			{
			case Netbootd::Network::PXEServer:
				client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(60), 9, "PXEServer"));
				break;
			case Netbootd::Network::PXEClient:
				client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(60), 9, "PXEClient"));

				if (client.Protocol.dhcp.IsEtherBootClient())
					printf("[I] Client reports EtherBoot capabilities!\n");

				client.Protocol.dhcp.set_filename("Boot/x86/wdsnbp.com");
				break;
			case Netbootd::Network::APPLEBSDP:
				client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(60), 9, "APPLEBSDP"));
				break;
			default:
				return;
			}

			switch (static_cast<Netbootd::Network::DHCPMSGTYPE>(client.Protocol.
				dhcp.options.at(53).Value[0]))
			{
			case Netbootd::Network::DISCOVER:
				client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(53), static_cast<unsigned char>
						(Netbootd::Network::OFFER)));

				switch (client.Protocol.dhcp.get_vendor())
				{
				case Netbootd::Network::PXEClient:
					vendorops.emplace_back(static_cast<unsigned char>(6),
						static_cast<unsigned char>(3));
					break;
				default: ;
				}
			break;
			case Netbootd::Network::REQUEST:
			case Netbootd::Network::INFORM:
				client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
					<unsigned char>(53), static_cast<unsigned char>
						(Netbootd::Network::ACK)));
				break;
			default:
				return;
			}

			if (!vendorops.empty())
				client.Protocol.dhcp.AddOption(DHCP_Option(43, vendorops));
			
			client.Protocol.dhcp.AddOption(DHCP_Option(static_cast
				<unsigned char>(255)));

			response = Netbootd::Network::DHCP_Packet(client);
			pktsize = sizeof(response);

			memcpy(&data, &response, pktsize);
			pktsize = StripPacket(data, pktsize);

			server.Send(client, data, pktsize);
			break;
		default:;
		}
		break;
	default:;
	}
}

EXPORT DHCP_Option GenerateBootMenue(const Netbootd::Network::client& client,
	std::vector<DHCP_Option> vendorOpts)
{
	std::vector<Netbootd::Network::BootMenuEntry> BootMenu;
	BootMenu.emplace_back("Local Boot");
	BootMenu.emplace_back("WDS Server (VPN)");
	BootMenu.emplace_back("Fritzbox");


	int offset = 0;

	for (auto & entry : BootMenu)
	{
		
	}
}

EXPORT void NetBootd::Listen() const
{
	if (server.Init() && server.Bind())
		server.Listen(&Handle_Request);
}

EXPORT INLINE void NetBootd::Close() const
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
