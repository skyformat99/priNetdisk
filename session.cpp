#include "session.h"

#include "prinetdisk.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>

static off64_t getFileSize(const char* filePath)
{
	struct stat fileInfo;
	bzero(&fileInfo,sizeof( fileInfo));
	int ret = stat(filePath,&fileInfo);
	if( ret < 0)
	{
		return -1;
	}
	return fileInfo.st_size;
}

session::session(const int &clientsock ,
				 const std::function<void (const std::string &,
										   const std::string &,
										   const int &, std::string &)> &tempDealMsg):
	clientSock(clientsock),sendFileFd(-1),
	headOver(false),
	haveRead(0),haveSend(0),bodyLen(0),
	dealMsg(tempDealMsg),
	SPLCHAR('#')
{
	memset(msg,0,MSGBODYLEN);
}
session::~session()
{
	close(sendFileFd);
	close(clientSock);
	std::cout << "destory session " <<std::endl;
}

void session::splitMsg()
{
	std::string result;
	auto itor = std::find(msg,msg+bodyLen,SPLCHAR);
	if( itor - msg == bodyLen)
		result = "MSG:msg not complete";
	else
	{
		dealMsg(std::string(msg,itor),std::string(itor+1),this->clientSock , result);
		result.insert( 0 ,msg, itor - msg + 1);		//添加返回消息头部
	}
	addMsgHead(result);
	strcpy(msg,result.c_str());
}

bool session::openFile(const std::string& filePath)
{
	sendFileFd = open(filePath.c_str() , O_RDONLY);
	fileSize = getFileSize(filePath.c_str());
	if(sendFileFd < 0 )
		return false;
	return true;
}

void session::closeFile()
{
	close(sendFileFd);
}

void session::addMsgHead(std::string &result)
{
	std::string Head = std::to_string(result.size());
	while( Head.size() != MSGHEADLEN)
		Head.insert(Head.begin(),'0');
	result.insert(0,Head);
}
















