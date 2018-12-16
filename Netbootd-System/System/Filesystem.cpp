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
			this->filename = ResolvePath(filename);
			this->isOpen = this->Open();
			this->ctype = nullptr;
			this->file = nullptr;

			if (this->isOpen)
			{
				if (this->filename.find(".txt") != std::string::npos)
					this->set_ctype("text/plain");
					
				if (this->filename.find(".bcd") != std::string::npos)
					this->set_ctype("application/octet-stream");
					
				if (this->filename.find(".wim") != std::string::npos)
					this->set_ctype("application/octet-stream");

				if (this->filename.find(".sdi") != std::string::npos)
					this->set_ctype("application/octet-stream");
			}
		}

		EXPORT bool Filesystem::Open()
		{
			switch (this->mode)
			{
			case FileWrite:
				this->file = fopen(this->filename.c_str(), "we");
				break;
			case FileRead:
				this->file = fopen(this->filename.c_str(), "re");
				break;
			case FileReadBinary:
				this->file = fopen(this->filename.c_str(), "rbe");
				break;
			case FileWriteBinary:
				this->file = fopen(this->filename.c_str(), "wbe");
				break;
			default:
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
			}

			return false;
		}

		EXPORT bool Filesystem::Init() const
		{
			CreateDir("tftp_root/");
			CreateDir("tftp_root/Boot/");
			CreateDir("tftp_root/Boot/x86/");
			CreateDir("tftp_root/Boot/x86/efi/");
			CreateDir("tftp_root/Boot/x64/");
			CreateDir("tftp_root/Boot/x64/efi/");
			CreateDir("tftp_root/OSChooser/");
			CreateDir("tftp_root/Setup/");

			return true;
		}

		EXPORT Filesystem::~Filesystem()
			= default;

		EXPORT size_t Filesystem::Write(const char* data,
			const _SIZE_T length, _SIZE_T offset, const long seek) const
		{
			_SIZE_T res = 0;

			if (!this->isOpen || this->mode == FileRead || this->mode == FileReadBinary)
				return res;

			if (length > 0 && fseek(this->file, seek, 2) == 0)
			{
				offset += fwrite(data, 1, length, this->file);
				res = offset;
			}

			return res;
		}

		EXPORT _SIZE_T Filesystem::Read(char* dest, const _SIZE_T offset,
			const long seek, const _SIZE_T length) const
		{
			_SIZE_T res = 0;

			if (!this->isOpen || this->mode == FileWrite || this->mode == FileWriteBinary)
				return res;

			if (fseek(this->file, seek, 0) == 0)
				res = fread(&dest[offset], 1, length, this->file);

			return res;
		}

		EXPORT int Filesystem::Close()
		{
			auto retval = -1;

			if (!this->isOpen)
				return retval;

			retval = fclose(this->file);
			this->filesize = 0;

			return retval;
		}

		EXPORT INLINE bool Filesystem::CreateDir(const std::string& path)
		{
			return CreateDirectory(ResolvePath(path)
				.c_str(), nullptr) == 0;
		}

		EXPORT std::string replace(std::string& str,
			const std::string& from, const std::string& to)
		{
			auto start_pos = str.find(from);

			while (str.find(from) != std::string::npos)
			{
				start_pos = str.find(from);

				if (start_pos != std::string::npos)
					str = str.replace(start_pos, from.length(), to);
			}

			return str;
		}

		EXPORT INLINE bool Filesystem::Exist() const
		{
			return this->isOpen;
		}

		EXPORT INLINE std::string Filesystem::get_ctype() const
		{
			return this->ctype;
		}

		EXPORT INLINE void Filesystem::set_ctype(const std::string& ctype)
		{
			this->ctype = ctype;
		}

		EXPORT INLINE _SIZE_T Filesystem::Length() const
		{
			return this->filesize;
		}

		EXPORT INLINE std::string Filesystem::Name() const
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
			ClearBuffer(cCurrentPath, sizeof cCurrentPath);
#ifdef _WIN32
			_getcwd(cCurrentPath, sizeof cCurrentPath);
#else
			getcwd(cCurrentPath, sizeof cCurrentPath);
#endif
			return std::string(cCurrentPath);
		}
	}
}

