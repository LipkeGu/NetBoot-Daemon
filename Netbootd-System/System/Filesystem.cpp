#include "Filesystem.h"

namespace Netbootd
{
	namespace System
	{
		EXPORT Filesystem::Filesystem()	= default;


		EXPORT Filesystem::Filesystem(const std::string& curDir)
		{
			this->workingDir = GetCurDir();
		}

		EXPORT bool Filesystem::Init()
		{
			CreateDir("tftp_root/");
			CreateDir("tftp_root/Boot/");

			CreateDir("tftp_root/OSChooser/");
			CreateDir("tftp_root/Setup/");

			CreateDir("tftp_root/Boot/x86/");
			CreateDir("tftp_root/Boot/x86/efi/");

			CreateDir("tftp_root/Boot/x64/");
			CreateDir("tftp_root/Boot/x64/efi/");

			return true;
		}

		EXPORT Filesystem::~Filesystem()
		= default;

		EXPORT void Filesystem::Write(const std::string& path, const char * buffer, const int offset, const int count)
		{
		}
		
		EXPORT void Filesystem::Read(const std::string& path, char * buffer, const int offset, const int count)
		{
		}

		EXPORT bool Filesystem::CreateDir(const std::string& path)
		{
			printf("Creating Directory: %s\n",ResolvePath(path).c_str());

			return CreateDirectory(ResolvePath(path).c_str(), nullptr) == 0;
		}

		EXPORT const std::string replace(std::string& str,
			const std::string& from, const std::string& to)
		{
			size_t start_pos = str.find(from);

			while (str.find(from) != std::string::npos)
			{
				start_pos = str.find(from);

				if (start_pos != std::string::npos)
					str = str.replace(start_pos, from.length(), to);
			}

			return str;
		}

		EXPORT std::string Filesystem::ResolvePath(const std::string& path)
		{
			std::string res;

			if (path.find_first_of('\\') == 0 || path.find_first_of('/') == 0)
			{
				res = GetCurDir() + "/" + path.substr(1, path.length() - 1);
#ifdef _WIN32
				res = replace(res, "/", "\\");
#else
				res = replace(res, "\\", "/");
#endif
			}
			else
			{
				res = GetCurDir() + "/" + path;
#ifdef _WIN32
				res = replace(res, "//", "/");
				res = replace(res, "/", "\\");
#else
				res = replace(res, "\\\\", "\\");
				res = replace(res, "\\", "/");
#endif
			}

			return res;
		}

		EXPORT std::string Filesystem::GetCurDir()
		{
			char cCurrentPath[260];
			ClearBuffer(cCurrentPath,260);
#ifdef _WIN32

			_getcwd(cCurrentPath, 260);
#else
			getcwd(cCurrentPath, 260);
#endif
			return std::string(cCurrentPath);
		}
	}
}

