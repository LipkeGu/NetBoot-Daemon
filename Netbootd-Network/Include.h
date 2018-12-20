#pragma once
#include <thread>
#include <map>
#include <string>
#include <direct.h>
#include <vector>
#include <strstream>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#define INLINE __inline

#define _read(s,b,l,f) recv(s,b,l,f);
#define _write(s,b,l,f) send(s,b,l,f);
#define _close(s) closesocket(s);
#define ClearBuffer(x, y) memset(x, 0, y);

#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN64
typedef unsigned long long _SOCKET;
typedef unsigned long long _SIZE_T;
#else
typedef unsigned int	_SOCKET;
typedef unsigned int	_SIZE_T;
#endif
#else
#define INLINE inline
typedef	_SOCKET			unsigned int;
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

static struct
{
	unsigned char DISCOVERY_MODE = 3;
	unsigned char PXEBOOTMENUE = 1;
	unsigned char PXEPROMPTTIMEOUT = 255;

	unsigned char PXE_MTFTP_TIMEOUT = 1;
	unsigned char PXE_MTFTP_DELAY = 10;

	std::string PXEPROMP = "Press [F8] to boot from network...";
	std::string MTFTP_ADDR = "224.20.0.1";

	unsigned short MTFTP_PORT = 69;
} SETTINGS;

EXPORT typedef enum ServerMode
{
	TCP = 0,
	TCP6 = 1,
	UDP = 2,
	UDP6 = 3,
	UDPMCAST = 4,
	UNKNOWN = 5
} ServerMode;

EXPORT typedef enum ServiceType
{
	DHCP = 0,
	BOOTP = 1,
	TFTP = 2,
	DHCP6 = 3,
	BOOTP6 = 4,
	TFTP6 = 5
} ServiceType;

typedef EXPORT struct DHCP_Option
{
	EXPORT DHCP_Option()
	{
		memset(this, 0, sizeof(this));
	};
	
	
	EXPORT DHCP_Option(const unsigned char opt, const unsigned char length, const void* value)
	{
		Option = opt;
		Length = length;
		memset(Value, 0, 1024);

		if (length != 0)
			memcpy(&Value, value, Length);
	}

	EXPORT DHCP_Option(const unsigned char opt, const unsigned char value)
	{
		Option = opt;
		Length = 1;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(Value, &value, Length);
	}

	EXPORT DHCP_Option(const unsigned char opt, const unsigned short value)
	{
		Option = opt;
		Length = 2;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	EXPORT DHCP_Option(const unsigned char opt, const unsigned int value)
	{
		Option = opt;
		Length = 4;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	EXPORT DHCP_Option(const unsigned char opt, const unsigned long value)
	{
		Option = opt;
		Length = 4;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	EXPORT DHCP_Option(const unsigned char opt, const std::vector<DHCP_Option> value)
	{
		Option = opt;
		Length = 0;
		
		auto offset = 0;

		// Get the entire options length!
		for (const auto & option : value)
			Length += option.Length + 2;

		memset(&Value, 0, Length);
		
		for (const auto & option : value)
		{
			memcpy(&Value[offset], &option.Option, 1);
			offset += 1;
			
			memcpy(&Value[offset], &option.Length, 1);
			offset += 1;

			memcpy(&Value[offset], &option.Value, option.Length);
			offset += option.Length;
		}

		Length = offset;
	}

	EXPORT DHCP_Option(const unsigned char opt)
	{
		Option = opt;
		Length = 0;
		memset(&Value, 0, 1024);
	}

	unsigned char Option;
	unsigned char Length;

	char Value[1024];
} DHCP_Option;

EXPORT const std::string replace(std::string& str,
	const std::string& from, const std::string& to);

EXPORT const char* AddressStr(const unsigned int ip);

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN						1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN						4321
#endif

#ifndef __BYTE_ORDER
#if defined(_BIG_ENDIAN)
#define __BYTE_ORDER __BIG_ENDIAN
#elif defined(_LITTLE_ENDIAN)
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BS32(x) x
#define BS16(x) x
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BS16(x) (((uint16_t)(x) >> 8) | (((uint16_t)(x) & 0xff) << 8))
#define BS32(x) (((uint32_t)(x) >> 24) | (((uint32_t)(x) >> 8) & 0xff00) | \
				(((uint32_t)(x) << 8) & 0xff0000) | ((uint32_t)(x) << 24))
#else
#define BS16(x) (((uint16_t)(x) >> 8) | (((uint16_t)(x) & 0xff) << 8))
#define BS32(x) (((uint32_t)(x) >> 24) | (((uint32_t)(x) >> 8) & 0xff00) | \
				(((uint32_t)(x) << 8) & 0xff0000) | ((uint32_t)(x) << 24))
#endif
