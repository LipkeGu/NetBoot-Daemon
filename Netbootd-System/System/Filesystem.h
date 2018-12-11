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

			EXPORT bool Init();

			EXPORT unsigned int Write(const char* data, unsigned int length, unsigned int byteswritten, long seek);
			EXPORT unsigned int Read(char* dest, unsigned int dest_offset, long seek, unsigned int length) const;
			EXPORT std::string ResolvePath(const std::string& path);
			EXPORT bool CreateDir(const std::string& path);
			EXPORT std::string GetCurDir();
			EXPORT bool Open();
			EXPORT bool Exist();
			EXPORT std::string Name();
			EXPORT long Length();
			EXPORT std::string CType();
			EXPORT int  Close();

		private:
			std::string workingDir;
			unsigned int byteswritten;

			bool isOpen;
			unsigned int filesize;
			std::string ctype;
			FILE* file;
			std::string filename;
			FileOpenMode mode;
		};
	}
}
