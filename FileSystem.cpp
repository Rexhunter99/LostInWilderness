#include "FileSystem.h"

#include <fstream>

using namespace FileSystem;

#if defined(_MSC_VER) || defined (_WIN32)

#include <Windows.h>

bool Directory::create(const std::string & path)
{
	return CreateDirectory(path.c_str(), nullptr) ? true : false;
}


void Directory::remove(const std::string & path)
{
	RemoveDirectory(path.c_str());
}

#elif defined(__GNUC__)

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

bool Directory::create(const std::string & path)
{
	return !mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}


void Directory::remove(const std::string & path)
{
	rmdir(path.c_str()); // returns 0 if it worked, -1 if it failed, use `errno` to find out why
}

#endif
