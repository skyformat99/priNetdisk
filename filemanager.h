#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <boost/filesystem.hpp>
#include <fcntl.h>
#include <string>

class fileManager
{
public:
	fileManager();
private:

	std::string storeFolder;

};

#endif // FILEMANAGER_H
