#pragma once
#include "../../../Include.h"
#include <vector>
namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef enum BootServerType
		{
			PXEBootstrapServer = 0,
			WindowsNTBootServer = 1,
			IntelLCMBootServer = 2,
		} BootServerTyp;

		EXPORT typedef struct BootServerEntry
		{
			BootServerType Type;
			std::vector<unsigned int> Addresses;
		} BootServerEntry;

		EXPORT typedef struct BootMenuEntry
		{
			BootServerType Type = PXEBootstrapServer;
			std::string Description;

			BootMenuEntry(const std::string& text, const BootServerType type = PXEBootstrapServer)
			{
				Type = type;
				Description = text;
			}

			~BootMenuEntry()
			{

			}
		} BootMenuEntry;

		EXPORT typedef enum DHCPOPCODE
		{
			BOOTREQUEST = 0x01,
			BOOTREPLY = 0x02
		} DHCPOPCODE;

		EXPORT typedef enum DHCPMSGTYPE
		{
			DISCOVER = 0x01,
			OFFER = 0x02,
			REQUEST = 0x03,
			ACK = 0x05,
			INFORM = 0x08
		} DHCPMSGTYPE;

		EXPORT typedef enum DHCPPXEVENDOR
		{
			UNKNOWNNO = 0,
			PXEClient = 1,
			PXEServer = 2,
			APPLEBSDP = 3,
			NETBOOTDS = 4,
			NETBOOTDC = 5,
		} DHCPPXEVENDOR;

		EXPORT typedef enum EtherBootOption // iPXE, gPXE
		{
			Priority	= 0x01,
			KeepSan		= 0x08,
			NoPXEDhcp	= 0xB0,
			BusId		= 0xB1,
			BiosDrive	= 0xBD,
			Username	= 0xBE,
			Password	= 0xBF,
			rUsername	= 0xC0,
			rpassword	= 0xC1,
			Version		= 0xEB
		} EtherBootOption;

		EXPORT typedef enum DHCPFLAGS
		{
			Unicast = 0x0000,
			Broadcast = 0x8000
		} DHCPFLAGS;

		EXPORT typedef enum DHCPHARDWARETYPE
		{
			Ethernet = 0x01
		} DHCPHARDWARETYPE;
	}
}
