#pragma once
#include "../../../Include.h"
#include <vector>
namespace Netbootd
{
	namespace Network
	{
		EXPORT typedef enum BootServerType
		{
			PXEBootstrapServer = 0x00,
			WindowsNTBootServer = 0x01,
			IntelLCMBootServer = 0x02,
		} BootServerType;

		EXPORT typedef struct BootServerEntry
		{
			unsigned int Addresses;
			unsigned short ident;
			unsigned char Type;

			BootServerEntry() {}
			BootServerEntry(const unsigned short id, unsigned int adresses)
			{
				ident = id;
				Addresses = adresses;
				Type = 1;
			}

			~BootServerEntry()
			{
			}

		} BootServerEntry;

		EXPORT typedef struct BootMenuEntry
		{
			BootServerType Type = PXEBootstrapServer;
			std::string Description;
			unsigned short Ident;
			unsigned char DescLength = 0;
			BootMenuEntry(const unsigned short id, const std::string& text, const BootServerType type = PXEBootstrapServer)
			{;
				Ident = id;
				Type = type;
				Description = text;
				DescLength = Description.size();
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
