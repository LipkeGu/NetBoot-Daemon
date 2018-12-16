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

		EXPORT Server::Server(const ServerMode serverMode,
			const ServiceType serviceType, const std::string& ident)
		{
			AddEndpoint(serverMode, serviceType, ident);
		}

		EXPORT Server::~Server()
			= default;

		EXPORT bool Server::AddEndpoint(const ServerMode serverMode,
			const ServiceType serviceType, const std::string& ident)
		{
			if (this->HasEndpoint(ident))
				return false;

			this->endpoints.insert(std::pair<std::string, Endpoint>
				(ident, Endpoint(this->LocalIP(), serverMode, serviceType, ident)));

			return this->HasEndpoint(ident);
		}

		EXPORT bool Server::AddClient(client c)
		{
			if (this->HasClient(c.ident))
				return false;

			this->clients.insert(std::pair<std::string, client>(c.ident, c));

			return this->HasClient(c.ident);
		}

		EXPORT void Server::RemoveEndpoint(const std::string& ident)
		{
			if (!this->HasEndpoint(ident))
				return;

			this->endpoints.at(ident)
				.RequestClose();
		}

		EXPORT void Server::RemoveClient(const std::string& ident)
		{
			if (!this->HasClient(ident))
				return;

			this->clients.erase(ident);
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

		EXPORT bool Server::MulticastEnabled() const
		{
			return this->multicast;
		}

		EXPORT bool Server::Init(const bool multicast,
			const long sec, const long usec)
		{
			auto retval = 0;
			this->multicast = multicast;
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
					printf("[E] Bind: (%s): Failed!\n",
						endpoint.second.GetIdent().c_str());
				else
					printf("[I] Bind: (%s): Ok!\n",
						endpoint.second.GetIdent().c_str());
				retval += result;
			}

			char hname[64];
			ClearBuffer(&hname, sizeof hname);

			gethostname(hname, sizeof hname);
			this->hName = std::string(hname);
			this->listening = (retval == 0);

			return this->IsListening();
		}

		EXPORT unsigned int Server::LocalIP() const
		{
			const auto hostentry = gethostbyname(this->hName.c_str())->h_addr_list;
			return (**reinterpret_cast<struct in_addr**>(hostentry)).s_addr;
		}

		EXPORT unsigned int Server::LocalMCASTIP(const std::string& ident) const
		{
			return GetEndpoint(ident)
			.MulticastGroup();
		}

		EXPORT void Server::Listen(void(*ListenCallBack)
			(const std::string&, const ServerMode, const ServiceType, const char*, client))
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

					const auto state = select(static_cast<int>(endpoint.second.GetSocket()),
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

		EXPORT INLINE Endpoint Server::GetEndpoint(const std::string& id) const
		{
			return this->endpoints.at(id);
		}

		EXPORT INLINE client& Server::GetClient(const std::string& id)
		{
			return this->clients.at(id);
		}

		EXPORT void Server::Update()
		{
			if (this->HasEndpoints())
				for (auto & endpoint : this->endpoints)
					endpoint.second.Update();

			if (this->HasClients())
				for (auto & client : this->clients)
					client.second.Update();
		}

		EXPORT std::string Server::GetHostName() const
		{
			return this->hName;
		}

		EXPORT INLINE bool Server::HasEndpoint(const std::string& ident) const
		{
			return this->endpoints.find(ident)
				!= this->endpoints.end();
		}

		EXPORT INLINE bool Server::HasClient(const std::string& ident) const
		{
			return this->clients.find(ident)
				!= this->clients.end();
		}

		EXPORT INLINE bool Server::HasEndpoints() const
		{
			return !this->endpoints.empty();
		}

		EXPORT INLINE bool Server::HasClients() const
		{
			return !this->clients.empty();
		}

		EXPORT int Server::Send(const std::string& ident,
			client& client, const char* buffer, int length)
		{
			const auto retval = SOCKET_ERROR;

			if (!this->HasEndpoint(ident))
				return retval;

			if (!this->IsListening() || this->GetEndpoint(ident)
				.CloseRequested())
				return retval;

			if (length != 0)
			{
				FD_ZERO(&this->fd_write);
				FD_SET(this->GetEndpoint(ident).GetSocket(), &this->fd_write);

				if (!FD_ISSET(this->GetEndpoint(ident)
					.GetSocket(), &this->fd_write))
					return 0;

				std::thread __sendthread(__Send,
					this->GetEndpoint(ident).GetMode(),
					this->GetEndpoint(ident).GetType(),
					this->GetEndpoint(ident).GetSocket(),
					client, buffer, length);

				__sendthread.join();

				FD_CLR(this->GetEndpoint(ident)
					.GetSocket(), &this->fd_write);
			}

			return retval;
		}
	}
}
