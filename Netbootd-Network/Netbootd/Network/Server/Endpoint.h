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

namespace Netbootd
{
	namespace Network
	{
		class Endpoint
		{
		public:
			EXPORT Endpoint();;
			EXPORT Endpoint(ServerMode serverMode, ServiceType serviceType, std::string ident);

			EXPORT ~Endpoint();;

			EXPORT int Init(int reuseaddr = 1, int broadcast = 1, int keepalive = 1, int backlog = 5);
			EXPORT int Bind();
			EXPORT int Close();
			EXPORT int Close(_SOCKET socket) const;
			EXPORT void RequestClose();
			EXPORT bool CloseRequested() const;
			EXPORT INLINE void Update();

			EXPORT std::string GetIdent() const;
			EXPORT int GetFlags() const;
			EXPORT int GetLogs() const;
			EXPORT ServerMode GetMode() const;
			EXPORT ServiceType GetType() const;
			EXPORT void SetFlags(int flags);
			EXPORT _SOCKET GetSocket() const;

			EXPORT std::string LocalIP() const;
		private:
			int broadcast;
			int reuseaddr;
			_SOCKET _socket;

			int af;
			int type;
			int flags;
			int proto;
			int backlog;

			std::string mcastgroup;

			std::string ident;
			sockaddr_in local;
			unsigned short _port;
			ServerMode mode;
			ServiceType serviceType;
			unsigned int address;
			bool closeReq;
		};
	}
}