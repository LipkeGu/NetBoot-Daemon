#include "Filesystem.h"

namespace Netbootd
{
	namespace System
	{
		EXPORT Filesystem::Filesystem(std::string filename, FileOpenMode mode)
		{
			this->workingDir = GetCurDir();
			this->mode = mode;
			this->filesize = 0;
			this->filename = this->ResolvePath(filename);
			this->isOpen = this->Open();
			this->ctype = nullptr;

			if (this->isOpen)
			{
				if (this->filename.find(".txt") != std::string::npos)
					this->ctype = "text/plain";

				if (this->filename.find(".bcd") != std::string::npos)
					this->ctype = "application/octet-stream";

				if (this->filename.find(".wim") != std::string::npos)
					this->ctype = "application/octet-stream";

				if (this->filename.find(".sdi") != std::string::npos)
					this->ctype = "application/octet-stream";
			}
			else
				this->ctype = "";
		}

		EXPORT bool Filesystem::Open()
		{
			switch (this->mode)
			{
			case FileWrite:
				this->file = fopen(this->filename.c_str(), "w");
				break;
			case FileRead:
				this->file = fopen(this->filename.c_str(), "r");
				break;
			case FileReadBinary:
				this->file = fopen(this->filename.c_str(), "rb");
				break;
			case FileWriteBinary:
				this->file = fopen(this->filename.c_str(), "wb");
				break;
			default:
				this->file = nullptr;
				break;
			}

			if (this->file != nullptr)
			{
				if (fseek(this->file, 0, SEEK_END) == 0)
				{
					this->filesize = ftell(this->file);
					rewind(this->file);

					return true;
				}
				else
					return false;
			}
			else
				return false;
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

		EXPORT unsigned int Filesystem::Write(const char* data,
			unsigned int length, unsigned int byteswritten, long seek)
		{
			unsigned int res = 0;

			if (!this->isOpen || this->mode == FileRead || this->mode == FileReadBinary)
				return res;

			if (length > 0)
			{
				if (fseek(this->file, seek, SEEK_END) == 0)
				{
					byteswritten += fwrite(data, 1, length, this->file);
					res = byteswritten;
				}
			}

			return res;
		}

		
		EXPORT unsigned int Filesystem::Read(char* dest, unsigned int dest_offset, long seek, unsigned int length) const
		{
			unsigned int res = 0;

			if (!this->isOpen || this->mode == FileWrite || this->mode == FileWriteBinary)
				return res;

			if (fseek(this->file, seek, SEEK_SET) == 0)
				res = fread(&dest[dest_offset], 1, length, this->file);

			return res;
		}

		EXPORT int Filesystem::Close()
		{
			int retval = -1;
			if (!this->isOpen)
				return retval;

			retval = fclose(this->file);
			this->filesize = 0;
		
			return retval;
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

		EXPORT bool Filesystem::Exist()
		{
			return this->isOpen;
		}

		EXPORT std::string Filesystem::CType()
		{
			return this->ctype;
		}

		EXPORT long Filesystem::Length()
		{
			return this->filesize;
		}

		EXPORT std::string Filesystem::Name()
		{
			return this->filename;
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

