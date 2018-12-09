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
#include "Server.h"
#include "Endpoint.h"
#include "../Client/Client.h"

namespace Netbootd
{
	namespace Network
	{
#ifdef _WIN32
		EXPORT INLINE bool Server::init_winsock()
		{
			WSADATA wsa;
			return WSAStartup(MAKEWORD(2, 0), &wsa) == 0;
		}

		EXPORT INLINE bool Server::close_winsock()
		{
			return WSACleanup() == 0;
		}
#endif
		EXPORT Server::Server()
		{
		}

		EXPORT Server::Server(ServerMode serverMode,
			ServiceType serviceType, std::string ident)
		{
			Add(serverMode, serviceType, ident);
		}

		EXPORT Server::~Server()
			= default;

		EXPORT bool Server::Add(ServerMode serverMode,
			ServiceType serviceType, std::string ident)
		{
			if (this->HasEndpoint(ident))
				return false;

			this->endpoints.insert(std::pair<std::string, Endpoint>
				(ident, Endpoint(serverMode, serviceType, ident)));

			return this->HasEndpoint(ident);
		}

		EXPORT void Server::Remove(std::string ident)
		{
			if (!this->HasEndpoint(ident))
				return;

			this->endpoints.at(ident)
				.RequestClose();
		}

		EXPORT INLINE bool Server::IsListening() const
		{
			return this->listening;
		}

		EXPORT bool Server::Close()
		{
			this->listening = false;
			auto retval = 0;

			if (!this->HasEndpoints())
				return true;

			for (auto & endpoint : this->endpoints)
				retval += endpoint.second.Close();

			this->endpoints.clear();

			return retval == 0;
		}

		EXPORT bool Server::Init(long sec, long usec)
		{
			auto retval = 0;
			this->timeout.tv_sec = sec;
			this->timeout.tv_usec = usec;

			for (auto & endpoint : this->endpoints)
				retval += endpoint.second.Init();

			return retval == 0;
		}

		EXPORT bool Server::Bind()
		{
			auto retval = 0;

			if (!this->HasEndpoints())
				return false;

			for (auto & endpoint : this->endpoints)
			{
				const auto result = endpoint.second.Bind();
				if (result == SOCKET_ERROR)
					printf("[E] BIND: (%s): Failed!\n",
						endpoint.second.GetIdent().c_str());
				else
					printf("[I] BIND: (%s): Ok!\n",
						endpoint.second.GetIdent().c_str());
				retval += result;
			}

			char hname[64];
			ClearBuffer(&hname, sizeof(hname));

			gethostname(hname, sizeof(hname));
			this->hName = std::string(hname);

			this->listening = (retval == 0);

			return this->IsListening();
		}

		EXPORT in_addr Server::LocalIP() const
		{
			const auto hostentry = gethostbyname(this->hName.c_str())->h_addr_list;
			return **reinterpret_cast<struct in_addr**>(hostentry);
		}

		EXPORT void Server::Listen(void(*ListenCallBack)(ServerMode, ServiceType, client))
		{
			while (this->IsListening())
			{
				FD_ZERO(&this->fd_read);
				FD_ZERO(&this->fd_except);

				for (auto & endpoint : this->endpoints)
				{
					if (endpoint.second.GetSocket() == INVALID_SOCKET)
						continue;

					if (endpoint.second.CloseRequested())
					{
						endpoint.second.Close();
						continue;
					}

					FD_SET(endpoint.second.GetSocket(), &this->fd_read);
					FD_SET(endpoint.second.GetSocket(), &this->fd_except);

					const auto state = select((int)endpoint.second.GetSocket(),
						&this->fd_read, &this->fd_write, &this->fd_except,
						&this->timeout);

					if (state == SOCKET_ERROR)
					{
						printf("[E] %s: Select (Socket) failed!\n", __FUNCTION__);
						continue;
					}

					if (FD_ISSET(endpoint.second
						.GetSocket(), &this->fd_write))
						continue;

					if (FD_ISSET(endpoint.second.GetSocket(), &this->fd_read))
					{
						FD_CLR(endpoint.second.GetSocket(), &this->fd_read);

						// Call and detach the Listen thread. 
						std::thread __listen_delegated(__Listen,
							endpoint.second.GetMode(),
							endpoint.second.GetType(),
							endpoint.second.GetIdent(),
							endpoint.second.GetSocket(),
							endpoint.second.GetFlags(),
							endpoint.second.GetLogs(),
							ListenCallBack);

						__listen_delegated.detach();
					}

					if (FD_ISSET(endpoint.second.GetSocket(), &this->fd_except))
					{
						FD_CLR(endpoint.second.GetSocket(), &this->fd_except);
						printf("[E] %s: Socket Error!\n", __FUNCTION__);
					}
				}
			}
		}

		EXPORT Endpoint Server::GetEndpoint(std::string id)
		{
			return this->endpoints.at(id);
		}

		EXPORT void Server::Update()
		{
			if (!this->HasEndpoints())
				return;

			for (auto & endpoint : this->endpoints)
				endpoint.second.Update();
		}

		EXPORT std::string Server::GetHostName() const
		{
			return this->hName;
		}

		EXPORT INLINE bool Server::HasEndpoint(std::string ident)
		{
			return this->endpoints.find(ident)
				!= this->endpoints.end();
		}

		EXPORT INLINE bool Server::HasEndpoints() const
		{
			return !this->endpoints.empty();
		}

		EXPORT int Server::Send(client client, const char* buffer, int length)
		{
			const auto retval = SOCKET_ERROR;

			if (!this->HasEndpoint(client.ident))
				return retval;

			if (!this->IsListening() || this->GetEndpoint(client.ident)
				.CloseRequested())
				return retval;

			if (length != 0)
			{
				FD_ZERO(&this->fd_write);
				FD_SET(this->GetEndpoint(client.ident).GetSocket(), &this->fd_write);

				if (!FD_ISSET(this->GetEndpoint(client.ident).GetSocket(), &this->fd_write))
					return 0;

				std::thread __sendthread(__Send,
					this->GetEndpoint(client.ident).GetMode(),
					this->GetEndpoint(client.ident).GetType(),
					this->GetEndpoint(client.ident).GetSocket(),
					client, buffer, length);

				__sendthread.join();

				FD_CLR(this->GetEndpoint(client.ident).GetSocket(), &this->fd_write);

			}

			return retval;
		}
	}
}
