#pragma once
#include "../../Netbootd-Network/Include.h"
#pragma comment(lib, "Advapi32.lib")
namespace Netbootd
{
	namespace System
	{
		class Filesystem
		{
		public:
			EXPORT Filesystem();
			EXPORT Filesystem(const std::string& curDir);
			EXPORT ~Filesystem();

			EXPORT bool Init();

			EXPORT void Write(const std::string& path, const char* buffer, const int offset, const int count);
			EXPORT void Read(const std::string& path, char* buffer, const int offset, const int count);
			EXPORT std::string ResolvePath(const std::string& path);
			EXPORT bool CreateDir(const std::string& path);
			EXPORT std::string GetCurDir();
		private:
			std::string workingDir;
		};
	}
}
