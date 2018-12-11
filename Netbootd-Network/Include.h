#pragma once
#include <thread>
#include <map>
#include <string>
#include <direct.h>
#include <vector>
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#define INLINE __inline

#define _read(s,b,l,f) recv(s,b,l,f);
#define _write(s,b,l,f) send(s,b,l,f);
#define _close(s) closesocket(s);
#define ClearBuffer(x, y) memset(x, 0, y);

#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN64
typedef unsigned long long _SOCKET;
#else
typedef unsigned int	_SOCKET;
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
