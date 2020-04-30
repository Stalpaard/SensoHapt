#pragma once
#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <fstream>
#include <string>

//Custom fstream class with support for stream names
class FileStream : public std::fstream
{
public:
	FileStream(const std::wstring& name) : name{ name } {}

	const std::wstring& getName() const { return name; }
private:
	std::wstring name;
};

#endif