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
			typedef enum FileOpenMode
			{
				FileRead = 0,
				FileWrite = 1,
				FileReadBinary = 2,
				FileWriteBinary = 3
			} FileOpenMode;

			EXPORT Filesystem(){};
			EXPORT Filesystem(std::string filename, FileOpenMode mode);
			EXPORT ~Filesystem();

			EXPORT bool Init() const;

			EXPORT _SIZE_T Write(const char* data, const _SIZE_T length,
				_SIZE_T offset, const long seek) const;

			EXPORT _SIZE_T Read(char* dest, const _SIZE_T offset,
				const long seek, const _SIZE_T length) const;
			static
			EXPORT std::string ResolvePath(const std::string& path, const bool external = false);
			static
			EXPORT bool CreateDir(const std::string& path);
			static
			EXPORT std::string GetCurDir();
			EXPORT bool Open();
			EXPORT INLINE bool Exist() const;
			EXPORT INLINE std::string Name() const;
			EXPORT INLINE _SIZE_T Length() const;
			EXPORT INLINE std::string get_ctype() const;
			EXPORT INLINE void set_ctype(const std::string& ctype);
			EXPORT int Close();

		private:
			std::string workingDir;

			bool isOpen;
			_SIZE_T filesize;
			std::string ctype;
			FILE* file;
			std::string filename;
			FileOpenMode mode;
		};
	}
}
