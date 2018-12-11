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
#include "../Client/Client.h"
#include "Endpoint.h"

namespace Netbootd
{
	namespace Network
	{
		class Server
		{
		public:
			EXPORT Server();

			EXPORT Server(const ServerMode serverMode,
				const ServiceType serviceType, const std::string& ident);

			EXPORT ~Server();

#ifdef _WIN32
			EXPORT static bool init_winsock();
			EXPORT static bool close_winsock();
#endif
			EXPORT INLINE bool IsListening() const;
			EXPORT bool Add(const ServerMode serverMode,
				const ServiceType serviceType, const std::string& ident);

			EXPORT bool HasEndpoint(const std::string& ident);
			EXPORT INLINE bool HasEndpoints() const;
			EXPORT bool Init(long sec = 0, long usec = 2);
			EXPORT bool Bind();
			EXPORT bool Close();
			EXPORT in_addr LocalIP() const;
			EXPORT void Update();
			EXPORT void Remove(const std::string& ident);
			EXPORT void Listen(void(*ListenCallBack)
				(const ServerMode, const ServiceType, client));

			EXPORT int Send(client& client, const char* buffer, int length);

			EXPORT INLINE std::string GetHostName() const;
			EXPORT INLINE Endpoint GetEndpoint(const std::string& id);
		private:

			std::map<std::string, Endpoint> endpoints;
			std::string hName;

			bool listening;
			timeval timeout;
			ServiceType serviceType;
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_except;
		};

		EXPORT void __Send(const ServerMode serverMode, const ServiceType serviceType,
			const _SOCKET __socket, client client, const char* buffer, int length);

		EXPORT void __Listen(const ServerMode serverMode, const ServiceType serviceType,
			const std::string ident, const _SOCKET _socket, const int flags,
			const int backlog, void(*ListenCallBack)(const ServerMode, const ServiceType, client));
	}
}
