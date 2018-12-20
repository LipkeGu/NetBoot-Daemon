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
#include "Endpoint.h"
#include "../Client/Client.h"

namespace Netbootd
{
	namespace Network
	{
		EXPORT Endpoint::Endpoint()
		{
		}

		EXPORT Endpoint::~Endpoint()
		{
		}

		EXPORT Endpoint::Endpoint(unsigned int addr, const ServerMode serverMode,
			const ServiceType serviceType, const std::string& ident)
		{
			this->mode = serverMode;
			this->ident = ident;
			this->serviceType = serviceType;
			this->address = INADDR_ANY;
			this->mcastgroup = SETTINGS.MTFTP_ADDR;
			switch (this->GetMode())
			{
			case UDPMCAST:
			case UDP:
				this->af = AF_INET;
				this->type = SOCK_DGRAM;
				this->proto = this->GetMode() != UDPMCAST ? IPPROTO_UDP : 0;

				switch (this->serviceType)
				{
				case DHCP:
					this->_port = 67;
					break;
				case BOOTP:
					this->_port = 4011;
					break;
				case TFTP:
					this->_port = 69;
					break;
				default:;
				}
				break;
			case TCP:
				this->backlog = 5;
				this->af = AF_INET;
				this->type = SOCK_STREAM;
				this->proto = IPPROTO_TCP;
				this->_port = 8900;
				break;
			default:;
			}

			this->closeReq = false;
			this->_socket = INVALID_SOCKET;
			this->flags = 0;

			ClearBuffer(&this->local, sizeof this->local);
		}

		EXPORT int Endpoint::Init(const int multicast, const int reuseaddr, const int broadcast,
			const int keepalive, const int backlog)
		{
			auto retval = SOCKET_ERROR;
			this->_socket = socket(this->af, this->type, this->proto);
			this->backlog = backlog;
			this->multicast = multicast;

			if (this->_socket == INVALID_SOCKET)
				return SOCKET_ERROR;

			auto mode = this->GetMode();
			switch (mode)
			{
			case UDPMCAST:
			case UDP:
				retval = setsockopt(this->_socket, SOL_SOCKET,
					SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(int));

				if (retval == SOCKET_ERROR)
					printf("[E] %s: setsockopt (BROADCAST) failed! (%d)\n",
						__FUNCTION__, WSAGetLastError());

				retval = setsockopt(this->_socket, SOL_SOCKET,
					SO_REUSEADDR, reinterpret_cast<const char*>(&reuseaddr), sizeof(int));

				if (retval == SOCKET_ERROR)
					printf("[E] %s: setsockopt (REUSEADDR) failed! (%d)\n",
						__FUNCTION__, WSAGetLastError());

				ClearBuffer(&this->local, sizeof this->local);

				this->local.sin_addr.s_addr = this->address;
				this->local.sin_family = this->af;
				this->local.sin_port = htons(this->_port);
				break;
			case TCP:
				retval = setsockopt(this->_socket, SOL_SOCKET,
					SO_KEEPALIVE, reinterpret_cast<const char*>(&keepalive), sizeof keepalive);

				if (retval == SOCKET_ERROR)
					printf("[E] %s: setsockopt (KEEPALIVE) failed! (%d)\n"
						, __FUNCTION__, WSAGetLastError());

				ClearBuffer(&this->local, sizeof this->local);

				this->local.sin_addr.s_addr = htonl(this->address);
				this->local.sin_family = this->af;
				this->local.sin_port = htons(this->_port);
				break;
			default:;
			}

			return retval;
		}

		EXPORT int Endpoint::Bind()
		{
			auto retval = SOCKET_ERROR;

			retval = bind(this->_socket,
				reinterpret_cast<struct sockaddr*>(&this->local),
				sizeof this->local);

			if (retval == SOCKET_ERROR)
				printf("[E] %s: Bind failed! (%d)\n", __FUNCTION__,
					WSAGetLastError());

			if (this->GetMode() == UDPMCAST && multicast == 1)
			{
				this->mreq.imr_multiaddr.s_addr = inet_addr(this->mcastgroup.c_str());
				this->mreq.imr_interface.s_addr = this->address;

				retval = setsockopt(this->_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
					reinterpret_cast<char*>(&this->mreq), sizeof this->mreq);

				if (retval == SOCKET_ERROR)
					printf("[E] %s: setsockopt (IP_ADD_MEMBERSHIP) failed! (%d)\n"
						, __FUNCTION__, WSAGetLastError());
			}

			return retval;
		}

		EXPORT INLINE void Endpoint::Update()
		{
			if (this->CloseRequested())
				this->Close();
		}

		EXPORT INLINE unsigned int Endpoint::MulticastGroup() const
		{
			return inet_addr(this->mcastgroup.c_str());
		}

		EXPORT int Endpoint::Close()
		{
			this->RequestClose();
			return this->Close(this->_socket);
		}

		EXPORT int Endpoint::Close(const _SOCKET socket)
		{
			if (this->GetMode() == UDPMCAST && multicast == 1)
			{
				this->mreq.imr_multiaddr.s_addr = inet_addr(this->mcastgroup.c_str());
				this->mreq.imr_interface.s_addr = this->address;

				setsockopt(this->_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
					reinterpret_cast<char*>(&this->mreq), sizeof this->mreq);
			}

			const int retval = _close(this->_socket);

			if (retval == SOCKET_ERROR)
				printf("[E] %s: Close failed! (%d)\n",
					__FUNCTION__, WSAGetLastError());

			if (retval == 0)
				printf("[I] %s: Socket closed!\n",
					__FUNCTION__);

			return retval;
		}

		EXPORT void __Send(const ServerMode serverMode, const ServiceType serviceType,
			const _SOCKET __socket, client client, const char* buffer,
			_SIZE_T length)
		{
			auto retval = SOCKET_ERROR;
			_SIZE_T bs = 0;
			switch (serverMode)
			{
			case TCP:
				do
				{
					bs += _write(__socket, &buffer[bs], static_cast<int>(length), 0);
				} while (bs < length);
				break;
			case UDPMCAST:
			case UDP:
				retval = sendto(__socket, buffer, static_cast<int>(length), 0,
					reinterpret_cast<struct sockaddr*>
					(&client.toAddr), sizeof client.toAddr);

				if (retval == SOCKET_ERROR)
					printf("[E] %s: Send failed!\n", __FUNCTION__);
				break;
			default:;
			}
		}

		EXPORT void Endpoint::RequestClose()
		{
			this->closeReq = true;
		}

		EXPORT bool Endpoint::CloseRequested() const
		{
			return this->closeReq;
		}

		EXPORT _SOCKET Endpoint::GetSocket() const
		{
			return this->_socket;
		}

		EXPORT std::string Endpoint::LocalIP() const
		{
			return std::string(inet_ntoa(local.sin_addr));
		}

		EXPORT std::string Endpoint::GetIdent() const
		{
			return this->ident;
		}

		EXPORT int Endpoint::GetFlags() const
		{
			return this->flags;
		}

		EXPORT void Endpoint::SetFlags(int flags)
		{
			this->flags = flags;
		}

		EXPORT void __Listen(const ServerMode serverMode, const ServiceType serviceType,
			const std::string& ident, const _SOCKET _socket, const int flags,
			const int backlog, void(*ListenCallBack)(const std::string&, const ServerMode, const ServiceType, const char*, _SIZE_T, client))
		{
			char buffer[16385];
			ClearBuffer(&buffer, sizeof buffer);

			sockaddr_in remote{};
			ClearBuffer(&remote, sizeof remote);


			int rlen = sizeof remote;
			auto retval = SOCKET_ERROR;
			_SOCKET s;

			switch (serverMode)
			{
			case TCP:
				retval = listen(_socket, backlog);

				if (retval == SOCKET_ERROR)
				{
					printf("[E] %s: Listen failed! (%d)\n", __FUNCTION__, WSAGetLastError());
					return;
				}

				s = _SOCKET(static_cast<_SOCKET>(accept(_socket,
					reinterpret_cast<struct sockaddr *>(&remote), &rlen)));

				if (!s)
				{
					retval = _close(s);
					if (retval == SOCKET_ERROR)
					{
						printf("[E] %s: Close failed! (%d)\n", __FUNCTION__, WSAGetLastError());
						return;
					}

					return;
				}
#ifdef _WIN32
				retval = _read(s, buffer, sizeof buffer, flags);
#else
				retval = _read(s, buffer, sizeof(buffer));
#endif
				if (retval == SOCKET_ERROR)
				{
					printf("[E] %s: Closing socket! (%d)\n", __FUNCTION__, WSAGetLastError());
					return;
				}
				break;
			case UDPMCAST:
			case UDP:
				retval = recvfrom(_socket, buffer, sizeof(buffer),
					flags, reinterpret_cast<struct sockaddr*>(&remote), &rlen);

				if (retval == SOCKET_ERROR)
				{
					printf("[E] %s: Receive failed! (%d)\n", __FUNCTION__, WSAGetLastError());
					return;
				}
			default: ;
			}

			if (ListenCallBack != nullptr)
				ListenCallBack(ident, serverMode, serviceType,
					buffer, retval, client(serviceType, "", remote, buffer, retval));
		}

		EXPORT int Endpoint::GetLogs() const
		{
			return this->backlog;
		}

		EXPORT ServerMode Endpoint::GetMode() const
		{
			return this->mode;
		}

		EXPORT ServiceType Endpoint::GetType() const
		{
			return this->serviceType;
		}
	}
}
