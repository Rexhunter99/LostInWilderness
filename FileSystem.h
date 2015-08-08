#pragma once

#if !defined(__FILESYSTEM_H__)
#define __FILESYSTEM_H__

#include <string>


namespace FileSystem
{

	namespace Directory
	{
		bool create(const std::string &);
		bool exists(const std::string &);
		void remove(const std::string &);

	}; //Directory

}; //FileSystem

#endif //__FILESYSTEM_H__